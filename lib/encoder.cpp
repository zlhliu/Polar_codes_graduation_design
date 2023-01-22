#include"encoder.h"

// rate:K/N,K=_data.size(),N=_code.size()
encoder::encoder(std::vector<int>&& data,int K,int N,const std::vector<int>& Q){
    _data=data;
    _K=K;
    _N=N;
    code_calculate(Q);
}
encoder::~encoder(){
}

void encoder::code_calculate(const std::vector<int>& Q){
    std::vector<int> temp(_N,0);
    _code=std::move(temp); // set frozen bits
    for(int i=0;i<_K;i++){ // set inf bits
        _code[Q[i+_N-_K]]=_data[i];
    }
    int m = 1; // number of bits combined
    for(int i=log2(_N)-1;i>=0;i--){ // i:number of coding tree layer
        for(int j=0;j<_N;j+=2*m){ // N bits,m bits per node
            std::vector<int>a(m),b(m);
            for(int k=0;k<m;j++){
                a[k]=_code[j+k];
                b[k]=_code[j+k+m];
            }
            for(int k=0;k<m;k++){
                _code[j+k]=a[k]^b[k];
                _code[j+k+m]=b[k];
            }
        }
        m*=2;
    }
}

std::vector<int> encoder::code_output(){
    return _code;
}
