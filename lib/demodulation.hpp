#ifndef _DEMODULATION_H_
#define _DEMODULATION_H_
#include<bits/stdc++.h>
class demodulation
{
private:
    std::vector<std::complex<double>> _message_through_awgn;
    std::vector<double> _likelyhood_value;
public:
    demodulation(std::vector<std::complex<double>>&& ,double noise_power);
    ~demodulation();
    std::vector<double> result();
};

#endif 