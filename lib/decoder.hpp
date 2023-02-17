#ifndef _DECODER_H_
#define _DECODER_H_
#include<bits/stdc++.h>
<<<<<<< HEAD

=======
#include<unordered_set>
>>>>>>> 936f85f (累了，先这样吧)
class decoder
{
private:
    std::string _decoder_type; // sc,fsc,scl,fscl
    std::vector<int> _message;
    std::vector<std::vector<int>> _code; // decoding result of every layer 
    std::vector<std::vector<double>> _likelyhood_value; // likelyhood value of every layer
    std::unordered_set<int> _frozen_channel;
    void code_calculate();
<<<<<<< HEAD
    void sc_calculate(const std::unordered_set<int>& frozen_channe);
=======
    void sc_calculate(const std::unordered_set<int>& frozen_channel);
    // void fsc_calculate(const std::unordered_set<int>& frozen_channel);
>>>>>>> 936f85f (累了，先这样吧)
public:
    std::vector<int> result(const std::vector<int>& Q);
    decoder(const std::vector<double>& likely_value,const std::unordered_set<int>& frozen_channel,std::string decoder_type);
    decoder();
    ~decoder();
    void start_calculation(const std::vector<double>& likely_value,const std::unordered_set<int>& frozen_channel,std::string decoder_type);
};


#endif