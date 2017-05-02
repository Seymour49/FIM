#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <ctime>
#include <getopt.h>

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
    cout << endl << "Score : " << s.score << endl;
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

    // Sélection d'une transaction de manière aléatoire
    int randTrans = rand() % data.getnbRows();
    
    // Initialisation de s
    s->nbBits = data.getnbCols() -1;
    s->bits = new char[s->nbBits];
    for(unsigned i = 0; i < s->nbBits; ++i) s->bits[i] = data.getBit(randTrans, i+1);
    
    // Extraction des positions à 1 dans s puis mélange
    vector<unsigned> bits;
    for(unsigned i=0; i < s->nbBits; ++i){
	if(s->bits[i] == '1'){
	    bits.push_back(i);
	}
    }
    
    int nbItem = rand() % (bits.size()/2) + (bits.size()/2);
    
    // Mélange des positions 
    random_shuffle(bits.begin(), bits.end());
    
    for(int i=0; i < nbItem ;++i){
	s->bits[bits[i]] = '0';
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


/**
 * Fonction de perturbation de la solution passée en paramètre
 */
void perturbate(Solution* S){
    displaySolution(*S);
}


Solution tabuSearch(Solution & s0, Dataset &_data, double remainingTime, unsigned maxIt, int tabuTenure){
 
    Solution _sCurrent, _SB;
    properCopy(s0, &_sCurrent);
    properCopy(_sCurrent, &_SB);
    
    time_t start = time(NULL);
    unsigned d = 0;
    int currentIt = 0;
    
    // Initialisation de la liste tabou
    vector<int> TabuList(_SB.nbBits, 0);
    
    do{
	// Sélection du meilleur voisin 
	pair<unsigned, float> bestN;
	bestN = naiveNeighboor(_sCurrent, _data, TabuList, currentIt, _SB.score);
	
	// Mise à jour TabuList
	TabuList[bestN.first] = currentIt+tabuTenure;
	
	// Changement de la solution courante et de son score
	if( _sCurrent.bits[bestN.first] == '0'){
	    _sCurrent.bits[bestN.first] = '1';
	}
	else{
	    _sCurrent.bits[bestN.first] = '0';
	}
	_sCurrent.score = bestN.second;
	
	// Comparaison avec la meilleure solution
	if( _sCurrent.score > _SB.score ){
	    // Copie de la solution courante vers la meilleure et remise à 0 de noUpIt
	    for(unsigned m=0; m < _sCurrent.nbBits; ++m)
		_SB.bits[m] = _sCurrent.bits[m];
	    _SB.score = _sCurrent.score;
	    
	    d = 0;
	}
	else{
	    ++d;
	}
	++currentIt;
      
      
    }while( (d < maxIt) && (difftime(time(NULL),start) < remainingTime) );
    
    delete [] _sCurrent.bits;
    return _SB;
}


int main(int argc, char** argv){

    // Initialisation de l'aléatoire
    srand(time(NULL));
  
    /*
     * Gestion des arguments
     */
    string file = "mushroom.dat"; 		// JDD par défaut
    double AllocTime = 30;			// Temps en seconde
    unsigned maxNoUpIt = 300;			// Max mouvement voisinage sans amélioration avant arrêt
    int tabuTenure = 15;			// TabuTenure
    int maxTS = 10;				// Maximum TS sans amélioration
    
    unsigned minS = 5000;			// Valeur du seuil minimal
    unsigned maxS = 250;			// Valeur du seuil maximal
    
    // Flag pour fonction évaluation
    static int evaluate_flag = 0;		// 0 = f1_measure, 1 = perso_measure
    static int reverseClass_flag = 0;		// 0 = pas d'inversion, 1 = inversion
    
    while(1){
	int opt;
	
	// Déclarations des long_options
	static struct option long_options[] = {
	    
	    /* Flags, i.e pas de version courte */
	    {"f1_measure", no_argument, &evaluate_flag, 0},
	    {"perso_measure", no_argument, &evaluate_flag, 1},
	    
	    {"reverseC", no_argument, &reverseClass_flag, 1},
	    
	    
	    /* Options avec version courtes */
	    {"allocTime", required_argument, 0, 't'},
	    {"dataFile", required_argument, 0, 'd'},
	    {"tabuTenure", required_argument, 0, 'b'},
	    {"maxNoUpTS", required_argument, 0, 'n'},
	    {"maxTS", required_argument, 0, 's'},
	    {"minS", required_argument, 0, 'l'},
	    {"maxS", required_argument, 0, 'u'},
	    {0,0,0,0}
	};
	
	// getopt_long recupere l'option ici
	int option_index = 0;
	
	opt = getopt_long(argc,argv, "t:d:b:n:s:l:u:", long_options, &option_index);
	
	// fin des options
	if(opt == -1)
	    break;
	
	switch(opt){
	    
	    //Gestion des flags
	    case 0:
		if(long_options[option_index].flag != 0)
		  break;
		
	    case 't':
		  AllocTime = atof(optarg);
		  break;
	    case 'd':
		  file = string(optarg);
		  break;
	    case 'b':
		  tabuTenure = atoi(optarg);
		  break;  
	    case 'n':
		  maxNoUpIt = atoi(optarg);
		  break;
	    case 's':
		  maxTS = atoi(optarg);
		  break;  
	    case 'l':
		  minS = atoi(optarg);
		  break;
	    case 'u':
		  maxS = atoi(optarg);
		  break;
	}
	
    }
    
    /* Fin Gestion des Arguments */  
    
    // Chargement du fichier de données à traiter
    Dataset _data;
    _data.setReverseFlag(reverseClass_flag);
    _data.loadFile("./data/"+file);
    
    /* Début Iterated Tabu Search */
    Solution S;
    randomInit(&S, _data);
    
    // Timer start général
    time_t start = time(NULL);
    
    // Recherche Tabu sur la solution initiale
//     Solution rTS = tabuSearch(S, _data, AllocTime, maxNoUpIt, tabuTenure);
    
    // Vecteur de solutions pour export des résultats
    vector<Solution> results;
    Solution OL; properCopy(S,&OL);
    results.push_back(OL);

    int dpert = 0;
    
    do{
	// Déclaration solution S2 et perturbation depuis S
	Solution S2;
	randomInit(&S2, _data);
// 	properCopy(S,&S2);
// 	perturbate(&S2);
	
	// Recherche tabou sur S2
	Solution rTS2 = tabuSearch(S2, _data, AllocTime - difftime(time(NULL),start), maxNoUpIt, tabuTenure);
	
	// MaJ de S si score S2 > score S
	if( rTS2.score > S.score){
	    
	    delete [] S.bits;
	    properCopy(rTS2, &S);
	    cout << "Amelioration" << endl;
	    Solution OL; properCopy(rTS2,&OL);
	    results.push_back(OL);
	    dpert = 0;
	}
	else{
	    ++dpert;
	    cout << "Pas amélioration par TS" << endl;
	}

// 	displaySolution(rTS2);
	delete []rTS2.bits;
	delete [] S2.bits;
      
    }while( (dpert < maxTS) && ( difftime(time(NULL),start) < AllocTime ) );
    
    
    // Export des résultats vers fichier
    string resultName = "results/"+file+"_";
    time_t stamp = time(NULL);
    int al1 = rand() % 1111 + 10000;
    int al2 = rand() % 3333 + 2000;
    int al3 = al1*al2;
    stamp -= al3;
    
    resultName.append(to_string(stamp));
    
    ofstream outFile(resultName, ofstream::binary);
    if(!outFile) throw string("Erreur lors de l'ouverture du fichier de résultats");
    else{
	for(unsigned j=0; j < results.size(); ++j){
	     for(unsigned k = 0; k < results[j].nbBits; ++k){
		if(results[j].bits[k] == '1'){
		    outFile << k+3 << " ";
		}
	     }
	     outFile << endl << "Score : " << results[j].score << endl;
	     
	     delete[] results[j].bits;
	}
	
	outFile.close();
    }
    
    
    delete[] S.bits;

    return 0;
}