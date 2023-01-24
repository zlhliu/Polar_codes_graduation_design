#include"decoder.h"
#define total_bits 1024
#define traverse_left_node 0
#define traverse_right_node 1
#define node_traversal_done 2
decoder::decoder(std::vector<double>&& likely_value,std::string decoder_type,const std::unordered_set<int>& Q)
{  
    _received_likelyhood_value=std::move(likely_value);
    _code=std::vector<std::vector<int>>(log2(total_bits),std::vector<int>(total_bits));
}
void decoder::sc_calculate(const std::unordered_set<int>& frozen_channel){
    int depth=0; // the depth of the tree 
    int node=0; // 0-01-012-0123
    bool done=false;
    int tree_depth=log2(_received_likelyhood_value.size())-1; // the depth of the tree
    std::vector<int> node_state(2*_received_likelyhood_value.size()-1); // used for tree traversal:travel to right/to left/pop
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
            if(node==_received_likelyhood_value.size()-1)
                done=true;
            else{ // children node pop to parent node 
                node/=2;depth-=1;
            }
        }
        else{
            // nonleaf
            int node_pos=(2^depth-1)+node; // node position on the tree
            int temp=pow(2,tree_depth-depth); // temp:number of likelihood values contained in each node
            std::vector<double> ln(temp);
            for(int i=0;i<temp;++i)
                ln[i]=_likelyhood_value[depth][i+temp*node];
            if(node_state[node_pos]==traverse_left_node){ // travel to right node
                temp/=2;node*=2;depth+=1; // right node likelyhood value
                for(int i=0;i<temp/2;++i)
                    _likelyhood_value[depth][i+temp*node]=f(ln[i],ln[i+temp/2]);
                node_state[node_pos] = traverse_right_node;      
            }
            else if(node_state[node_pos]==traverse_right_node){ // travel to left node
                int lnode = 2*node,ldepth = depth + 1,ltemp=temp/2;
                std::vector<int> code_temp(temp);
                for(int i=0;i<temp;++i)
                    code_temp[i]=_code[ldepth][i+ltemp*lnode];
                node = node *2 + 1; depth = depth + 1; 
                temp = temp / 2; 
                for(int i=0;i<temp/2;i++)
                    _likelyhood_value[depth][temp*node]=g(ln[i],ln[i+temp/2],code_temp[i]);
                node_state[node_pos]=node_traversal_done;    
            }
            else if(node_state[node_pos]==node_traversal_done){ // deal with current node
                int lnode = 2*node,rnode = 2*node + 1,cdepth = depth + 1; // left and right child node
                int ctemp = temp/2;
                std::vector<int> code_left_temp(ctemp),code_right_temp(ctemp);
                for(int i=0;i<ctemp;++i)
                    code_left_temp[i]=_code[cdepth][ctemp*lnode]; // incoming decisions from left child
                for(int i=0;i<ctemp;++i)
                    code_right_temp[i]=_code[cdepth][ctemp*rnode]; // incoming decisions from right child
                for(int i=0;i<temp/2;++i){
                    _code[depth][i+temp*node]=(code_left_temp[i]+code_right_temp[i])%2;
                    _code[depth][i+temp*node+temp/2]=code_right_temp[i];
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
decoder::~decoder()
{
}