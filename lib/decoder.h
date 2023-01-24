#ifndef _DECODER_H_
#define _DECODER_H_
#include<bits/stdc++.h>

class decoder
{
private:
    std::string _decoder_type; // sc,fsc,scl,fscl
    std::vector<int> _message;
    std::vector<std::vector<int>> _code;
    std::vector<std::vector<double>> _likelyhood_value;
    std::vector<double> _received_likelyhood_value; // store the likelyhood value after demodulated
    void code_calculate();
    void sc_calculate(const std::unordered_set<int>& frozen_channel);
    double f(double x,double y);
    double g(double x,double y,double z);
public:
    decoder(std::vector<double>&& likely_value,std::string decoder_type,const std::unordered_set<int>& frozen_channel);
    ~decoder();
};


#endif