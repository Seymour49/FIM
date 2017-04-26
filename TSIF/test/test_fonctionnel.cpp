#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <ctime>

#include "../include/dataset.h"
using namespace std;

struct Solution{
    unsigned nbBits;
    char* bits;
    float score; 
  
}; typedef struct Solution Solution;


/**
 * Fonction affichage en base 10 du bitset de la solution passée en paramètre
 */
void displaySolution(const Solution &s){
    
    for(unsigned i=0; i < s.nbBits; ++i){
	if( s.bits[i] == '1'){
	    cout << i+3 << " ";
	}
    }
    cout << endl;
//     cout << endl << "Score : " << s.score << endl;
}


/**
 * Fonction retournant le nombre de bits à 1 dans la solution
 * passée en paramètre
 */
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
float perso_measure(int k ,float tp, float fn, unsigned minS, unsigned maxS){      
    return k*( (tp/(2*minS))+ (maxS/(2*fn)) );
}


/**
 * Comparaison de deux voisins selon leur score
 */
static bool compareGain(const pair<int,float> a, const pair<int,float> b){
	return a.second > b.second;
}

/**
 * Fonction d'évaluation du voisinage par union et intersection sur les tidlists
 * des différents ensembles.
 * Retourne la paire <pos,score> correspondant à l'item à fliper et au score qui lui
 * est associé.
 */
pair<unsigned,float> tidListNeighboor(Solution s, Dataset& data, vector<int> TL, int iter, float best){
  
    vector<pair<unsigned,float>> scores;
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
	    
	    // Les tidlists des ensemble tp,fp,tn,fn sont calculés.
	    scores.push_back(make_pair(i,f1_measure((float)tpN.size(),(float)fpN.size(),(float)fnN.size())));
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
	    scores.push_back(make_pair(i,f1_measure((float)tpN.size(),(float)fpN.size(),(float)fnN.size())));
	    delete [] tmp2.bits;
	}
    }
    
    // Sélection du meilleur voisin non-tabou (avec mécanisme d'aspiration)
    sort(scores.begin(), scores.end(), compareGain);
    
    // Mélange des différents clusters d'éléments de scores ayant la même valeur (i.e garde fou pour choix aléatoire des égalités)
    vector<int> part;
    
    part.push_back(0);
    for(unsigned i=1; i < scores.size(); ++i){
	if( scores[i].second < scores[i-1].second ){
	    part.push_back(i);
	}
    }
    
    part.push_back(scores.size()+1);
    for(unsigned i=1; i < part.size(); ++i){
	if( (part[i] - part[i-1]) > 1 ){
	    random_shuffle(scores.begin()+part[i-1], scores.begin()+part[i] - 1);
	}
    }
    
    bool chosen = false;
    unsigned pos = 0;
    while ( !chosen && (pos < scores.size()) ){
      
	// Mouvement interdit par la liste tabou
	if( TL[scores[pos].first] > iter ){
	    
	    // Mécanisme d'aspiration
	    if( scores[pos].second > best ){
		chosen = true;
	    }
	    else{
		++pos;
	    }
	}
	else{
	    chosen = true;
	}
    }
    
//     cout << "Le meilleur voisin revient à fliper l'item " << scores[pos].first << " pour une évaluation à " << scores[pos].second << ". Gain = " << scores[pos].second - s.score << endl; 
    
    return scores[pos]; 
}

/**
 * Fonction d'évaluation du voisinage via scan complet de data pour chaque voisin
 * de s.
 * Retourne la paire <pos,score> correspondant à l'item à fliper et au score qui lui
 * est associé.
 */
pair<unsigned, float> naiveNeighboor(Solution s, Dataset& data, vector<int> &TL, int iter, float best){
    
    vector<pair<unsigned,float>> scores;
    
    // Copie de s dans tmp
    Solution tmp;
    tmp.nbBits = s.nbBits;
    tmp.bits = new char[tmp.nbBits];
    
    for(unsigned i=0; i < tmp.nbBits; ++i){
	tmp.bits[i] = s.bits[i];
    }
    
    // Pour chacun des bits on teste sa valeur et on évalue le voisin correspondant
    for(unsigned i=0; i < tmp.nbBits; ++i){
	if(tmp.bits[i] == '0'){
	    tmp.bits[i] = '1';
	    vector<int>cm = data.confusionMatrix(tmp.bits);
	    
	    scores.push_back(make_pair(i,f1_measure((float)cm[0], (float)cm[1], (float)cm[3])));
	    tmp.bits[i] = '0';
	}
	else{
	    tmp.bits[i] = '0';
	    if(getK(tmp) != 0){
		vector<int>cm = data.confusionMatrix(tmp.bits);
		
		scores.push_back(make_pair(i,f1_measure((float)cm[0], (float)cm[1], (float)cm[3])));
	    }
	    else{
		scores.push_back(make_pair(i,0.0));
	    }
	    tmp.bits[i] = '1';
	}
    }
    
    // Sélection du meilleur voisin non-tabou (avec mécanisme d'aspiration)
    sort(scores.begin(), scores.end(), compareGain);
    
    // Mélange des différents clusters d'éléments de scores ayant la même valeur (i.e garde fou pour choix aléatoire des égalités)
    vector<int> part;
    
    part.push_back(0);
    for(unsigned i=1; i < scores.size(); ++i){
	if( scores[i].second < scores[i-1].second ){
	    part.push_back(i);
	}
    }
    
    part.push_back(scores.size()+1);
    for(unsigned i=1; i < part.size(); ++i){
	if( (part[i] - part[i-1]) > 1 ){
	    random_shuffle(scores.begin()+part[i-1], scores.begin()+part[i] - 1);
	}
    }
    
    bool chosen = false;
    unsigned pos = 0;
    while ( !chosen && (pos < scores.size()) ){
      
	// Mouvement interdit par la liste tabou
	if( TL[scores[pos].first] > iter ){
	    
	    // Mécanisme d'aspiration
	    if( scores[pos].second > best ){
// 		cout << "Aspiration sur item " << scores[pos].first << endl;
		chosen = true;
	    }
	    else{
		++pos;
	    }
	}
	else{
	    chosen = true;
	}
    }
    
//     cout << "Le meilleur voisin revient à fliper l'item " << scores[pos].first << " pour une évaluation à " << scores[pos].second << ". Gain = " << scores[pos].second - s.score << endl; 
    
    delete [] tmp.bits;
    
    return scores[pos];
}

/**
 * Fonction d'initialisation de la solution passée en paramètre
 */
void randomInit(Solution *s, Dataset & data){  

    s->nbBits = data.getnbCols() -1;
    s->bits = new char[s->nbBits];
    for(unsigned i = 0; i < s->nbBits; ++i) s->bits[i] = '0';
    // Assignation aléatoire de 3 bits à 1 TODO à revoir avec encadrants
    for(unsigned i=0; i < 1;++i){
	int randomPos = rand() % s->nbBits;
	s->bits[108] = '1';

// 	s->bits[randomPos] = '1';
    }
    
    vector<int>CMS = data.confusionMatrix(s->bits);
    s->score = f1_measure((float)CMS[0], (float)CMS[1], (float)CMS[3]);
}


/**
 * Copie de s1 vers s2
 */ 
void properCopy(Solution &s1, Solution* s2){

    s2->nbBits = s1.nbBits;
    s2->bits = new char[s2->nbBits];
    for(unsigned i=0; i < s2->nbBits; ++i) s2->bits[i] = s1.bits[i];
    s2->score = s1.score;
}


void validNeighboor(Solution s, Dataset& data){
    // Copie de s dans tmp
    Solution tmp;
    tmp.nbBits = s.nbBits;
    tmp.bits = new char[tmp.nbBits];
    
    for(unsigned i=0; i < tmp.nbBits; ++i){
	tmp.bits[i] = s.bits[i];
    }
    
    // Pour chacun des bits on teste sa valeur et on évalue le voisin correspondant
    for(unsigned i=0; i < tmp.nbBits; ++i){
	if(tmp.bits[i] == '0'){
	    tmp.bits[i] = '1';
	    displaySolution(tmp);
	    tmp.bits[i] = '0';
	}
	else{
	    tmp.bits[i] = '0';
	    displaySolution(tmp);
	    tmp.bits[i] = '1';
	}
	

    }
  
    delete []tmp.bits;
}


int main(int argc, char** argv){

    /*	Tests réalisés sur bitset 3/5/8 avec classe positive si le premier item est
     * 	2, classe négative si 1.
     * 	Fichier debug.dat utilisé, résultats fonctionnels ok
     *	Changement valeur dans dataset.cpp line 99 
     */
    
    // Chargement du fichier de données à traiter
    string file = "./data/debug.dat";
    Dataset _data;
    _data.loadFile(file);
    
    // Initialisation de l'aléatoire
    srand(time(NULL));
    
    // Déclaration et initialisation de la première solution
    Solution _sCurrent;
    _sCurrent.nbBits = _data.getnbCols() - 1;
    _sCurrent.bits = new char[_sCurrent.nbBits];
    for(unsigned i=0; i < _sCurrent.nbBits; ++i) _sCurrent.bits[i] = '0';
    
    _sCurrent.bits[0] = '1'; _sCurrent.bits[2] = '1'; _sCurrent.bits[5] = '1';
    
    for(unsigned i=0; i < _data.getnbRows(); ++i){
	if( _data.include(_sCurrent.bits, i) ){
	    displaySolution(_sCurrent);
	    cout << "est contenu dans la transaction " << i << endl;
	}
    }
    
    
    vector<int> CM = _data.confusionMatrix(_sCurrent.bits);
    vector<vector<int>> CL = _data.confusionLists(_sCurrent.bits);
    cout << "TP = " << CM[0] << "; FP = " << CM[1] << "; TN = " << CM[2] << "; FN = " << CM[3] << endl;
    
    for(unsigned i=0 ; i < 4; ++i){
	for(unsigned j=0; j < CL[i].size(); ++j){
	    cout << " " << CL[i][j];
	}
	cout << endl;
    }
    
    validNeighboor(_sCurrent,_data);
    
    delete []_sCurrent.bits;
    return 0;
}