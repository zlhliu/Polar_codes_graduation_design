#include"decoder.h"

decoder::decoder(std::vector<double>&& likely_value,std::string decoder_type,const std::vector<int>& Q)
{  
    _likelyhood_value=std::move(likely_value);
    
}
void decoder::sc_calculate(const std::vector<int>& Q){
    for(int i=0;i<_likelyhood_value.size();++i){
        
    }
}
decoder::~decoder()
{
}