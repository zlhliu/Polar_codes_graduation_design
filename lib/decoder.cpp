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
    std::mutex mutex_for_code; // _code的访问需要加锁
    std::mutex mutex_for_likelyhood_value; // _likelyhood_value的访问需要加锁

    // 计算R0节点,需要把R0节点的位置信息给入函数,然后将对应的_code[tree_depth]全部置零
    // 位置信息,即是所在译码树的层数depth和这一层的位置node_pos
    std::function<void(int,int,std::vector<std::vector<int>>&& _code)> 
        calculate_R0 = [&](int node,int depth,std::vector<std::vector<int>>&& _code){
        // 直接将对应位置的所有_code置零
        int start,end;
        int d=tree_depth-depth;// 从depth到译码树的底部需要d层
        start=node*pow(2,d);
        end=start+pow(2,d)-1;

        {// _code向量需要加锁
            std::lock_guard<std::mutex> lock(mutex_for_code);
            for (size_t i = start; i <= end; i++){
                _code[tree_depth][i]=0;
            }   
        }
    };

    // 计算R1节点,因为全都是信息节点,所以不会有似然值的修改,所以只需要硬解码即可
    std::function<void(int,int,const std::vector<std::vector<double>>&,std::vector<std::vector<int>>&& _code)> 
        calculate_R1 = [&](int node,int depth,const std::vector<std::vector<double>>& _likelyhood_value,std::vector<std::vector<int>>&& _code){
        // 直接最大似然估计
        // 写一个likehood_value存储_likelyhood_value的储存值,不然要锁住内存，那主进程就不能继续了
        int start,end;
        int d=tree_depth-depth;// 从depth到译码树的底部需要d层
        start=node*pow(2,d);
        end=start+pow(2,d)-1;

        std::vector<int> code(pow(2,d));
        {   //  访问_likelyhood_value时需要加锁
            std::lock_guard<std::mutex> lock(mutex_for_likelyhood_value);
            for (size_t i = 0; i < pow(2,d); i++){
                if(_likelyhood_value[depth][node+i]>=0)
                    code[i]=0;
                else
                    code[i]=1;
            }
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

        { // 写入答案
            std::lock_guard<std::mutex> lock(mutex_for_code);
            for (size_t i = start; i <= end; i++){
                _code[tree_depth][i]=code[i-start];
            }
        }
    };
    
    // std::function<void(int,int,const std::vector<std::vector<double>>&,std::vector<std::vector<int>>&& _code)> 
    while(!done){
        if(depth==tree_depth){
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