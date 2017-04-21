#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
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


void bestNeighboor(Solution s, Dataset& data){
  
    vector<float> scores;
    vector<Solution> voisins;
    
    // TP = 0, FP = 1, TN, = 2, FN = 3
    vector<vector<int>> sTL = data.confusionLists(s.bits);
    
    // Il y a N voisin
    for(unsigned i=0; i < s.nbBits; ++i){
      
	// Si s[i] = 0, alors le voisin correspond à l'ajout de l'item
	// Dans ce cas, voici les traitements à effectuer :
	// TP' = TP n tidlist(i) puis FP' = FP u (TP\TP')
	// FN' = FN n tidlist(i) puis TN' = TN u (FN\FN')
	if( s.bits[i] == '0'){
	    // Récupération des tidlists de l'1-itemset à ajouter
	    Solution tmp;
	    tmp.nbBits = s.nbBits;
	    tmp.bits = new char[tmp.nbBits];
	    for(unsigned k=0; k < tmp.nbBits; ++k){
		tmp.bits[k] = '0';
	    }
	    tmp.bits[i] = '1';
	    vector<vector<int>> tmpCL = data.confusionLists(tmp.bits);
	    vector<int> tpN(sTL[0].size());
	    vector<int> fpN(sTL[0].size()+sTL[1].size());
	    vector<int> tnN(sTL[2].size()+sTL[3].size());
	    vector<int> fnN(sTL[3].size());
	    
	    vector<int>::iterator it;
	    // TP' = TP(S) inter TP(i) 
	    it = set_intersection(sTL[0].begin(), sTL[0].end(), tmpCL[0].begin(), tmpCL[0].end(), tpN.begin());
	    tpN.resize(it-tpN.begin());
	    
	    // vmp = TP \ TP'
	    vector<int> vmp(sTL[0].size() - tpN.size());
	    it = set_difference(sTL[0].begin(), sTL[0].end(), tpN.begin(), tpN.end(), vmp.begin());
	    vmp.resize(it-vmp.begin());
	    
	    // FP' = FP union vmp
	    it = set_union(sTL[1].begin(), sTL[1].end(), vmp.begin(), vmp.end(), fpN.begin());
	    fpN.resize(it-fpN.begin());
	    
	    // FN' = FN(S) inter FN(i)
	    it = set_intersection(sTL[3].begin(),sTL[3].end(), tmpCL[3].begin(), tmpCL[3].end(), fnN.begin());
	    fnN.resize(it-fnN.begin());
	    
	    // vmp = FN\FN'
	    vmp.clear(); vmp.shrink_to_fit(); vmp.resize(sTL[3].size() - fnN.size());
	    it = set_difference(sTL[3].begin(), sTL[3].end(), fnN.begin(), fnN.end(), vmp.begin());
	    vmp.resize(it - vmp.begin());
	    
	    // TN' = TN union FN\FN'
	    it = set_union(sTL[2].begin(), sTL[2].end(), vmp.begin(), vmp.end(), tnN.begin());
	    tnN.resize(it - tnN.begin());
	    
	    // Les tidlists des ensemble tp,fp,tn,fn sont calculés. debuggage par somme
	    cout << "Total " << tpN.size()+fpN.size()+tnN.size()+fnN.size() << endl;
	    
	    delete []tmp.bits;
	}
	
	else{
	  
	}
    }
    
   
  
}


int main(int argc, char** argv){
  
    string file = "./data/mushroom.dat";
    Dataset _data;
    
    _data.loadFile(file);
    
    Solution s1;
    unsigned nbB = _data.getnbCols()-1;
    
    s1.bits = new char[nbB];
    s1.nbBits = nbB;
    for(unsigned k=0; k < nbB; ++k) s1.bits[k] = '0';
    s1.bits[0] = '1';
    s1.bits[25] = '1';
    s1.score = 0.0;
    
    cout << "F_1(s1) : " << f1_measure(s1,_data) << endl;
    cout << "F(s1) : " << perso_measure(s1, _data, 100, 40) << endl;
    
    vector<vector<int>> CL = _data.confusionLists(s1.bits);
    for(unsigned i=0; i < CL.size(); ++i){
	cout << " " << CL[i].size();
    }
    cout << endl;
    
    vector<int> v1,v2,v4;
    vector<int> v3(20);
    vector<int>::iterator it;
    for(int w=0; w<10;++w){
	v1.push_back(w);
	if(w%2 == 0){
	    v2.push_back(w);
	}
	else{
	    v4.push_back(w);
	}
    }
    
    bestNeighboor(s1,_data);
    /*
    it = set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), v3.begin());
    v3.resize(it-v3.begin());
    for(unsigned n=0; n < v3.size(); ++n)
	cout << v3[n] << " ";
    cout << endl;
    it = set_difference(v1.begin(), v1.end(), v4.begin(), v4.end(), v3.begin());
    v3.resize(it-v3.begin());
    for(unsigned n=0; n < v3.size(); ++n)
	cout << v3[n] << " ";*/
    delete[] s1.bits;

    return 0;
}