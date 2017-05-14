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