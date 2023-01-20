#define _USE_MATH_DEFINES
#include<bits/stdc++.h>
#include"lib/encoder.h"

#define ln(x)  log10(x)/log10(M_E)



int main() {
    int N=512; // 512 inf bits  
    int K=1024; // 1024 total bits
    int signal_power=1; // signal power
    int num_snr=30; 
    std::vector<double> snr(num_snr); // 10log10(signal_powe/noise_power)
    for(int i=0;i<snr.size();i++)snr[i]=(double)i/10;
    std::vector<int> message(N);

    // simulate num_snr times
    for(int times=0;times<num_snr;times++){
        {// generate random message
            srand(time(NULL));
            for(auto&&d:message){
                d=rand()%2;
            }
        }
        
        encoder my_encoder(message,512,1024); // coding message
        std::vector<std::complex<double>> message_through_awgn(K/2); // message through awgn channel
        {// modulation mode:using QPSK
            // 00:e^i45°, 01:e^i135°,11:e^i225°,10:e^i270°
            for(int i=0;i<my_encoder.code_output().size();i+=2){
                if(my_encoder.code_output()[i]==0&&my_encoder.code_output()[i+1]==0){
                    message_through_awgn[i/2].real(sqrt(2));
                    message_through_awgn[i/2].imag(sqrt(2));
                } 
                else if(my_encoder.code_output()[i]==0&&my_encoder.code_output()[i+1]==1){
                    message_through_awgn[i/2].real(-sqrt(2));
                    message_through_awgn[i/2].imag(sqrt(2));
                }
                else if(my_encoder.code_output()[i]==1&&my_encoder.code_output()[i+1]==0){
                    message_through_awgn[i/2].real(-sqrt(2));
                    message_through_awgn[i/2].imag(-sqrt(2));
                }
                else if(my_encoder.code_output()[i]==1&&my_encoder.code_output()[i+1]==1){
                    message_through_awgn[i/2].real(sqrt(2));
                    message_through_awgn[i/2].imag(-sqrt(2));
                }
            }
        }

        {// awgn channel
            double noise_power=signal_power/(1/(pow(10,snr[times]/10)));
            for(auto&&d:message_through_awgn){
                // Box–Muller algorithm generate N(0,1)
                // X1 is uncorrelated to X2
                double U1=(double)(rand()%100)/100; // U1~U(0,1)
                double U2=(double)(rand()%100)/100; // U2~U(0,1)
                double X1=sqrt(-2*ln(U1)) * cos(2 * M_PI * U2); // X1~N(0,1)
                double X2=sqrt(-2*ln(U1)) * sin(2 * M_PI * U2); // X2~N(0,1)
                // generate I~N(μ,σ^2),μ==0，σ^2==noise_power==signal_power/(10^(snr/10))^(-1)
                // the power should be divided equally
                // I=σ/sqrt(2)*X1+μ
                if(d.real()==sqrt(2))d.real(d.real()+(noise_power/sqrt(2))*X1+sqrt(2));
                else d.real(d.real()+(noise_power/sqrt(2))*X1-sqrt(2));
                if(d.imag()==sqrt(2))d.imag(d.real()+(noise_power/sqrt(2))*X1+sqrt(2));
                else d.imag(d.imag()+(noise_power/sqrt(2))*X1-sqrt(2));
            }
        }

        // {// store data
        //     std::vector<double> x(100),y(100);
        //     for(int i=0;i<x.size();i++){
        //         x[i]=i;
        //         y[i]=sin(i);
        //     }
        //     std::ofstream data;
        //     data.open("D:/polar_code/data/data.txt");
        //     if (data.is_open()){
        //         for(int i=0;i<x.size();i++){
        //             data<<x[i]<<' '<<y[i]<<'\n';
        //         }
        //         data.close();
        //     }
        //     else{
        //         std::cout<<"D:/polar_code/data/data.txt !is_open";
        //     }
        // }
    }

    return 0;
}