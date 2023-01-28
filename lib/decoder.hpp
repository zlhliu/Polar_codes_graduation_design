#ifndef _DECODER_H_
#define _DECODER_H_
#include<bits/stdc++.h>

class decoder
{
private:
    std::string _decoder_type; // sc,fsc,scl,fscl
    
    std::vector<int> _message;
    std::vector<std::vector<int>> _code; // decoding result of every layer 
    std::vector<std::vector<double>> _likelyhood_value; // likelyhood value of every layer
    std::unordered_set<int> _frozen_channel;
    void code_calculate();
    void sc_calculate();
    double f(double x,double y);
    double g(double x,double y,double z);
public:
    std::vector<int> result();
    decoder(std::vector<double>&& likely_value,std::unordered_set<int>&& frozen_channel,std::string decoder_type);
    ~decoder();
};


#endif