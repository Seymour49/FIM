#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

class Dataset {
  
private:
    unsigned int _nbCols;
    unsigned int _nbRows;
    
    char** _Matrice;
    
    
public:
  
    Dataset();    
    Dataset(unsigned c, unsigned r);
    
    ~Dataset();
    
    void loadFile(const std::string& filename);
    
    unsigned int getnbRows() const { return _nbRows; }
    unsigned int getnbCols() const { return _nbCols; }
    
    
    std::vector<int> confusionMatrix(char* bitset);
    
  
};

std::vector< std::string > explode2(const std::string& str);

#endif