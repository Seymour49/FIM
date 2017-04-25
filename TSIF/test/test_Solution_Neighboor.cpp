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
    vector<int> CM = _data.confusionMatrix(s1.bits);
    s1.score = f1_measure((float)CM[0], (float)CM[1], (float)CM[3]);
 
//     tidListNeighboor(s1,_data);
//     naiveNeighboor(s1, _data);
    
    cout << "Classe + = " << CM[0]+CM[1] << "; Classe - = " << CM[2]+CM[3] << endl;
    delete[] s1.bits;

    return 0;
}