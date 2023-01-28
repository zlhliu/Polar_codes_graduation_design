#include"demodulation.hpp"

demodulation::demodulation(std::vector<std::complex<double>>&& message_through_awgn,double noise_power){
    _message_through_awgn=message_through_awgn;
    _likelyhood_value=std::vector<double>(_message_through_awgn.size()*2);
    // 
    for(size_t i=0;i<_message_through_awgn.size();i++){
        _likelyhood_value[2*i]=_message_through_awgn[i].imag()*sqrt(2)/noise_power;
        _likelyhood_value[2*i+1]=_message_through_awgn[i].real()*sqrt(2)/noise_power;
    }
}

std::vector<double> demodulation::result(){
    return _likelyhood_value;
}
demodulation::~demodulation(){}