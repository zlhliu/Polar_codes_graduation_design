#include"decoder.hpp"
#define total_bits 1024
#define info_bits 512
#define tree_depth 10
#define traverse_left_node 0
#define traverse_right_node 1
#define pop_to_parent_node 2


inline double f(double x,double y){
    // approximation of log((1+exp(x+y))/(exp(x)+exp(y)))
    return (1-2*(x<0))*(1-2*(y<0))*std::min(abs(x),abs(y)); // sign(x)*sign(y)*min(|x|,|y|)
}

inline double g(double x,double y,int z){
    return y+(1-2*z)*x;
}

// TODO:不要用&&
decoder::decoder(const std::vector<double>& likely_value,const std::unordered_set<int>& frozen_channel,std::string decoder_type)
{  
    _likelyhood_value=std::vector<std::vector<double>>(log2(total_bits)+1,std::vector<double>(total_bits));
    for (size_t i = 0; i < likely_value.size(); i++)
    {
        _likelyhood_value[0][i]=likely_value[i];
    }
    _code=std::vector<std::vector<int>>(log2(total_bits)+1,std::vector<int>(total_bits));
    _message=std::vector<int>(likely_value.size());
    if(decoder_type=="sc")
        sc_calculate(frozen_channel);
    else if(decoder_type=="fsc")
        fsc_calculate(frozen_channel);
}

decoder::decoder(std::vector<std::vector<fast_node_state>>&& temp){
    _fast_node_type=std::move(temp);
}
decoder::decoder(){
    _code=std::vector<std::vector<int>>(log2(total_bits)+1,std::vector<int>(total_bits));
}
void decoder::start_calculation(const std::vector<double>& likely_value,const std::unordered_set<int>& frozen_channel,std::string decoder_type){
    _likelyhood_value=std::vector<std::vector<double>>(log2(total_bits)+1,std::vector<double>(total_bits));
    for (size_t i = 0; i < likely_value.size(); i++)
    {
        _likelyhood_value[0][i]=likely_value[i];
    }
    _message=std::vector<int>(likely_value.size());
    if(decoder_type=="sc")
        sc_calculate(frozen_channel);
    else if(decoder_type=="fsc")
        fsc_calculate(frozen_channel);
}

// TODO:log2(1024)+1的修改
void decoder::sc_calculate(const std::unordered_set<int>& frozen_channel){
    int depth=0; // the depth of the tree 
    int node=0; // 0-01-012-0123
    bool done=false;
    std::vector<int> node_state(2*total_bits-1); // used for tree traversal:travel to right/to left/pop

    while(!done){
        if(depth==tree_depth){
            // leaf level
            // in leaf level:node==channel index
            if(frozen_channel.count(node)){
                _code[tree_depth][node]=0;
            }
            else{
                if(_likelyhood_value[tree_depth][node]>=0)
                    _code[tree_depth][node]=0;
                else 
                    _code[tree_depth][node]=1;
            }
            if(node==_likelyhood_value[0].size()-1){
                int temp = _likelyhood_value[0].size()-1;
                done=true;
            }
            else{ // children node pop to parent node 
                node/=2;depth-=1;
            }
        }
        // TODO:temp大小写错了
        else{
            // nonleaf
            int node_pos=pow(2,depth)-1+node; // node position on the tree
            int temp=pow(2,tree_depth-depth); // temp:number of likelihood values contained in each node
            if(node_state[node_pos]==traverse_left_node){ // travel to left node
                int ltemp=temp/2,lnode=node*2,ldepth=depth+1; // l:left node
                for(size_t i=0;i<ltemp;++i) // f
                    _likelyhood_value[ldepth][i+ltemp*lnode]=f(_likelyhood_value[depth][i+temp*node],_likelyhood_value[depth][i+temp*node+temp/2]);
                depth+=1;node*=2;
                node_state[node_pos] = traverse_right_node;      
            }
            else if(node_state[node_pos]==traverse_right_node){ // travel to right node
                int ntemp=temp/2,ndepth=depth+1; // n:left or right node
                int lnode=2*node,rnode=2*node+1;
                for(size_t i=0;i<ntemp;i++) // g
                    _likelyhood_value[ndepth][i+ntemp*rnode]=g(_likelyhood_value[depth][i+temp*node],_likelyhood_value[depth][i+temp*node+ntemp],_code[ndepth][i+ntemp*lnode]);
                depth+=1;node=2*node+1;
                node_state[node_pos]=pop_to_parent_node;    
            }
            else if(node_state[node_pos]==pop_to_parent_node){ // deal with current node and pop
                int lnode=2*node,rnode=2*node+1;
                int ntemp=temp/2,ndepth=depth+1;
                for(size_t i=0;i<temp/2;++i){
                    _code[depth][i+temp*node]=(_code[ndepth][i+ntemp*lnode]+_code[ndepth][i+ntemp*rnode])%2;
                    _code[depth][i+temp*node+temp/2]=_code[ndepth][i+ntemp*rnode];
                }                  
                node = node/2; depth = depth - 1; // pop
            }
        }
    }
}


void decoder::fsc_calculate(const std::unordered_set<int>& frozen_channel){
    int depth=0; // the depth of the tree 
    int node=0; // 0-01-012-0123
    bool done=false;
    std::vector<int> node_state(2*total_bits-1); // used for tree traversal:travel to right/to left/pop

    ThreadPool pool(8); // 8个线程的线程池
    // std::vector<std::future<std::vector<int>>> results; // results中放置快速节点计算出的最终结果,即_code[tree_depth]的一部分结果
    // std::mutex mutex_for_code; // _code的访问需要加锁
    // std::mutex mutex_for_likelyhood_value; // _likelyhood_value的访问需要加锁


    // 计算R0节点,需要把R0节点的位置信息给入函数,然后将对应的_code[tree_depth]全部置零
    // 位置信息,即是所在译码树的层数depth和这一层的位置node_pos
    std::function<void(int,int,std::vector<std::vector<int>>&& _code)> 
        calculate_R0 = [&](int node,int depth,std::vector<std::vector<int>>&& _code){ // node , depth, _code
        // 直接将对应位置的所有_code置零
        int start,end;
        int d=tree_depth-depth;// 从depth到译码树的底部需要d层
        start=node*pow(2,d);
        end=start+pow(2,d)-1;
        
        // 注意到每个子线程一定是写_code的不同内存段，所以不需要加锁
        for (size_t i = start; i <= end; i++){
            _code[tree_depth][i]=0;
        }          
    };

    // 计算R1节点,因为全都是信息节点,所以不会有似然值的修改,所以只需要硬解码即可
    std::function<void(int node,int depth,const std::vector<std::vector<double>>& _likelyhood_value,std::vector<std::vector<int>>&& _code)> 
        calculate_R1 = [&](int node,int depth,const std::vector<std::vector<double>>& _likelyhood_value,std::vector<std::vector<int>>&& _code){
            // 直接最大似然估计
            // 写一个likehood_value存储_likelyhood_value的储存值,不然要锁住内存，那主进程就不能继续了
            int start,end;
            int d=tree_depth-depth;// 从depth到译码树的底部需要d层
            start=node*pow(2,d);
            end=start+pow(2,d)-1;

            std::vector<int> code(pow(2,d));
            // _likelyhood_value会被主线程频繁访问，所以不易加锁
            for (size_t i = start; i <= end; i++){
                if(_likelyhood_value[depth][i]>=0)
                    code[i-start]=0;
                else
                    code[i-start]=1;
            }
            
            int m = 1; // number of bits combined
            for(size_t i=0;i=d;i++){ // i:number of coding tree layer
                for(size_t j=0;j<pow(2,d);j+=2*m){ // N bits,m bits per node
                    std::vector<int>a(m),b(m);
                    for(size_t k=0;k<m;k++){
                        a[k]=code[j+k];
                        b[k]=code[j+k+m];
                    }
                    for(size_t k=0;k<m;k++){
                        code[j+k]=a[k]^b[k];
                        code[j+k+m]=b[k];
                    }
                }
                m*=2;
            }

            // 不需要加锁
            for (size_t i = start; i <= end; i++){
                _code[tree_depth][i]=code[i-start];
            }
            
        };
    
    // 计算REP节点
    // REP节点为00……1,此时顶层的code为1信道信息的复制
    // 所有likelyhood_value相加即使最大似然估计
    std::function<void(int,int,double,std::vector<std::vector<int>>&&)> 
        calculate_REP = [&](int node,int depth,double likelyhood_sum,std::vector<std::vector<int>>&& _code){
            int start,end;
            int d=tree_depth-depth;// 从depth到译码树的底部需要d层
            start=node*pow(2,d);
            end=start+pow(2,d)-1;

            if (likelyhood_sum>0)
                _code[tree_depth][end]=0;
            else
                _code[tree_depth][end]=1;
            
            for (size_t i = start; i < end; i++){
                _code[tree_depth][i]=0;
            }
        };


    // 计算SPC节点
    // 简单的说,SPC节点是一个奇偶校验节点
    // 要注意的是,likelyhood_value有可能要修改
    std::function<void(int,int,std::vector<std::vector<double>>&& _likelyhood_value,std::vector<std::vector<int>>&& _code)> 
        calculate_SPC = [&](int node,int depth,std::vector<std::vector<double>>&& _likelyhood_value,std::vector<std::vector<int>>&& _code){
            int start,end;
            int d=tree_depth-depth;// 从depth到译码树的底部需要d层
            start=node*pow(2,d);
            end=start+pow(2,d)-1;
            bool p=0;

            for (size_t i = start; i <= end ; i++){
                p^=(_likelyhood_value[depth][i]<0);
            }

            // 如果p==0,则大概率没有错误,如果p==1,则必定有奇数个错误
            // 修改似然值绝对值最小的bit
            if(p==0)
                calculate_R1(node,depth,_likelyhood_value,std::move(_code));
            else if(p==1){
                // 找到似然值绝对值最小的
                double min=DBL_MAX;
                size_t min_i=-1;
                for (size_t i = start; i <= end; i++){
                    if(min>abs(_likelyhood_value[depth][i])){
                        min=abs(_likelyhood_value[depth][i]);
                        min_i=i;
                    }
                }
                // 修改似然值,因为此数值最后可能是错误的
                _likelyhood_value[depth][min_i]*=-1;
                calculate_R1(node,depth,_likelyhood_value,std::move(_code));
            }
        };

    while(!done){
        // 可以注意到,译码的最小单位是00,01,11,10所以不可能有到tree_depth层译码的情况
        int node_pos=pow(2,depth)-1+node; // node position on the tree
        int temp=pow(2,tree_depth-depth); // temp:每个节点中的似然值/码字的数量
        if(node_state[node_pos]==traverse_left_node){ // 遍历左子节点
            int ltemp=temp/2,lnode=node*2,ldepth=depth+1; // l:左节点
            if(_fast_node_type[depth][node].state==NONE){ // 此节点非快速节点,执行正常的f,然后遍历子节点
                for(size_t i=0;i<ltemp;++i) // f
                    _likelyhood_value[ldepth][i+ltemp*lnode]=f(_likelyhood_value[depth][i+temp*node],_likelyhood_value[depth][i+temp*node+temp/2]);
                depth+=1;node*=2;
                node_state[node_pos] = traverse_right_node;   
            }
            else if(_fast_node_type[depth][node].state==R0){ // 此节点为R0节点
                // R0节点可以直接计算本层_code,立马回返父节点
                for (size_t i = 0; i < temp; i++){
                    _code[depth][i+temp*node]=0;
                }
                // 将计算R0节点的任务加入线程池
                pool.enqueue(calculate_R0,node,depth,std::move(_code));
                node_state[node_pos] = pop_to_parent_node;
            }
            else if(_fast_node_type[depth][node].state==R1){ // 此节点为R1节点
                // R1节点可以直接计算本层_code,立马回返父节点
                for (size_t i = 0; i < temp; i++){
                    _code[depth][i+temp*node]=(_likelyhood_value[depth][i+temp*node]<0);
                }
                pool.enqueue(calculate_R1,node,depth,_likelyhood_value,std::move(_code));
                node_state[node_pos] = pop_to_parent_node;
            }
            else if(_fast_node_type[depth][node].state==REP){
                // REP节点可以直接计算本层_code,立马回返父节点
                double sum=0;
                for (size_t i = 0; i < temp; i++){
                    sum+=_likelyhood_value[depth][i+temp*node];
                }
                for (size_t i = 0; i < temp; i++){
                    _code[depth][i+temp*node]=(sum<0);
                }
                pool.enqueue(calculate_REP,node,depth,sum,std::move(_code));
                node_state[node_pos] = pop_to_parent_node;
            }
            else if(_fast_node_type[depth][node].state==SPC){
                // SPC节点可以直接计算本层_code,立马回返父节点
                bool p=0;
                for (size_t i = 0; i <= temp ; i++){
                    p^=(_likelyhood_value[depth][i+temp*node]<0);
                }
                // 如果p==0,则大概率没有错误,如果p==1,则必定有奇数个错误
                // 修改似然值绝对值最小的bit
                if(p==0){
                    for (size_t i = 0; i < temp; i++){
                        _code[depth][i+node*temp]=(_likelyhood_value[depth][i+temp*node]<0);
                    }
                    pool.enqueue(calculate_R1,node,depth,_likelyhood_value,std::move(_code));
                }
                else if(p==1){
                    // 找到似然值绝对值最小的
                    double min=DBL_MAX;
                    size_t min_i=-1;
                    for (size_t i = 0; i <= temp; i++){
                        if(min>abs(_likelyhood_value[depth][i+node*temp])){
                            min=abs(_likelyhood_value[depth][i+node*temp]);
                            min_i=i;
                        }
                    }
                    // 修改似然值,因为此数值最后可能是错误的
                    _likelyhood_value[depth][min_i]*=-1;
                    pool.enqueue(calculate_SPC,node,depth,std::move(_likelyhood_value),std::move(_code));
                    
                }
            }
        else if(node_state[node_pos]==traverse_right_node){ // travel to right node
            int ntemp=temp/2,ndepth=depth+1; // n:left or right node
            int lnode=2*node,rnode=2*node+1;
            for(size_t i=0;i<ntemp;i++) // g
                _likelyhood_value[ndepth][i+ntemp*rnode]=g(_likelyhood_value[depth][i+temp*node],_likelyhood_value[depth][i+temp*node+ntemp],_code[ndepth][i+ntemp*lnode]);
            depth+=1;node=2*node+1;
            node_state[node_pos]=pop_to_parent_node;    
        }
        else if(node_state[node_pos]==pop_to_parent_node){ // deal with current node and pop
            int lnode=2*node,rnode=2*node+1;
            int ntemp=temp/2,ndepth=depth+1;
            for(size_t i=0;i<temp/2;++i){
                _code[depth][i+temp*node]=(_code[ndepth][i+ntemp*lnode]+_code[ndepth][i+ntemp*rnode])%2;
                _code[depth][i+temp*node+temp/2]=_code[ndepth][i+ntemp*rnode];
            }                  
            node = node/2; depth = depth - 1; // pop
        }
        
    }
}

// TODO:存在inf，nan
std::vector<int> decoder::result(const std::vector<int>& Q){
    // debug
    // std::vector<int> test={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,0,0,0,1,1,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,1,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1,1,0,0,1,0,1,0,0,0,1,0,1,1,1,0,0,0,1,0,1,0,0,0,0,1,1,0,1,1,1,0,0,1,1,1,0,0,0,1,0,0,1,1,1,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,1,0,0,0,1,1,0,0,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,1,1,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,1,1,0,1,0,0,1,1,1,1,0,0,1,0,0,0,1,1,0,1,0,1,1,0,0,1,0,0,0,0,0,0,1,1,0,1,1,1,1,0,1,1,0,0,1,0,0,0,0,0,0,0,1,0,0,1,0,1,0,1,1,0,0,0,0,0,0,1,0,0,1,1,0,1,1,1,1,1,1,0,0,1,0,1,1,1,0,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,1,0,1,0,1,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,1,0,0,1,1,0,1,1,0,0,1,0,0,1,0,0,0,1,1,1,1,0,0,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,0,1,0,1,1,0,0,1,0,1,0,0,1,1,1,1,0,0,0,0,1,1,0,1,0,0,0,0,0,1,0,1,0,0,1,0,1,0,1,0,0,0,1,1,0,1,0,1,1,1,0,0,0,0,1,0,1,0,0,1,0,1,0,1,0,0,1,0,1,1,0,0,0,0,0,0,1,1,1,0,0,1,1,0,1,0,1,0,0,1,0,1,0,0,1,1,1,0,1,1,1,1,1,0,1,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,1,1,1,0};
    // for(size_t i=0;i<test.size();i++){
    //     if(_code[10][i]!=test[i]){
    //         std::printf("why");
    //         // why?
    //     }
    // }
    
    // 注意的是，需要Q做坐标映射
    for(size_t i=0;i<info_bits;++i){
        _message[i]=_code[tree_depth][Q[i+info_bits]];
    }
    return _message;
}
decoder::~decoder()
{
}