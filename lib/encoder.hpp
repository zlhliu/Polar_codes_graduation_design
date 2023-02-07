#ifndef _ENCODER_H_
#define _ENCODER_H_
#include<bits/stdc++.h>

class encoder
{
private:
    std::vector<int> _code;
    int _N; // number of information bits
    int _K; // number of code
    void code_calculate(const std::vector<int>& Q,const std::vector<int>& data);
public:
    encoder();
    encoder(const std::vector<int>& data,int K,int N,const std::vector<int>& Q);
    void start_calculation(const std::vector<int>& data,int K,int N,const std::vector<int>& Q);
    ~encoder();
    std::vector<int> code_output();
};


#endif