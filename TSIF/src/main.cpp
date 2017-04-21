#include <iostream>
#include <string>
#include <vector>
#include <array>

#include "../include/dataset.h"
using namespace std;

struct Solution{
    char* bits;
    float score; 
  
}; typedef struct Solution Solution;


int main(int argc, char** argv){
  
    string file = "./data/mushroom.dat";
    Dataset _data;
    
    _data.loadFile(file);
    
    Solution s1;
    s1.bits = new char[_data.getnbCols()-1];
    s1.score = 0.0;
    
    return 0;
}