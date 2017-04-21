#include "../include/dataset.h"

using namespace std;

Dataset::Dataset():_nbCols(0), _nbRows(0), _Matrice(NULL)
{
}


Dataset::Dataset(unsigned int c, unsigned int r): _nbCols(c), _nbRows(r)
{
    _Matrice = new char*[_nbRows];
    for(unsigned i=0; i < _nbRows; ++i){
	_Matrice[i] = new char[_nbCols];
	for(unsigned j=0; j < _nbCols; ++j){
	    _Matrice[i][j] = '0';
	}
      
    }
}

Dataset::~Dataset()
{
    if(_Matrice != NULL){
	for(unsigned int i=0; i < _nbRows; ++i){
	    delete [] _Matrice[i];	  
	}
	delete [] _Matrice;
    }
}


void Dataset::loadFile(const string& filename)
{
    if ((_nbRows != 0)||(_nbCols != 0)) {
	for (unsigned int i = 0; i < _nbRows; ++i) delete []_Matrice[i];
	delete [] _Matrice;
    }
    
    ifstream f(filename.c_str());
  
    if(!f) throw string("Erreur lors de l'ouverture du fichier " + filename + " ! (charDataSetO)");
    else {
	string line;
	vector<string> tokens;
	bool firstLineRead = false;
	int Max = 0; 
	while(!firstLineRead){
	    getline(f,line);
	    
	    if(!line.empty()){
		firstLineRead = true;
		tokens = explode2(line);
	    }	  
	}
	
	cout << "toto" << endl;
	vector<vector<int>> mat;
	vector<int> row;
	
	for(unsigned int i=0; i < tokens.size(); ++i){
	    row.push_back(atoi(tokens[i].c_str()));
	}
	Max = atoi(tokens[tokens.size()-1].c_str());
	mat.push_back(row);
	
	while( getline(f,line) ){
	    
	    if(!line.empty()){
		  tokens.clear(); tokens.shrink_to_fit();
		  row.clear(); row.shrink_to_fit();
		  
		  tokens = explode2(line);
		  for( unsigned int i=0; i < tokens.size(); ++i){
		      row.push_back(atoi(tokens[i].c_str()));
		  }
		  
		  if( row[row.size()-1] > Max){
		      Max = row[row.size()-1];
		  }  
		  
		  mat.push_back(row);
		  
	    }
	  
	}
	
	// Matrice d'entiers remplis, conversion binaire
	_nbRows = mat.size();
	_nbCols = Max - 1; // Car dans mushroom, la classe de l'échantillon est soit 1 soit 2
	
	_Matrice = new char*[_nbRows];
	for(unsigned i = 0; i < _nbRows; ++i){
	    _Matrice[i] = new char[_nbCols];
	    
	    for(unsigned int j=0; j< _nbCols; ++j){
		_Matrice[i][j] = '0';
	    }
	    
	    // Détermination de la classe de la transaction
	    if( mat[i][0] == 1){
		_Matrice[i][0] = '1';
	    }
	    else{
		_Matrice[i][0] = '0';
	      
	    }
	    
	    for(unsigned it=1; it < mat[i].size(); ++it){
		_Matrice[i][mat[i][it]-2] = '1';
	    }
	}
		
    }
    f.close();
}


bool Dataset::include(char* bitset, unsigned int i)
{
    bool isIn = true;
    for(unsigned j=1; (j < _nbCols) && (isIn); ++j){
	if( bitset[j-1] == '1' && _Matrice[i][j] == '0')
	    isIn = false;
    }
    return isIn;
}



vector< int > Dataset::confusionMatrix(char* bitset)
{
    // confusionMatrix avec TP, FP, TN, FN 
    vector<int> CM;
    
    for( int i=0; i < 4; ++i){
	CM.push_back(0);
    }
    
    for(unsigned i=0; i < _nbRows; ++i){
	if( include(bitset, i) ){
	    if(_Matrice[i][0] == '1'){
		CM[0] = CM[0] + 1;
	    }
	    else{
		CM[3] = CM[3] + 1;
	    }
	}
	else{
	    if(_Matrice[i][0] == '1' ){
		CM[1] = CM[1] + 1;	      
	    }
	    else{
		CM[2] = CM[2] + 1;
	    } 
	}
    }
    
    return CM;
}


vector< vector< int > > Dataset::confusionLists(char* bitset)
{
    vector<vector<int>> CL;
    for(unsigned i=0; i < 4; ++i){
	vector<int>L;
	CL.push_back(L);
    }
    
    for(unsigned i=0; i < _nbRows; ++i){
	if( include(bitset, i) ){
	    if(_Matrice[i][0] == '1'){
		CL[0].push_back(i);
	    }
	    else{
		CL[3].push_back(i);
	    }
	}
	else{
	    if(_Matrice[i][0] == '1' ){
		CL[1].push_back(i);
	    }
	    else{
		CL[2].push_back(i);
	    } 
	}
    }
    
    return CL;
}



vector< string > explode2(const string& str)
{
  istringstream split(str);
  vector< string > tokens;

  for(string each; getline(split, each, ' '); tokens.push_back( each.c_str()) );

  return tokens;
}


