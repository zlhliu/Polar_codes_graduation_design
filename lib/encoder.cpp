#include"encoder.hpp"

// rate:K/N,K=_data.size(),N=_code.size()
encoder::encoder(const std::vector<int>& data,int K,int N,const std::vector<int>& Q){
    _data=data;
    _K=K;
    _N=N;
    code_calculate(Q);
}
encoder::~encoder(){
}

void encoder::code_calculate(const std::vector<int>& Q){
    _code=std::vector<int>(_N,0); // set frozen bits
    for(size_t i=0;i<_K;i++){ // set inf bits
        _code[Q[i+_N-_K]]=_data[i];
    }
    int m = 1; // number of bits combined
    for(size_t i=0;i<log2(_N);i++){ // i:number of coding tree layer
        for(size_t j=0;j<_N;j+=2*m){ // N bits,m bits per node
            std::vector<int>a(m),b(m);
            for(size_t k=0;k<m;k++){
                a[k]=_code[j+k];
                b[k]=_code[j+k+m];
            }
            for(size_t k=0;k<m;k++){
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


