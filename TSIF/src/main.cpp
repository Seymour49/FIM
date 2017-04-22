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
float f1_measure(float tp, float fp, float fn){
    
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


void tidListNeighboor(Solution s, Dataset& data){
  
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
	    
	    vector<int>::iterator it;
	    // TP' = TP(S) inter TP(i) 
	    vector<int> tpN(sTL[0].size());
	    it = set_intersection(sTL[0].begin(), sTL[0].end(), tmpCL[0].begin(), tmpCL[0].end(), tpN.begin());
	    tpN.resize(it-tpN.begin());
	    
	    // vmp = TP \ TP'
	    vector<int> vmp(sTL[0].size() - tpN.size());
	    it = set_difference(sTL[0].begin(), sTL[0].end(), tpN.begin(), tpN.end(), vmp.begin());
	    vmp.resize(it-vmp.begin());
	    
	    // FP' = FP union vmp
	    vector<int> fpN(sTL[1].size() + vmp.size());
	    it = set_union(sTL[1].begin(), sTL[1].end(), vmp.begin(), vmp.end(), fpN.begin());
	    fpN.resize(it-fpN.begin());
	    
	    // FN' = FN(S) inter FN(i)
	    vector<int> fnN(sTL[3].size());
	    it = set_intersection(sTL[3].begin(),sTL[3].end(), tmpCL[3].begin(), tmpCL[3].end(), fnN.begin());
	    fnN.resize(it-fnN.begin());
	    
	    // vmp = FN\FN'
	    vmp.clear(); vmp.shrink_to_fit(); vmp.resize(sTL[3].size() - fnN.size());
	    it = set_difference(sTL[3].begin(), sTL[3].end(), fnN.begin(), fnN.end(), vmp.begin());
	    vmp.resize(it - vmp.begin());
	    
	    // TN' = TN union FN\FN'
	    vector<int> tnN(sTL[2].size() + vmp.size());
	    it = set_union(sTL[2].begin(), sTL[2].end(), vmp.begin(), vmp.end(), tnN.begin());
	    tnN.resize(it - tnN.begin());
	    
	    // Les tidlists des ensemble tp,fp,tn,fn sont calculés. debuggage par somme
// 	    cout << "TPN : " << tpN.size() << ";FPN : " << fpN.size() << ";TNN : " << tnN.size() << ";FNN : " << fnN.size() << ";Total " << tpN.size()+fpN.size()+tnN.size()+fnN.size() << endl;
	    
	    scores.push_back(f1_measure((float)tpN.size(),(float)fpN.size(),(float)fnN.size()));
	    delete []tmp.bits;
	}
	else{
	    // L'item courant appartient à S, on doit donc le supprimer de S et tester ce voisin
	    // On doit donc calculer sur FP(S) les transactions qui contiennent S\i puis les ajouter 
	    // à TP pour obtenir TP' puis FP' (par différence)
	    Solution tmp2;
	    tmp2.nbBits = s.nbBits;
	    tmp2.bits = new char[tmp2.nbBits];
	    for( unsigned k=0; k < tmp2.nbBits; ++k){
		tmp2.bits[k] = s.bits[k];
	    }
	    tmp2.bits[i] = '0';
	    
	    // la copie avec l'item en moins est prête à être traitée
	    
	    // Calcul de TP(tmp2) sur FP(S)n
	    vector<int> vmp2 = data.tidList(tmp2.bits, sTL[1]);
	    // TP' = TP U vmp2
	    vector<int> tpN(sTL[0].size() + vmp2.size());
	    vector<int>::iterator it;
	    
	    it = set_union(sTL[0].begin(), sTL[0].end(), vmp2.begin(), vmp2.end(), tpN.begin());
	    tpN.resize(it - tpN.begin());
	    
	    // FP' = FP \ vmp2
	    vector<int> fpN(sTL[1].size() - vmp2.size());
	    it = set_difference(sTL[1].begin(), sTL[1].end(), vmp2.begin(), vmp2.end(), fpN.begin());
	    fpN.resize(it - fpN.begin());
	    
	    // vmp2 = tidlist(tmp2, TN)
	    vmp2.clear(); vmp2.shrink_to_fit();
	    vmp2 = data.tidList(tmp2.bits, sTL[2]);
	    
	    // FN' = FN U vmp2
	    vector<int> fnN(sTL[3].size() + vmp2.size());
	    it = set_union(sTL[3].begin(), sTL[3].end(), vmp2.begin(), vmp2.end(), fnN.begin());
	    fnN.resize(it - fnN.begin());
	    
	    // TN' = TN \ vmp2
	    vector<int> tnN(sTL[2].size() - vmp2.size());
	    it = set_difference(sTL[2].begin(), sTL[2].end(), vmp2.begin(), vmp2.end(), tnN.begin());
	    tnN.resize(it - tnN.begin());
	    
	    // Les tidlists des ensemble tp,fp,tn,fn sont calculés. debuggage par somme
// 	    cout << "TPN : " << tpN.size() << ";FPN : " << fpN.size() << ";TNN : " << tnN.size() << ";FNN : " << fnN.size() << ";Total " << tpN.size()+fpN.size()+tnN.size()+fnN.size() << endl;
	    scores.push_back(f1_measure((float)tpN.size(),(float)fpN.size(),(float)fnN.size()));
	    delete [] tmp2.bits;
	}
    }
    
    int maxPos = 0;
    float maxVal = scores[0];
    for(unsigned i=1; i < scores.size(); ++i){
	if(scores[i] > maxVal )
	  maxPos = i;
    }
    cout << "Le meilleur voisin revient à fliper l'item " << maxPos << " pour une évaluation à " << maxVal << ". Gain = " << maxVal - s.score << endl; 
}


void naiveNeighboor(Solution s, Dataset& data){
    
    vector<float> scores;
    
    Solution tmp;
    tmp.nbBits = s.nbBits;
    tmp.bits = new char[tmp.nbBits];
    
    for(unsigned i=0; i < tmp.nbBits; ++i){
	tmp.bits[i] = s.bits[i];
    }
    
    for(unsigned i=0; i < tmp.nbBits; ++i){
	if(tmp.bits[i] == '0'){
	    tmp.bits[i] = '1';
	    vector<int>cm = data.confusionMatrix(tmp.bits);
	    
	    scores.push_back(f1_measure((float)cm[0], (float)cm[1], (float)cm[3]));
	    tmp.bits[i] = '0';
	}
	else{
	    tmp.bits[i] = '0';
	    if(getK(tmp) != 0){
		vector<int>cm = data.confusionMatrix(tmp.bits);
		
		scores.push_back(f1_measure((float)cm[0], (float)cm[1], (float)cm[3]));
	    }
	    else{
		scores.push_back(0.0);
	    }
	    tmp.bits[i] = '1';
	}
    }
    
    int maxPos = 0;
    float maxVal = scores[0];
    for(unsigned i=1; i < scores.size(); ++i){
	if(scores[i] > maxVal )
	  maxPos = i;
    }
    cout << "Le meilleur voisin revient à fliper l'item " << maxPos << " pour une évaluation à " << maxVal << ". Gain = " << maxVal - s.score << endl; 
    delete [] tmp.bits;
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
//     s1.bits[0] = '0';
    s1.bits[108] = '1';
//     s1.bits[25] = '1';
    s1.score = 0.0;
    
    cout << "F(s1) : " << perso_measure(s1, _data, 100, 40) << endl;
    vector<int> CM = _data.confusionMatrix(s1.bits);
    s1.score = f1_measure((float)CM[0], (float)CM[1], (float)CM[3]);
    
  
//     tidListNeighboor(s1,_data);
    naiveNeighboor(s1, _data);
    
    
    delete[] s1.bits;

    return 0;
}