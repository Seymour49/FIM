#include <iostream>
#include <string>
#include <vector>
#include <array>

#include "../include/dataset.h"
using namespace std;

struct Solution{
    unsigned nbBits;
    char* bits;
    float score; 
  
}; typedef struct Solution Solution;

int getK(Solution s){
    int result = 0;
    
    for(unsigned i=0; i < s.nbBits; ++i){
	if( s.bits[i] == '1' )
	    ++result;
    }  
    return result;
}

/**
 * Fonction calculant la f_mesure de la matrice de confusion passée en paramètre
 * tp = cm[0], fp = cm[1], tn = cm[2], fn = cm[3]
 */
float f1_measure(Solution s, Dataset& data){
  
    vector<int>cm = data.confusionMatrix(s.bits);
    
    float tp = (float)cm[0];
    float fp = (float)cm[1];
    float fn = (float)cm[3];
    
    cout << "tp = " << tp << "; fp = " << fp << "; fn = " << fn << endl;
    
    return (2*tp)/(2*tp + fp + fn);  
}

/**
 * Fonction évaluation proposée
 */
float perso_measure(Solution s, Dataset& data, unsigned minS, unsigned maxS){  
    int k = getK(s);
    
    vector<int>cm = data.confusionMatrix(s.bits);
    float tp = (float)cm[0];
    float fn = (float)cm[3];
    
    return k*( (tp/(2*minS))+ (maxS/(2*fn)) );
}


int main(int argc, char** argv){
  
    string file = "./data/mushroom.dat";
    Dataset _data;
    
    _data.loadFile(file);
    
    Solution s1, s2;
    unsigned nbB = _data.getnbCols()-1;
    
    s1.bits = new char[nbB];
    s1.nbBits = nbB;
    for(unsigned k=0; k < nbB; ++k) s1.bits[k] = '0';
    s1.bits[0] = '1';
    s1.score = 0.0;
    
    s2.bits = new char[nbB];
    s2.nbBits = nbB;
    for(unsigned k=0; k < nbB; ++k) s2.bits[k] = '0';
    s2.score = 0.0;
    s2.bits[10] = '1';

    cout << "F_1(s1) : " << f1_measure(s1,_data) << endl;
    cout << "F(s1) : " << perso_measure(s1, _data, 100, 40) << endl;
    cout << "F_1(s2) : " << f1_measure(s2, _data) << endl;
    cout << "F(s2) : " << perso_measure(s2, _data, 100, 40) << endl;
    delete[] s1.bits;
    delete[] s2.bits;
    return 0;
}