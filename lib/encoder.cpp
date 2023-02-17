#include"encoder.hpp"
// TODO:把所有class都换成无参数初始化,只使用一个实例
// rate:K/N,K=_data.size(),N=_code.size()
// TODO::_code的初始化为啥不行

#define total_bits 1024
encoder::encoder(){
    _code=std::vector<int>(total_bits); // set frozen bits
<<<<<<< HEAD
    int a=0;
=======
>>>>>>> 936f85f (累了，先这样吧)
}
encoder::encoder(const std::vector<int>& data,int K,int N,const std::vector<int>& Q){
    _N=N;
    _K=K;
    _code=std::vector<int>(_N,0); // set frozen bits
    code_calculate(Q,data);
}
encoder::~encoder(){
}
void encoder::start_calculation(const std::vector<int>& data,int K,int N,const std::vector<int>& Q){
    _K=K;
    _N=N;
    code_calculate(Q,data);
}
void encoder::code_calculate(const std::vector<int>& Q,const std::vector<int>& data){
    for(auto&&d:_code){
        d=0;
    }
    for(size_t i=0;i<_K;i++){ // set inf bits
        _code[Q[i+_N-_K]]=data[i];
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

<<<<<<< HEAD
std::vector<int> encoder::code_output(){
=======
std::vector<int> encoder::code_output()const{
>>>>>>> 936f85f (累了，先这样吧)
    return _code;
}


