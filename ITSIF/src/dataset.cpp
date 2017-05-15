#include "../include/dataset.h"

using namespace std;

Dataset::Dataset():_nbCols(0), _nbRows(0), _Matrice(NULL), reverseFlag(0)
{
}


Dataset::Dataset(unsigned int c, unsigned int r, int rFlag): _nbCols(c), _nbRows(r), reverseFlag(rFlag)
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


void Dataset::setReverseFlag(int reverseClass_flag)
{
    reverseFlag = reverseClass_flag;
}


// A utiliser pour mushroom
void Dataset::loadFileInteger(const string& filename)
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
	    if(reverseFlag == 0){
		if( mat[i][0] == 1){
		    _Matrice[i][0] = '1';
		}
		else{
		    _Matrice[i][0] = '0';
		}
	    }
	    else{
		if( mat[i][0] == 1){
		    _Matrice[i][0] = '0';
		}
		else{
		    _Matrice[i][0] = '1'; 
		}
	    }
	    
	    for(unsigned it=1; it < mat[i].size(); ++it){
		_Matrice[i][mat[i][it]-2] = '1';
	    }
	}
		
    }
    f.close();
}


void Dataset::loadFileBinary(const string& filename)
{

   if ((_nbRows != 0)||(_nbCols != 0)) {
	for (unsigned int i = 0; i < _nbRows; ++i) delete []_Matrice[i];
	delete [] _Matrice;
    }
    
    ifstream f(filename.c_str());
  
    if(!f) throw string("Erreur lors de l'ouverture du fichier " + filename + " ! (charDataSetO)");
    else {
      
      
// 	cout << "Lecture commentaire" << endl;
	string line;
	vector<string> tokens;
	
	vector<vector<int>> mat;
	vector<int> row;
	
	while( getline(f,line) ){
	
	    if(!line.empty()){
		  tokens.clear(); tokens.shrink_to_fit();
		  row.clear(); row.shrink_to_fit();
		  
		  tokens = explode2(line);
		  
		  // La ligne n'est plus un paramètre
		  if( tokens[0].compare("#") != 0 ){
		    
		      for( long long unsigned i=0; i < tokens.size(); ++i){
			  row.push_back(atoi(tokens[i].c_str()));
		      }
		    
		      mat.push_back(row);
		  }
		  else{
		      _comment.push_back(line);
		      
		  }
	      }
	  
	}
	
	// Matrice binaire totalement remplie
	_nbRows = mat.size();
	_nbCols = mat[0].size();
	
	_Matrice = new char*[_nbRows];
	
	for(long long unsigned t=0; t < _nbRows; ++t){
	    _Matrice[t] = new char[_nbCols];
	    
	    for(long long unsigned k=0; k < _nbCols; ++k){
		if( mat[t][k] == 1 ){
		    _Matrice[t][k] = '1';
		}
		else{
		    _Matrice[t][k] = '0';
		}
	    }
	}
	f.close();
    }    
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
      
	// Classe prédite = 1
	if( include(bitset, i) ){
	    // Classe réelle = 1
	    if(_Matrice[i][0] == '1'){
		++CM[0];	// TP
	    }
	    // Classe réelle = 0
	    else{
		++CM[1]; 	// FP
	    }
	}
	// Classe prédite = 0
	else{
	    // Classe réelle = 1
	    if(_Matrice[i][0] == '1' ){
		++CM[3];	// FN	      
	    }
	    // Classe réelle = 0
	    else{
		++CM[2];	// TN
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
		CL[1].push_back(i);
	    }
	}
	else{
	    if(_Matrice[i][0] == '1' ){
		CL[3].push_back(i);
	    }
	    else{
		CL[2].push_back(i);
	    } 
	}
    }
    
    return CL;
}


vector<long long int > Dataset::tidList(char* bitset, vector<long long int> tid)
{
    vector<long long int> tidR;
    
    for( long long unsigned t = 0; t < tid.size(); ++t){
	if( include(bitset, tid[t]) ){
	    tidR.push_back(tid[t]);
	}
    }
    return tidR;
}


void Dataset::encodeInteger(const string& filename)
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
	
	vector<vector<string>> mat;
	vector<string> row;
	
	vector<vector<pair<string,int>>> domaines;
	vector<pair<string,int>> d;
	int nbAttr = 1;
	while( getline(f,line) ){
	    
	    if(!line.empty()){
		  tokens.clear(); tokens.shrink_to_fit();
		  row.clear(); row.shrink_to_fit();
		  
		  tokens = explode2(line);
		  // Ligne paramètre
		  if(tokens[0] == "#"){
		      d.clear(); d.shrink_to_fit();
		      
		      for(unsigned i=1; i < tokens.size(); ++i){
			  d.push_back(make_pair(tokens[i], nbAttr));
			  ++nbAttr;
		      }
		      domaines.push_back(d);
		  }
		  else{
		      for( unsigned int i=0; i < tokens.size(); ++i){
			  row.push_back(tokens[i]);
		      }
		      
		      mat.push_back(row);      
		  }	  
	    }  
	}
	
	string fileName = "./data/mushroom.dat";
	
	ofstream outFile(fileName, ofstream::binary);
	
	if(!outFile) throw string("Erreur lors de l'ouverture du fichier");
	else{
	    // Début de la conversion, balancez ça dans un fichier
	    for(unsigned i=0; i < mat.size(); ++i){
		
		for(unsigned j=0; j < mat[i].size(); ++j){    
		    for(unsigned k=0; k < domaines[j].size(); ++k){
			  if( mat[i][j] == domaines[j][k].first ){
				outFile << domaines[j][k].second << " ";
			  }
		    }    
		}
		outFile << endl;
	    }
	    outFile.close();
	}
	f.close();
    }
}


long long unsigned int Dataset::getNBTCP()
{
    long long unsigned r = 0;
    
    for(long long unsigned t=0; t < _nbRows; ++t){
	if( _Matrice[t][0] == '1') ++r;      
    }
    
    return r;
}



void Dataset::clearDataset(char* bitset)
{
    vector<long long int> tid;
    for( long long unsigned t=0; t < _nbRows; ++t) tid.push_back(t);
    
    vector<long long int> tidTP = tidList(bitset,tid);
    
    vector<long long int>::iterator it = set_difference(tid.begin(),tid.end(),tidTP.begin(),tidTP.end(),tid.begin());
    
    tid.resize(it-tid.begin());
    
    char** newM = new char*[tid.size()];
    for(long long unsigned t=0; t < tid.size(); ++t){
	newM[t] = new char[_nbCols];
	
	for(long long unsigned k=0; k < _nbCols; ++k){
	    newM[t][k] = _Matrice[tid[t]][k];	  
	}
    }
    
    for(long long unsigned t=0; t < _nbRows; ++t){
	delete[] _Matrice[t];
    }
    
    delete[] _Matrice;
    
    cout << "Retrait de " << _nbRows - tid.size() << " transactions" << endl;
    
    _nbRows = tid.size();
    
    _Matrice = new char*[_nbRows];
    for( long long unsigned t=0; t < _nbRows; ++t){
	_Matrice[t] = new char[_nbCols];
	for(long long unsigned k=0; k < _nbCols; ++k){
	    _Matrice[t][k] = newM[t][k];
	}
	delete[] newM[t];
    }
    delete[] newM;
    
}


vector< string > explode2(const string& str)
{
  istringstream split(str);
  vector< string > tokens;

  for(string each; getline(split, each, ' '); tokens.push_back( each.c_str()) );

  return tokens;
}