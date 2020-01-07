#include <SFML/Graphics.hpp>

#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <ctgmath>
#include <string>
#include <cstring>
#include <boost/filesystem.hpp>

#include "parametres.h"
#include "selectFile.h"

using namespace std;

Parametres::Parametres(){
  
}

Parametres::~Parametres(){
}

void Parametres::initParametres(string fconf){
  ifstream fichier(fconf, ios::in);       // ouverture du fichier parametres.conf
  string contenu;
  std::stringstream adr;
  string pf[2];
  int i=0;
  if(fichier){                                        // si l'ouverture a réussi
     while(getline(fichier, contenu)){
       char * cstr = new char [contenu.length()+1];
       std::strcpy (cstr, contenu.c_str());
       char * p = std::strtok (cstr,"=");
       i=0;
       while (p!=0){
 	     pf[i]=p;
 	     i++;
	     p = std::strtok(NULL," ");
       }
       if(pf[0]=="Thème"){
		  	  Theme=pf[1];
			  adr << std::fixed << Theme;
			  Theme=adr.str();
			  adr.clear();
			  adr.str("");
			  adr << std::fixed << dossierConfig <<"/Themes" << "/"<< Theme;
			  fTheme=adr.str();
			  adr.clear();
			  adr.str("");
       }
       if(pf[0]=="Lang"){
			  Lang=pf[1];
			  adr << std::fixed << dossierConfig <<"/Lang/divers."<<Lang;
			  diversLang=adr.str();
			  adr.clear();
			  adr.str("");
			  
       }
       if(pf[0]=="dossierConfig"){
		    dossierConfig=pf[1];
		    pf[1]="";
	     }
	     if(pf[0]=="Editeur"){
	     	 nEditeur.setString("");
		    Editeur=pf[1];
		    nEditeur.setString(Editeur);
		    pf[1]="";
	     }
	     if(pf[0]=="dirBanque"){
	     	 nBanques.setString("");
	     	 Banques=pf[1];
		  	 nBanques.setString(Banques);
	     }
	     if(pf[0]=="simpleLecteur"){
		    simpleLecteur=pf[1];
	     }
	     if(pf[0]=="midiPlayer"){
		    midiPlayer=pf[1];
	     }
     }		
  }else{  // sinon
       cerr << "Erreur à l'ouverture Préférences!" << endl;
  }
  
  std::cout << "Theme : " << Theme <<std::endl;
  std::cout << "Lang : " << Lang <<std::endl;
  std::cout << "Banques : " << Banques <<std::endl; 
  
}

string Parametres::drawParametres(int ftype, string fG,string fParametres, string titreWParam, sf::Font font,sf::Color fColor,int fSize, sf::Color ongSelColor, sf::Color ongColor, sf::Color fOngColor, int fOngSize){
  initParametres(fParametres);
  fGui=fG;
  stringstream adr;
  type=ftype;
  titreWinParam=titreWParam;   
  sf::Text ongConf1;
  ongConf1.setFont(font);
  ongConf1.setString(L"Paramètres");
  ongConf1.setCharacterSize(fOngSize);
  ongConf1.setPosition(sf::Vector2f(6, 4));
  ongConf1.setFillColor(fOngColor);
  

  sf::RectangleShape planConf(sf::Vector2f(500, 480)); // bloc de titre
  planConf.setFillColor(ongSelColor);
  planConf.setPosition(sf::Vector2f(0,24));
  sf::RectangleShape shOngConf1(sf::Vector2f(80, 24)); // bloc de titre
  shOngConf1.setFillColor(ongSelColor);
  shOngConf1.setPosition(sf::Vector2f(0,0));
  shOngConf1.setOutlineThickness(2.f);
  shOngConf1.setOutlineColor(sf::Color(0,0,0));
  
  circle.setRadius(4.f);
  circle1.setRadius(4.f);
  circle.setPosition(sf::Vector2f(116, 79));
  circle1.setPosition(sf::Vector2f(278, 79));
  circle2.setRadius(4.f);
  circle3.setRadius(4.f);
  circle2.setPosition(sf::Vector2f(117, 114));
  circle3.setPosition(sf::Vector2f(279, 114));

  sf::Texture ImgConf1;
  adr << std::fixed << fG<<"/conf1.png";
  ImgConf1.loadFromFile(adr.str());
  sf::Sprite panConf1(ImgConf1);
  panConf1.setPosition(sf::Vector2f(0, 24));
  adr.clear();
  adr.str("");
  

  sf::Texture ImgConf5;
  adr << std::fixed << fG<<"/bAnnuler.png";
  ImgConf5.loadFromFile(adr.str());
  sf::Sprite annuler(ImgConf5);
  annuler.setPosition(sf::Vector2f(360, 390));
  adr.clear();
  adr.str("");
  sf::Texture ImgConf6;
  adr << std::fixed << fG<<"/bValider.png";
  ImgConf6.loadFromFile(adr.str());
  sf::Sprite valider(ImgConf6);
  valider.setPosition(sf::Vector2f(430, 390));
  adr.clear();
  adr.str("");
  sf::Texture ImgConf7;
  adr << std::fixed << fG<<"/bParDefaut.png";
  ImgConf7.loadFromFile(adr.str());
  sf::Sprite parDefaut(ImgConf7);
  parDefaut.setPosition(sf::Vector2f(25, 390));
  adr.clear();
  adr.str("");
  
  winParam.create(sf::VideoMode(winParamWidth, winParamHeight), titreWinParam);
  sf::RectangleShape menuBar(sf::Vector2f( winParamWidth,ParamBarHeight));
  nText.setFont(font);
  nText.setCharacterSize(fSize);
  nText.setPosition(sf::Vector2f(134, 58));
  nText.setFillColor(fColor);
  nEditeur.setFont(font);
  nEditeur.setCharacterSize(fSize);
  nEditeur.setPosition(sf::Vector2f(128, 170));
  nEditeur.setFillColor(fColor);
  nBanques.setFont(font);
  nBanques.setCharacterSize(fSize);
  nBanques.setPosition(sf::Vector2f(128, 222));
  nBanques.setFillColor(fColor);

  
  while (winParam.isOpen()) {                             // Événements
        sf::Event event;
        while (winParam.pollEvent(event)){                // Gestion des évènements de la fenêtre principale
    	       	switch (event.type){
		        case sf::Event::Closed:            // Fermeture de la fenêtre
			        onClose();
			break;
			case sf::Event::TextEntered:
           			newText1(event);
			break;
            	        case sf::Event::MouseButtonPressed:
			  	onClick(event);
                	break;
            	        case sf::Event::MouseMoved:
			        onMouseMove(event);
                	break;
        				  					
		default:                            // on ne traite pas les autres types d'évènements
                	break;
  	       	}            
        
        }
        // Clear screen
	winParam.setActive(true);
	winParam.pushGLStates();
   winParam.clear(sf::Color(19,37,53));
   winParam.draw(ongConf1);
   winParam.draw(planConf);
  	shOngConf1.setFillColor(sf::Color(203,213,217));
	winParam.draw(shOngConf1);
	winParam.draw(ongConf1);
	winParam.draw(panConf1);
	
   winParam.draw(nEditeur);	
	winParam.draw(nBanques); 
	winParam.draw(parDefaut);
   winParam.draw(annuler);
   winParam.draw(valider);
   
   if(Lang=="Fr"){
   	circle.setFillColor(sf::Color(88, 142, 181));
   	circle1.setFillColor(sf::Color(255, 255, 255));
   }else{
   	circle.setFillColor(sf::Color(255, 255, 255));
   	circle1.setFillColor(sf::Color(88, 142, 181));
   }
   winParam.draw(circle);
   winParam.draw(circle1);
   if(Theme=="base"){
   	circle2.setFillColor(sf::Color(88, 142, 181));
   	circle3.setFillColor(sf::Color(255, 255, 255));
   }else{
   	circle2.setFillColor(sf::Color(255, 255, 255));
   	circle3.setFillColor(sf::Color(88, 142, 181));
   }
   winParam.draw(circle2);
   winParam.draw(circle3);
   
	winParam.display();
	winParam.popGLStates();
	winParam.setActive(false);
   }
   return txt;
}
void Parametres::onClose(){
  txt="";
  winParam.close();
}
void Parametres::valider(){
  saveParametres();
  winParam.close();
}
void Parametres::defaut(){
  std::cout << "Valeurs par défaut" << std::endl;
  winParam.close();
}

void Parametres::onMouseMove(sf::Event e){
   std::cout << "mouse x: " << e.mouseMove.x << std::endl;
   std::cout << "mouse y: " << e.mouseMove.y << std::endl;
  
}
void Parametres::onClick(sf::Event e){
   if (e.mouseButton.button == sf::Mouse::Left){
      std::cout << "the left button was pressed" << std::endl;
      std::cout << "mouse x: " << e.mouseButton.x << std::endl;
      std::cout << "mouse y: " << e.mouseButton.y << std::endl;
   }
  if(e.mouseButton.x>114 && e.mouseButton.x<127 && e.mouseButton.y>74 && e.mouseButton.y<96){
    Lang="Fr"; 
  }
  if(e.mouseButton.x>276 && e.mouseButton.x<289 && e.mouseButton.y>74 && e.mouseButton.y<96){
    Lang="En";
  }
  if(e.mouseButton.x>114 && e.mouseButton.x<127  && e.mouseButton.y>108 && e.mouseButton.y<130){
    Theme="base";
  }
  if(e.mouseButton.x>276 && e.mouseButton.x<289  && e.mouseButton.y>108 && e.mouseButton.y<130){
    Theme="Dark";
  }
  if(e.mouseButton.x>115 && e.mouseButton.x<271 && e.mouseButton.y>164 && e.mouseButton.y<200){
    Editeur="";
    std::cout << "editeur: " << std::endl;
    flagTxt=1;
  }
  if(e.mouseButton.x>114 && e.mouseButton.x<464 && e.mouseButton.y>214 && e.mouseButton.y<250){
  	 selectFile fileSelector;
    fileSelector.initSelector(0,getcwd(NULL,0),fGui,fTheme,Theme,diversLang,simpleLecteur);
	 string rtf2=fileSelector.selector();
	 Banques=getcwd(NULL,0);
	 nBanques.setString(Banques);
  }

   if( e.mouseButton.y>392 && e.mouseButton.y<422){
     if(e.mouseButton.x>26 && e.mouseButton.x<96){
       defaut();
     }
     if(e.mouseButton.x>360 && e.mouseButton.x<418){
       onClose();
     }
     if(e.mouseButton.x>432 && e.mouseButton.x<480){
       valider();
     }
   }
   
   
}

void Parametres::newText1(sf::Event e){
     int key=e.key.code;
     if (e.text.unicode < 128){
		switch (key){
	     case 8:
	         if(flagTxt==1){
	       		txt=txt.substr(0,txt.length()-1);
	       		nEditeur.setString(txt);
	       	}
	 	  break;
	     case 13:
	     		if(flagTxt==1){
	     			Editeur=txt;
	     			nEditeur.setString(txt);
	     		}
		  break;
	     default:
			txt=txt+static_cast<char>(e.text.unicode);
			if(flagTxt==1){
	     	 nEditeur.setString(txt);
	  		}
		break;
    	}
    }
}
void Parametres::saveParametres(){
	bool alerteFlag=0;
	string keyFileName;
   string name="parametres.conf";
	string nameFile=dossierConfig+"/"+name;
	ofstream fichier(nameFile, ios::out | ios::trunc);
	if(fichier){  // si l'ouverture a réussi
	  fichier<<"Thème="<<Theme<<endl;
	  fichier<<"Lang="<< Lang<<endl;
	  string s=nEditeur.getString();
	  fichier<<"Editeur="<<s<<endl;
	  fichier<<"dossierConfig="<<dossierConfig<<endl;
	  fichier<<"simpleLecteur="<<simpleLecteur<<endl;
	  fichier<<"midiPlayer="<<midiPlayer<<endl;
	  fichier<<"dirBanque="<<Banques<<endl;
	}else{  // sinon
     cerr << "Erreur à l'ouverture !" << endl;
	}
	fichier.close(); 
}