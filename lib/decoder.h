#ifndef _DECODER_H_
#define _DECODER_H_
#include<bits/stdc++.h>

class decoder
{
private:
    std::string _decoder_type;
    std::vector<int> _message;
    std::vector<double> _likelyhood_value; // store the likelyhood value after demodulation
    void calculate();
    void sc_calculate(const std::vector<int>& Q);
public:
    decoder(std::vector<double>&& likely_value,std::string decoder_type,const std::vector<int>& Q);
    ~decoder();
};


#endif