#include<bits/stdc++.h>
int main() {
    std::ofstream data;
    data.open("D:/polar_code/data/data.txt");
    
    std::vector<int> x,y;
    if (data.is_open())
    {
        data << "This is a line.\n";
        data << "This is another line.\n";
        data.close();
    }
    return 0;
} 

