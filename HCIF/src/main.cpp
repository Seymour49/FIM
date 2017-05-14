#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <ctime>
#include <getopt.h>
#include <fstream>
#include "../include/dataset.h"

using namespace std;

struct Solution{
    unsigned nbBits;
    char* bits;
    int CM[4];
    float score; 
  
}; typedef struct Solution Solution;


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
 * Calcul du PhiCoefficient depuis la matrice de confusion 
 * représentée par les 4 nombres passés en paramètre
 * Equivalent au coefficient de corrélation de Matthews
 */
float phi_coeff(float tp, float fp, float tn, float fn){
    float p = tp + fp;
    float n = tn + fn;
    float pcomp = tp + fn;
    float ncomp = tn + fp;
    
    float num = tp*tn - fp*fn;
    float den = sqrt(p*n*pcomp*ncomp);
    
    return num/den;
}

/**
 * Youden's J Statistique. Calculée à partir de CM représentée
 * par les 4 nombres passés en paramètre 
 */
float j_stat(float tp, float fp, float tn, float fn){
    
    return ( (tp/(tp+fn)) + (tn/(tn+fp)) - 1);
}

/**
 * Comparaison de deux voisins selon leur score ( vector<float> s[0])
 */
static bool compareGain(const pair<int,vector<float>> a, const pair<int,vector<float>> b){
	return a.second[0] > b.second[0];
}


/**
 * Fonction d'évaluation du voisinage via scan complet de data pour chaque voisin
 * de s.
 * Retourne la paire <pos,score> correspondant à l'item à fliper et au score qui lui
 * est associé.
 */
pair<unsigned, vector<float>> naiveNeighboor(Solution s, Dataset& data, int eval_flag, unsigned minS, unsigned maxS){
    
    vector<pair<unsigned,vector<float>>> scores;
    
    // Copie de s dans tmp
    Solution tmp;
    tmp.nbBits = s.nbBits;
    tmp.bits = new char[tmp.nbBits];
    int nbK = 0;
    for(unsigned i=0; i < tmp.nbBits; ++i){
	tmp.bits[i] = s.bits[i];
	if( s.bits[i] == '1')
	    ++nbK;
    }
    
    // Pour chacun des bits on teste sa valeur et on évalue le voisin correspondant
    for(unsigned i=0; i < tmp.nbBits; ++i){
	if(tmp.bits[i] == '0'){
	  
	    tmp.bits[i] = '1';
	    // TP = 0, FP = 1, TN = 2, FN = 3
	    vector<int>cm = data.confusionMatrix(tmp.bits);
	    
	    // Score enregistre le score calculé en position 0 et la CM dans les positions suivantes
	    vector<float> score;
	    score.push_back(0.0);
	    for(unsigned v=0; v < 4; ++v) score.push_back((float)cm[v]);
	    
	    switch(eval_flag){
		case 0:
		    score[0] = f1_measure(score[1], score[2], score[4]);
		    break;
		case 1:
		    score[0] = perso_measure(nbK+1,score[1], score[4], minS, maxS );
		    break;
		case 2:
		    score[0] = phi_coeff(score[1],score[2],score[3],score[4]);
		    break;
		case 3:
		    score[0] = j_stat(score[1],score[2],score[3],score[4]);
		    break;
	    }	    
	    scores.push_back(make_pair(i,score));
	    tmp.bits[i] = '0';
	}
	else{
	    tmp.bits[i] = '0';
	    
	    if((nbK-1) > 0){
		vector<int>cm = data.confusionMatrix(tmp.bits);
		
		for(unsigned v=0; v < 4; ++v){
		    tmp.CM[v] = cm[v];
		}
		
		// Score enregistre le score calculé en position 0 et la CM dans les positions suivantes
		vector<float> score;
		score.push_back(0.0);
		for(unsigned v=0; v < 4; ++v) score.push_back((float)cm[v]);
		
		switch(eval_flag){
		    case 0:
		    score[0] = f1_measure(score[1], score[2], score[4]);
		    break;
		case 1:
		    score[0] = perso_measure(nbK+1,score[1], score[4], minS, maxS );
		    break;
		case 2:
		    score[0] = phi_coeff(score[1],score[2],score[3],score[4]);
		    break;
		case 3:
		    score[0] = j_stat(score[1],score[2],score[3],score[4]);
		    break;
		}
		
	    
		scores.push_back(make_pair(i,score));
	    }
	    else{
		// Score enregistre le score calculé en position 0 et la CM dans les positions suivantes
		vector<float> score;
		for(unsigned v=0; v < 5; ++v) score.push_back(0.0);
		
		scores.push_back(make_pair(i,score));
	    }
	    tmp.bits[i] = '1';
	}
    }
    
    sort(scores.begin(), scores.end(), compareGain);
    
    // Calcul des éléments de score maximal équivalent
    vector<int> part;
    
    part.push_back(0);
    bool goon = true;
    
    for(unsigned i=1; (i < scores.size()) && goon; ++i){
	if( scores[i].second[0] == scores[0].second[0] ){
	    part.push_back(i);
	}
	else{
	    goon = false;
	}
    }
    
    // Mélange des différents éléments
    random_shuffle(part.begin(), part.end());
    
    // Sélection aléatoire
    unsigned pos = rand() % part.size();
    
    delete [] tmp.bits;
    
    return scores[pos];
}

/**
 * Fonction d'initialisation de la solution passée en paramètre
 */
void randomInit(Solution *s, Dataset & data, int eval_flag, unsigned minS, unsigned maxS){  

    // Sélection d'une transaction de manière aléatoire 
    int randTrans = rand() % data.getnbRows();
    
    while( data.getBit(randTrans,0) == '0' ){
	randTrans = rand() % data.getnbRows();
    }
    
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
    
    int nbItem = rand() % (bits.size()/4) + (bits.size()/5);
    
    // Mélange des positions 
    random_shuffle(bits.begin(), bits.end());
    
    for(int i=0; i < nbItem ;++i){
	s->bits[bits[i]] = '0';
    }
    
    vector<int>CMS = data.confusionMatrix(s->bits);
    
    for(unsigned v=0; v < 4; ++v){
	s->CM[v] = CMS[v];
    }
    
    float score = 0.0;
    int nbK = getK(*s);
    switch(eval_flag){
	case 0:
	    score = f1_measure((float)CMS[0], (float)CMS[1], (float)CMS[3]);
	    break;
	case 1:
	    score = perso_measure(nbK,(float)CMS[0], (float)CMS[3], minS, maxS );
	    break;
	case 2:
	    score = phi_coeff((float)CMS[0],(float)CMS[1],(float)CMS[2],(float)CMS[3]);
	    break;
	case 3:
	    score = j_stat((float)CMS[0],(float)CMS[1],(float)CMS[2],(float)CMS[3]);
	    break;
    }
    
    s->score = score;
}


/**
 * Copie de s1 vers s2
 */ 
void properCopy(Solution &s1, Solution* s2){

    s2->nbBits = s1.nbBits;
    s2->bits = new char[s2->nbBits];
    for(unsigned i=0; i < s2->nbBits; ++i) s2->bits[i] = s1.bits[i];
    s2->score = s1.score;
    
    for(unsigned i=0; i < 4; ++i) s2->CM[i] = s1.CM[i];
}


Solution hillClimber(Solution & s0, Dataset &_data, long long int maxIt,long long int maxNoUp, int eval_flag, unsigned minS, unsigned maxS){
 
    // Création des variables et initialisation par copie
    Solution _sCurrent, _SB;
    properCopy(s0, &_sCurrent);
    properCopy(_sCurrent, &_SB);
    
    long long int d = 0;
    long long int currentIt = 0;
    
    
    do{
	// Sélection du meilleur voisin 
	pair<unsigned, vector<float>> bestN;
	bestN = naiveNeighboor(_sCurrent, _data,eval_flag, minS, maxS);
	
	// Changement de la solution courante et de son score
	if( _sCurrent.bits[bestN.first] == '0'){
	    _sCurrent.bits[bestN.first] = '1';
	}
	else{
	    _sCurrent.bits[bestN.first] = '0';
	}
	
	// Maj de CM et du score
	_sCurrent.score = bestN.second[0];
	for(unsigned k=0; k < 4; ++k) _sCurrent.CM[k] = (int)bestN.second[k+1];
	// Comparaison avec la meilleure solution
	if( _sCurrent.score > _SB.score ){
	    // Copie de la solution courante vers la meilleure et remise à 0 de noUpIt
	    for(unsigned m=0; m < _sCurrent.nbBits; ++m)
		_SB.bits[m] = _sCurrent.bits[m];
	    _SB.score = _sCurrent.score;
	    
	    for(unsigned k=0; k < 4; ++k) _SB.CM[k] = _sCurrent.CM[k];
	    
	    d = 0;
	}
	else{
	    ++d;
	}
	++currentIt;
      
      
    }while( (d < maxNoUp) && (currentIt < maxIt) );
    
    delete [] _sCurrent.bits;
    return _SB;
}


int main(int argc, char** argv){

    // Initialisation de l'aléatoire
    srand(time(NULL));
  
    /*
     * Gestion des arguments
     */
    string file = "SD_I20_T50_D0.72"; 		// JDD par défaut
    long long int maxIt = 10000;		// Max mouvement voisinage
    long long int maxNoUp = 1500;		// Max mouvement sans amélioration
    long long int minS = 5000;			// Valeur du seuil minimal
    long long int maxS = 250;			// Valeur du seuil maximal
    unsigned repeat = 5;			// Nombre de répétition
    
    // Flag pour fonction évaluation
    int evaluate_flag = 0;		// 0 = f1_measure, 1 = perso_measure
    int reverseClass_flag = 0;		// 0 = pas d'inversion, 1 = inversion
    
    while(1){
	int opt;
	
	// Déclarations des long_options
	static struct option long_options[] = {
	    
	    /* Flags, i.e pas de version courte */
	    {"f1_measure", no_argument, &evaluate_flag, 0},
	    {"perso_measure", no_argument, &evaluate_flag, 1},
	    {"phi_coeff", no_argument, &evaluate_flag, 2},
	    {"j_stat", no_argument, &evaluate_flag, 3},
	    
	    {"reverseC", no_argument, &reverseClass_flag, 1},
	    
	    
	    /* Options avec version courtes */
	    {"dataFile", required_argument, 0, 'd'},
	    {"maxIT", required_argument, 0, 't'},
	    {"maxNoUp", required_argument, 0, 's'},
	    {"minS", required_argument, 0, 'l'},
	    {"maxS", required_argument, 0, 'u'},
	    {"repeat", required_argument,0,'r'},
	    {0,0,0,0}
	};
	
	// getopt_long recupere l'option ici
	int option_index = 0;
	
	opt = getopt_long(argc,argv, "d:t:s:l:u:r:", long_options, &option_index);
	
	// fin des options
	if(opt == -1)
	    break;
	
	switch(opt){
	    
	    //Gestion des flags
	    case 0:
		if(long_options[option_index].flag != 0)
		  break;
	    case 't':
		  maxIt = stoll(optarg);
		  break;
	    case 'd':
		  file = string(optarg);
		  break;
	    case 's':
		  maxNoUp = stoll(optarg);
		  break;  
	    case 'l':
		  minS = stoll(optarg);
		  break;
	    case 'u':
		  maxS = stoll(optarg);
		  break;
	    case 'r':
		  repeat = atoi(optarg);
		  break;
	}
	
    }
    
    /* Fin Gestion des Arguments */  
    vector<Solution> results;
    
    // Chargement du fichier de données à traiter
    Dataset _data;
    _data.setReverseFlag(reverseClass_flag);
    
    _data.loadFileBinary("./data/"+file);
   
    for( unsigned k=0; k < repeat; ++k){
	
	// Début HillClimber
	Solution S;
	randomInit(&S, _data,evaluate_flag, minS, maxS);
	
	// Recherche Tabu sur la solution initiale
	Solution rHC = hillClimber(S, _data, maxIt, maxNoUp, evaluate_flag, minS, maxS);
	
	// Vecteur de solutions pour export des résultats
	Solution OL; properCopy(rHC,&OL);
	results.push_back(OL);

	delete[] S.bits;
	delete[] rHC.bits;
    }
    

    // Export des résultats vers fichier
    string resultName = "results/"+file+"_HC_";
    // Ajout de la méthode d'évaluation utilisée dans le nom du fichier de sortie
    switch(evaluate_flag){
	case 0:
	    resultName.append("f1_");
	    break;
	case 1:
	    resultName.append("perso_");
	    break;
	case 2:
	    resultName.append("phi_");
	    break;
	case 3:
	    resultName.append("j_");
	    break;      
    }
    
    time_t stamp = time(NULL);
    int al1 = rand() % 1111 + 1000;
    int al2 = rand() % 3333 + 200;
    int al3 = al1*al2;
    stamp -= al3;
    
    resultName.append(to_string(stamp));
    
    
    ofstream outFile(resultName, ofstream::binary);
    
    if(!outFile) throw string("Erreur lors de l'ouverture du fichier de résultats");
    else{
      
	vector<string> comment = _data.getComment();
	for(unsigned i=0; i < comment.size(); ++i){
	    outFile << comment[i] << endl;
	}
	outFile << endl;
	for(unsigned j=0; j < results.size(); ++j){
	     for(long long unsigned k = 0; k < results[j].nbBits; ++k){
		
		    outFile << results[j].bits[k] << " ";
		
	     }
	     outFile << endl << "Score : " << results[j].score << endl;
	     
	     for( long long unsigned k=0; k < results[j].nbBits; ++k){
		if( results[j].bits[k] == '1' )
		    outFile << k << " ";
	     }
	     outFile << endl;
	     outFile << "TP : " << results[j].CM[0] << " | FP : " << results[j].CM[1] << endl;
	     outFile << "TN : " << results[j].CM[2] << " | FN : " << results[j].CM[3] << endl;
	     delete[] results[j].bits;
	}
	
	outFile.close();
    }
    

    return 0;
}