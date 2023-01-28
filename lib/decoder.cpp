#include"decoder.hpp"
#define total_bits 1024
#define traverse_left_node 0
#define traverse_right_node 1
#define pop_to_parent_node 2
decoder::decoder(std::vector<double>&& likely_value,std::unordered_set<int>&& frozen_channe,std::string decoder_type)
{  
    _likelyhood_value=std::vector<std::vector<double>>(log2(total_bits)+1,std::vector<double>(total_bits));
    for (size_t i = 0; i < likely_value.size(); i++)
    {
        _likelyhood_value[0][i]=likely_value[i];
    }
    _code=std::vector<std::vector<int>>(log2(total_bits)+1,std::vector<int>(total_bits));
    _frozen_channel=std::move(frozen_channe);
    sc_calculate();
}
// TODO:log2(1024)+1的修改
void decoder::sc_calculate(){
    int depth=0; // the depth of the tree 
    int node=0; // 0-01-012-0123
    bool done=false;
    int tree_depth=log2(_likelyhood_value[0].size()); // the depth of the tree
    std::vector<int> node_state(2*_likelyhood_value[0].size()-1); // used for tree traversal:travel to right/to left/pop
    while(!done){
        if(depth==tree_depth){
            // leaf level
            // in leaf level:node==channel index
            if(_frozen_channel.count(node)){
                _code[tree_depth][node]=0;
            }
            else{
                if(_likelyhood_value[tree_depth][node]>=0)
                    _code[tree_depth][node]=0;
                else 
                    _code[tree_depth][node]=1;
            }
            if(node==_likelyhood_value[0].size()-1)
                done=true;
            else{ // children node pop to parent node 
                node/=2;depth-=1;
            }
        }
        else{
            // nonleaf
            int node_pos=pow(2,depth)-1+node; // node position on the tree
            int temp=pow(2,tree_depth-depth+1); // temp:number of likelihood values contained in each node
            if(node_state[node_pos]==traverse_left_node){ // travel to left node
                temp/=2;node*=2;depth+=1;
                for(size_t i=0;i<temp/2;++i) // f
                    _likelyhood_value[depth][i+temp*node]=f(_likelyhood_value[depth-1][i+temp*node],_likelyhood_value[depth-1][i+temp*node+temp/2]);
                node_state[node_pos] = traverse_right_node;      
            }
            else if(node_state[node_pos]==traverse_right_node){ // travel to right node
                int lnode = 2*node,ldepth = depth + 1,ltemp=temp/2;
                node = node *2 + 1; depth = depth + 1; 
                temp = temp / 2; 
                for(size_t i=0;i<temp/2;i++) // g
                    _likelyhood_value[depth][temp*node]=g(_likelyhood_value[depth-1][i+temp*node],_likelyhood_value[depth-1][i+temp*node+temp/2],_code[ldepth][i+ltemp*lnode]);
                node_state[node_pos]=pop_to_parent_node;    
            }
            else if(node_state[node_pos]==pop_to_parent_node){ // deal with current node and pop
                int lnode = 2*node,rnode = 2*node + 1,cdepth = depth + 1; // left and right child node
                int ctemp = temp/2;
                for(size_t i=0;i<temp/2;++i){
                    _code[depth][i+temp*node]=(_code[cdepth][i+ctemp*lnode]+_code[cdepth][i+ctemp*rnode])%2;
                    _code[depth][i+temp*node+temp/2]=_code[cdepth][i+ctemp*rnode];
                }                  
                node = node/2; depth = depth - 1; // pop
            }
        }
    }
}

inline double decoder::f(double x,double y){
    return log2((1+exp(x+y))/(exp(x)+exp(y)));
}

inline double decoder::g(double x,double y,double z){
    return y+(1-2*z)*x;
}

std::vector<int> decoder::result(){
    size_t cnt=0;
    for(size_t i=0;i<_code.size();++i){
        if(_frozen_channel.count(i))continue;
        _message[cnt]=_code[log2(total_bits)-1][i];
        ++cnt;
    }
    return _message;
}
decoder::~decoder()
{
}