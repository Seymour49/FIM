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
 * Méthode de calcul du voisinage n'effectuant qu'un scan de la base de données
 * et maintenant un compteur de 5 float par voisins. 
 * 3 fois moins rapide que la version naïve
 */
pair<unsigned, vector<float>> neighboor( Solution s, Dataset& data, vector<int> &TL, int iter, float best, int eval_flag, unsigned minS, unsigned maxS){
 
    vector<pair<unsigned, vector<float>>> scores;
    
    int nbk = 0;
    
    for(long long int i=0; i < s.nbBits; ++i){      
	vector<float> cm(5,0.0);
	scores.push_back(make_pair(i,cm));
	if( s.bits[i] == '1') ++nbk;
    }
    
    
    for(long long int t=0; t < data.getnbRows(); ++t){
      
	  // La solution n'est pas incluse
	  if( !data.include(s.bits, t) ){
	    
	      for(long long int i=0; i < s.nbBits; ++i){
		
		  // Ajout d'un item = pas de changement
		  if( s.bits[i] == '0' ){
		      
		      if( data.getBit(t,0) == '1' ){
			
			  ++scores[i].second[4];
		      }
		      else{
			  ++scores[i].second[3];
		      }
		  }
		  // Retrait de l'item
		  else{
		    char *tmp = new char[s.nbBits];
		    for(long long int x=0; x < s.nbBits; ++x){
			tmp[x] = s.bits[x];
		    }
		    tmp[i] = '0';
		    
		    if( data.include(tmp,t) ){
			if( data.getBit(t,0) == '1'){
			    ++scores[i].second[1];
			}
			else{
			    ++scores[i].second[2];
			}
		    }
		    else{
			if( data.getBit(t,0) == '1'){
			    ++scores[i].second[4];
			}
			else{
			    ++scores[i].second[3];
			}
		    }
		    delete[] tmp;
		  }
		
	      }
	    
	  }
	  // La solution est incluse dans t
	  else{
	    
	      for( long long int i=0; i < s.nbBits; ++i){
		  // Retirer un item ne changera pas le classement de t
		  if( s.bits[i] == '1' ){
		    
		      if( data.getBit(t,0) == '1' ){
			  ++scores[i].second[1];
		      }
		      else{
			  ++scores[i].second[2];
		      }		    
		  }
		  /*  Ajout d'un item. Si l'item appartient à t, ça ne change rien
		      Sinon : si t est positive, alors t appartient à FN
			      sinon, t appartient à TN
		  */
		  else{
		      
		      if( data.getBit(t,i+1) == '1' ){
			  if( data.getBit(t,0) == '1' ){
			      ++scores[i].second[1];
			  }
			  else{
			      ++scores[i].second[2];
			  }
		      }
		      else{
			  if( data.getBit(t,0) == '1'){
			      ++scores[i].second[4];
			  }
			  else{
			      ++scores[i].second[3];
			  }
		      }
		  }
	      }
	  }
    }
    
    // Calcul du score selon eval_flag
    for( long long unsigned i=0; i < scores.size(); ++i){
      
	switch(eval_flag){
	    case 0:
		scores[i].second[0] = f1_measure(scores[i].second[1],scores[i].second[2],scores[i].second[4]);
		break;
	    case 1:
		if( s.bits[i] == '1')
		    scores[i].second[0] = perso_measure(nbk-1,scores[i].second[1],scores[i].second[4], minS, maxS);
		else
		    scores[i].second[0] = perso_measure(nbk+1,scores[i].second[1],scores[i].second[4], minS, maxS);
		break;
	    case 2:
		scores[i].second[0] = phi_coeff(scores[i].second[1],scores[i].second[2],scores[i].second[3],scores[i].second[4]);
		break;
	    case 3:
		scores[i].second[0] = j_stat(scores[i].second[1],scores[i].second[2],scores[i].second[3],scores[i].second[4]);
		break;
	}
    }
    
    // Tri via compareGain
    sort(scores.begin(), scores.end(), compareGain);
    
    // Détection des mouvements de score équivalent
    // Mélange des différents clusters d'éléments de scores ayant la même valeur (i.e garde fou pour choix aléatoire des égalités)
    vector<int> part;
    
    part.push_back(0);
    for(unsigned i=1; i < scores.size(); ++i){
	if( scores[i].second[0] < scores[i-1].second[0] ){
	    part.push_back(i);
	}
    }
    
    part.push_back(scores.size()+1);
    for(unsigned i=1; i < part.size(); ++i){
	if( (part[i] - part[i-1]) > 1 ){
	    random_shuffle(scores.begin()+part[i-1], scores.begin()+part[i] - 1);
	}
    }
    
    // Sélection aléatoire avec aspiration tabou
    bool chosen = false;
    long long unsigned pos = 0;
    while ( !chosen && (pos < scores.size()) ){
      
	// Mouvement interdit par la liste tabou
	if( TL[scores[pos].first] > iter ){
	    
	    // Mécanisme d'aspiration
	    if( scores[pos].second[0] > best ){
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

    return scores[pos];
}

/**
 * Fonction d'évaluation du voisinage via scan complet de data pour chaque voisin
 * de s.
 * Retourne la paire <pos,score> correspondant à l'item à fliper et au score qui lui
 * est associé.
 */
pair<unsigned, vector<float>> naiveNeighboor(Solution s, Dataset& data, vector<int> &TL, int iter, float best, int eval_flag, unsigned minS, unsigned maxS){
    
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
    
    // Sélection du meilleur voisin non-tabou (avec mécanisme d'aspiration)
    sort(scores.begin(), scores.end(), compareGain);
    
    // Mélange des différents clusters d'éléments de scores ayant la même valeur (i.e garde fou pour choix aléatoire des égalités)
    vector<int> part;
    
    part.push_back(0);
    for(unsigned i=1; i < scores.size(); ++i){
	if( scores[i].second[0] < scores[i-1].second[0] ){
	    part.push_back(i);
	}
    }
    
    part.push_back(scores.size()+1);
    for(unsigned i=1; i < part.size(); ++i){
	if( (part[i] - part[i-1]) > 1 ){
	    random_shuffle(scores.begin()+part[i-1], scores.begin()+part[i] - 1);
	}
    }
    
    // Choix du meilleur voisin parmi le cluster 0
    bool chosen = false;
    unsigned pos = 0;
    while ( !chosen && (pos < scores.size()) ){
      
	// Mouvement interdit par la liste tabou
	if( TL[scores[pos].first] > iter ){
	    
	    // Mécanisme d'aspiration
	    if( scores[pos].second[0] > best ){
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
    
    delete [] tmp.bits;
    
    return scores[pos];
}

/**
 * Fonction d'initialisation de la solution passée en paramètre
 */
void randomInit(Solution *s, Dataset & data, int eval_flag, unsigned minS, unsigned maxS){  

    // Sélection d'une transaction de manière aléatoire 
    int randTrans = rand() % data.getnbRows();
    
    while( data.getBit(randTrans,0) == 0 ){
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


/**
 * Fonction de perturbation de la solution passée en paramètre
 */
void perturbate(Solution* S){
    displaySolution(*S);
}


Solution tabuSearch(Solution & s0, Dataset &_data, double remainingTime, unsigned maxIt, int tabuTenure, int eval_flag, unsigned minS, unsigned maxS){
 
    // Création des variables et initialisation par copie
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
	pair<unsigned, vector<float>> bestN;
	bestN = naiveNeighboor(_sCurrent, _data, TabuList, currentIt, _SB.score,eval_flag, minS, maxS);
	
	// Mise à jour TabuList
	TabuList[bestN.first] = currentIt+tabuTenure;
	
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
    string file = "SD_I20_T50_D0.72"; 		// JDD par défaut
    double AllocTime = 300;			// Temps en seconde
    unsigned maxNoUpIt = 10000;			// Max mouvement voisinage sans amélioration avant arrêt
    int tabuTenure = 15;			// TabuTenure
    int maxTS = 10;				// Maximum TS sans amélioration
    
    unsigned minS = 5000;			// Valeur du seuil minimal
    unsigned maxS = 250;			// Valeur du seuil maximal
    
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
    
//     _data.loadFileInteger("./data/mushroom.dat");
    _data.loadFileBinary("./data/"+file);
   
   
      // Début Iterated Tabu Search 
      Solution S;
      randomInit(&S, _data,evaluate_flag, minS, maxS);
      vector<int> TL(S.nbBits,0);
      // Test neighboor
      pair<unsigned, vector<float>> t;
      t = naiveNeighboor(S, _data,TL,0,0.0, 0, 0, 0);
    
/*    
    // Timer start général
    time_t start = time(NULL);
    
    // Recherche Tabu sur la solution initiale
    Solution rTS = tabuSearch(S, _data, AllocTime, maxNoUpIt, tabuTenure, evaluate_flag, minS, maxS);
    
    // Vecteur de solutions pour export des résultats
    vector<Solution> results;
    Solution OL; properCopy(rTS,&OL);
    results.push_back(OL);
*/

    // Code de la recherche tabou itérée.
/*
    int dpert = 0;
   
    do{
	// Déclaration solution S2 et perturbation depuis S
	Solution S2;

// 	properCopy(S,&S2);
// 	perturbate(&S2);
	randomInit(&S2, _data,evaluate_flag, minS, maxS);
	
	// Recherche tabou sur S2
	Solution rTS2 = tabuSearch(S2, _data, AllocTime - difftime(time(NULL),start), maxNoUpIt, tabuTenure, evaluate_flag, minS, maxS);
	
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

	delete []rTS2.bits;
	delete [] S2.bits;
      
    }while( (dpert < maxTS) && ( difftime(time(NULL),start) < AllocTime ) );
    
*/  

/*
    // Export des résultats vers fichier
    string resultName = "results/"+file+"_";
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
	     outFile << "TP : " << results[j].CM[0] << " | FP : " << results[j].CM[1] << endl;
	     outFile << "TN : " << results[j].CM[2] << " | FN : " << results[j].CM[3] << endl;
	     delete[] results[j].bits;
	}
	
	outFile.close();
    }
    
 */   
//     delete[] S.bits;

    return 0;
}