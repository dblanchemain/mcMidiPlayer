#include <SFML/Graphics.hpp>
#include <sstream>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <stdlib.h>
#include <ctgmath>
#include <string>
#include <cstring>
#include <boost/filesystem.hpp>

#include "info.h"

using namespace std;

Info::Info(){
  
}

Info::~Info(){
}

void Info::initInfo(){
    
  
}

bool Info::drawInfo(int ftype, string fG, string titreWInfo){
  stringstream adr;
  type=ftype;
  adr << std::fixed <<fG;
  fGui=adr.str();
  adr.clear();
  adr.str("");
  titreWinInfo=titreWInfo;
  rtf=0;   
  sf::Texture selectF1;
  if(type==0){
    adr << std::fixed << fGui<<"/alertFichier.png";
    selectF1.loadFromFile(adr.str());
  }
  if(type==1){
    adr << std::fixed << fGui<<"/alertDossier.png";
    selectF1.loadFromFile(adr.str());
  }
  if(type==2){
    adr << std::fixed << fGui<<"/nomInstrument.png";
    selectF1.loadFromFile(adr.str());
  }
  if(type==3){
    adr << std::fixed << fGui<<"/nbCanaux.png";
    selectF1.loadFromFile(adr.str());
  }
  if(type==4){
    adr << std::fixed << fGui<<"/nbFichiers.png";
    selectF1.loadFromFile(adr.str());
  }
  sf::Sprite alertFichier(selectF1);
  alertFichier.setPosition(sf::Vector2f(0, 0));
  adr.clear();
  adr.str("");
  
  winInfo.create(sf::VideoMode(winInfoWidth, winInfoHeight), titreWinInfo);
  sf::RectangleShape menuBar(sf::Vector2f( winInfoWidth,infoBarHeight));
  

  
  while (winInfo.isOpen()) {                             // Événements
        sf::Event event;
        while (winInfo.pollEvent(event)){                // Gestion des évènements de la fenêtre principale
    	       	switch (event.type){
		        case sf::Event::Closed:            // Fermeture de la fenêtre
			        onClose();
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
	winInfo.setActive(true);
	winInfo.pushGLStates();
        //winDialog.clear(dialogBkg);
	winInfo.draw(alertFichier);
       
	winInfo.display();
	winInfo.popGLStates();
	winInfo.setActive(false);
    }
    return rtf;
}
void Info::onClose(){
  rtf=0;
  winInfo.close();
}

void Info::onMouseMove(sf::Event e){
   std::cout << "mouse x: " << e.mouseMove.x << std::endl;
   std::cout << "mouse y: " << e.mouseMove.y << std::endl;
  
}
void Info::onClick(sf::Event e){
   if (e.mouseButton.button == sf::Mouse::Left){
      std::cout << "the left button was pressed" << std::endl;
      std::cout << "mouse x: " << e.mouseButton.x << std::endl;
      std::cout << "mouse y: " << e.mouseButton.y << std::endl;
   }
   if(e.mouseButton.x>202 && e.mouseButton.x<258 && e.mouseButton.y>98 && e.mouseButton.y<126){
     rtf=0;
     winInfo.close();
   }
    if(e.mouseButton.x>266 && e.mouseButton.x<316 && e.mouseButton.y>98 && e.mouseButton.y<126){
     rtf=1;
     winInfo.close();
   }
   
}

