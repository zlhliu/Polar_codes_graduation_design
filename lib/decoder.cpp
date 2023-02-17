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

<<<<<<< HEAD
=======
// inline void repu

>>>>>>> 936f85f (累了，先这样吧)
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
    sc_calculate(frozen_channel);
}
<<<<<<< HEAD
=======



>>>>>>> 936f85f (累了，先这样吧)
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
<<<<<<< HEAD
    sc_calculate(frozen_channel);
=======
    if(decoder_type=="sc")
        sc_calculate(frozen_channel);
    // else if(decoder_type=="fsc")
    //     fsc_calculate(frozen_channel);
>>>>>>> 936f85f (累了，先这样吧)
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

<<<<<<< HEAD
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

=======


// void decoder::fsc_calculate(const std::unordered_set<int>& frozen_channel){
//     int depth=0; // the depth of the tree 
//     int node=0; // 0-01-012-0123
//     bool done=false;
//     std::vector<int> node_state(2*total_bits-1); // used for tree traversal:travel to right/to left/pop

//     while(!done){

//         if(depth==tree_depth){
//             if(frozen_channel.count(node)){
//                 _code[tree_depth][node]=0;
//             }
//             else{
//                 if(_likelyhood_value[tree_depth][node]>=0)
//                     _code[tree_depth][node]=0;
//                 else 
//                     _code[tree_depth][node]=1;
//             }
//             if(node==_likelyhood_value[0].size()-1){
//                 int temp = _likelyhood_value[0].size()-1;
//                 done=true;
//             }
//             else{ // children node pop to parent node 
//                 node/=2;depth-=1;
//             }
//         }
//         // TODO:temp大小写错了
//         else{
//             // // nonleaf
//             // if()
//             int node_pos=pow(2,depth)-1+node; // node position on the tree
//             int temp=pow(2,tree_depth-depth); // temp:number of likelihood values contained in each node
//             if(node_state[node_pos]==traverse_left_node){ // travel to left node
//                 int ltemp=temp/2,lnode=node*2,ldepth=depth+1; // l:left node
//                 for(size_t i=0;i<ltemp;++i) // f
//                     _likelyhood_value[ldepth][i+ltemp*lnode]=f(_likelyhood_value[depth][i+temp*node],_likelyhood_value[depth][i+temp*node+temp/2]);
//                 depth+=1;node*=2;
//                 node_state[node_pos] = traverse_right_node;      
//             }
//             else if(node_state[node_pos]==traverse_right_node){ // travel to right node
//                 int ntemp=temp/2,ndepth=depth+1; // n:left or right node
//                 int lnode=2*node,rnode=2*node+1;
//                 for(size_t i=0;i<ntemp;i++) // g
//                     _likelyhood_value[ndepth][i+ntemp*rnode]=g(_likelyhood_value[depth][i+temp*node],_likelyhood_value[depth][i+temp*node+ntemp],_code[ndepth][i+ntemp*lnode]);
//                 depth+=1;node=2*node+1;
//                 node_state[node_pos]=pop_to_parent_node;    
//             }
//             else if(node_state[node_pos]==pop_to_parent_node){ // deal with current node and pop
//                 int lnode=2*node,rnode=2*node+1;
//                 int ntemp=temp/2,ndepth=depth+1;
//                 for(size_t i=0;i<temp/2;++i){
//                     _code[depth][i+temp*node]=(_code[ndepth][i+ntemp*lnode]+_code[ndepth][i+ntemp*rnode])%2;
//                     _code[depth][i+temp*node+temp/2]=_code[ndepth][i+ntemp*rnode];
//                 }                  
//                 node = node/2; depth = depth - 1; // pop
//             }
//         }
//     }
// }
// TODO:存在inf，nan
std::vector<int> decoder::result(const std::vector<int>& Q){
>>>>>>> 936f85f (累了，先这样吧)
    // 注意的是，需要Q做坐标映射
    for(size_t i=0;i<info_bits;++i){
        _message[i]=_code[tree_depth][Q[i+info_bits]];
    }
    return _message;
}
decoder::~decoder()
{
}