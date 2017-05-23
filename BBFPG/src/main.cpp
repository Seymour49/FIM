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
 * Fonction de gain utilisée dans l'algorithme FOIL
 * G = TP* (log( TP/(TP+FP) ) - log( (TP+FN)/(TP+FP+TN+FN) ) )
 */
float foil(float tp, float fp, float tn, float fn){
 
    return tp*(log(tp/(tp+fp))-log((tp+fn)/(tp+fp+tn+fn)));
}

/**
 * Comparaison de deux voisins selon leur score ( vector<float> s[0])
 */
static bool compareGain(const Solution a, const Solution b){
	return a.score > b.score;
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


vector< string > explode4(const string& str)
{
  istringstream split(str);
  vector< string > tokens;

  for(string each; getline(split, each, ' '); tokens.push_back( each.c_str()) );

  return tokens;
}


int main(int argc, char** argv){

    // Initialisation de l'aléatoire
    srand(time(NULL));
  
    /*
     * Gestion des arguments
     */
    string file = "SD_I20_T50_D0.55"; 		// JDD par défaut
    string fpfile = "SD_I20_T50_D0.55CP_for_FP_result_c20"; 	// Résultat fpgrowth
    
    
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
	    {"foil", no_argument, &evaluate_flag, 4},
	    
	    {"reverseC", no_argument, &reverseClass_flag, 1},
	    
	    /* Options avec version courtes */
	    {"dataFile", required_argument, 0, 'd'},
	    {"fpFile", required_argument, 0, 'f'},
	    {0,0,0,0}
	};
	
	// getopt_long recupere l'option ici
	int option_index = 0;
	
	opt = getopt_long(argc,argv, "d:f:", long_options, &option_index);
	
	// fin des options
	if(opt == -1)
	    break;
	
	switch(opt){
	    
	    //Gestion des flags
	    case 0:
		if(long_options[option_index].flag != 0)
		  break;
	    case 'd':
		  file = string(optarg);
		  break;
	    case 'f':
		  fpfile = string(optarg);
		  break;
	}
	
    }
    
    /* Fin Gestion des Arguments */  
   
    // Chargement du fichier de données à traiter
    Dataset _data;
    _data.setReverseFlag(reverseClass_flag);
    
    _data.loadFileBinary("./BBFPG/data/"+file);
   
    // Charger  le fichier d'itemset obtenus par fpgrowth
    string fppath = "./BBFPG/fpfile/"+fpfile;
    
    ifstream f(fppath.c_str());
    vector<Solution> VS;
    
    if(!f)  throw string("Erreur lors de l'ouverture du fichier " + fppath);
    else{
      
	string line;
	vector<string> tokens;
      
	
	
	while( getline(f,line) ){
	    
	    if(!line.empty() ){
	     
		tokens.clear(); tokens.shrink_to_fit();
		tokens = explode4(line);
		
		Solution s;
		s.nbBits = _data.getnbCols() -1;
		
		s.bits = new char[s.nbBits];
		for(unsigned i=0; i < s.nbBits; ++i) s.bits[i] = '0';
		
		for(unsigned i=0; i < tokens.size()-1; ++i){
		    s.bits[atoi(tokens[i].c_str())] = '1';
		}
		
		vector<int> cm = _data.confusionMatrix(s.bits);
		
		for(unsigned k=0; k < cm.size(); ++k) s.CM[k] = cm[k];
		
		switch(evaluate_flag){
		    case 0:
			s.score = f1_measure((float)s.CM[0], (float)s.CM[1], (float)s.CM[3]);
			break;
		    case 1:
			s.score = perso_measure(tokens.size()-1,(float)s.CM[0], (float)s.CM[3], 10, 2 );
			break;
		    case 2:
			s.score = phi_coeff((float)s.CM[0],(float)s.CM[1],(float)s.CM[2],(float)s.CM[3]);
			break;
		    case 3:
			s.score = j_stat((float)s.CM[0],(float)s.CM[1],(float)s.CM[2],(float)s.CM[3]);
			break;
		    case 4:
			s.score = foil((float)s.CM[0],(float)s.CM[1],(float)s.CM[2],(float)s.CM[3]);
		}
		
		
		VS.push_back(s);
		
	    }
	}
	f.close();
    }
   
   
    // Tri des résultats
    sort(VS.begin(), VS.end(), compareGain);
    

    // Export des résultats vers fichier
    string resultName = "./BBFPG/results/"+fpfile+"_BB_";
    // Ajout de la méthode d'évaluation utilisée dans le nom du fichier de sortie
    switch(evaluate_flag){
	case 0:
	    resultName.append("f1");
	    break;
	case 1:
	    resultName.append("perso");
	    break;
	case 2:
	    resultName.append("phi");
	    break;
	case 3:
	    resultName.append("jstat");
	    break;  
	case 4:
	    resultName.append("foil");
	    break;
    }

    
    ofstream outFile(resultName, ofstream::binary);
    
    if(!outFile) throw string("Erreur lors de l'ouverture du fichier de résultats");
    else{
      
	vector<string> comment = _data.getComment();
	for(unsigned i=0; i < comment.size(); ++i){
	    outFile << comment[i] << endl;
	}
	outFile << endl;

	for( long long unsigned j=0; j < VS.size(); ++j){
	  
	      for(unsigned i=0; i < VS[j].nbBits; ++i){
		      if( VS[j].bits[i] == '1' )
			outFile << i << " ";
		}
		outFile << endl << "Score : " << VS[j].score << " | TP " << VS[j].CM[0] << " | FP " << VS[j].CM[1] << " | TN " << VS[j].CM[2] << " | FN " << VS[j].CM[3] << endl;
	  
	      delete[] VS[j].bits;
	}
	outFile.close();
    }
    

    return 0;
}