#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

class Dataset {
  
private:
    long long unsigned int _nbCols;
    long long unsigned int _nbRows;
    
    std::vector<std::string> _comment;
    
    char** _Matrice;
    
    int reverseFlag;
public:
  
    Dataset();    
    Dataset(unsigned c, unsigned r, int rFlag);
    
    ~Dataset();
    
    void loadFileInteger(const std::string& filename);
    
    void loadFileBinary(const std::string & filename);
    
    unsigned int getnbRows() const { return _nbRows; }
    unsigned int getnbCols() const { return _nbCols; }
    
    std::vector<std::string> getComment() const { return _comment;}
    
    long long unsigned getNBTCP();
    
    char getBit(unsigned r, unsigned c) const { return _Matrice[r][c]; }
    
    void setReverseFlag(int reverseClass_flag);
    
    void encodeInteger(const std::string& filename);
    
    void clearDataset(char* bitset);
    
    /**
     * Retourne la matrice de confusion du bitset passé en paramètre
     * sous forme d'un vecteur de 4 entiers :
     * TP, FP, TN, FN
     */
    std::vector<int> confusionMatrix(char* bitset);
    
    /**
     * Retourne les tidLists des 4 ensembles TP, FP, TN, FN du 
     * bitset passé en paramètre 
     */
    std::vector<std::vector<int>> confusionLists(char* bitset);
    
    /**
     * Retourne la tidList du bitset passé en paramètre vis à vis
     * de l'ensemble de transactions représenté par le vecteur
     * d'entiers en paramètre
     */
    std::vector<long long int> tidList(char* bitset, std::vector<long long int> tid);
    
    /**
     * Fonction booléenne retournant vrai si le bitset passé en paramètre
     * est contenu dans la i-ème transaction du jeu de données
     */
    bool include(char *bitset, unsigned int i);
    
  
};

std::vector< std::string > explode2(const std::string& str);

#endif