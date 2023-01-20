#ifndef _ENCODER_H_
#define _ENCODER_H_
#include<bits/stdc++.h>

class encoder
{
private:
    std::vector<int>_Q;
    std::vector<int> _data;
    std::vector<int> _code;
    int _N; // number of information bits
    int _K; // number of code
    void code_calculate();
public:
    encoder(const std::vector<int> data,int K,int N);
    ~encoder();
    std::vector<int> code_output();
};


#endif