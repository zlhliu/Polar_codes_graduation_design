#ifndef _DEMODULATION_H_
#define _DEMODULATION_H_
#include<bits/stdc++.h>
class demodulation
{
private:
    std::vector<std::complex<double>> _message_through_awgn;
    std::vector<double> _likelyhood_value;
public:
    demodulation(const std::vector<std::complex<double>>& ,double noise_power);
    demodulation();
    ~demodulation();
    std::vector<double> result();
    void start_calculation(const std::vector<std::complex<double>>& message_through_awgn,double noise_power);
};

#endif 