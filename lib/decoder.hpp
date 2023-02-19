#ifndef _DECODER_H_
#define _DECODER_H_
#include<bits/stdc++.h>

#include"thread_pool.hpp"
#include"fast_node.hpp"

class decoder
{
private:
    std::vector<int> _message;
    std::vector<std::vector<int>> _code; // decoding result of every layer 
    std::vector<std::vector<double>> _likelyhood_value; // likelyhood value of every layer
    std::unordered_set<int> _frozen_channel;
    std::vector<std::vector<fast_node_state>> _fast_node_type;

    void code_calculate();
    void sc_calculate(const std::unordered_set<int>& frozen_channel);
    void fsc_calculate(const std::unordered_set<int>& frozen_channel);
    
public:
    std::vector<int> result(const std::vector<int>& Q);
    decoder(const std::vector<double>& likely_value,const std::unordered_set<int>& frozen_channel,std::string decoder_type);
    decoder(std::vector<std::vector<fast_node_state>>&&);
    decoder();
    ~decoder();
    void start_calculation(const std::vector<double>& likely_value,const std::unordered_set<int>& frozen_channel,std::string decoder_type);
};


#endif