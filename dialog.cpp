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

#include "dialog.h"

using namespace std;

Dialog::Dialog(){
  
}

Dialog::~Dialog(){
}

void Dialog::initDialog(){
    
  
}

string Dialog::drawDialog(int ftype, string fG, string titreWDialog, sf::Font font,sf::Color fColor,int fSize){
  stringstream adr;
  type=ftype;
  fGui=fG;
  titreWinDialog=titreWDialog;   
  sf::Texture selectF1;
  if(type==0){
    adr << std::fixed << fGui<<"/newSession.png";
    selectF1.loadFromFile(adr.str());
    adr.clear();
    adr.str("");
  }
  if(type==1){
    adr << std::fixed << fGui<<"/alertDossier.png";
    selectF1.loadFromFile(adr.str());
    adr.clear();
    adr.str("");
  }
  sf::Sprite alertFichier(selectF1);
  alertFichier.setPosition(sf::Vector2f(0, 0));
  
  winDialog.create(sf::VideoMode(winDialogWidth, winDialogHeight), titreWinDialog);
  sf::RectangleShape menuBar(sf::Vector2f( winDialogWidth,dialogBarHeight));
  nText.setFont(font);
  nText.setCharacterSize(fSize);
  nText.setPosition(sf::Vector2f(134, 58));
  nText.setFillColor(fColor);

  
  while (winDialog.isOpen()) {                             // Événements
        sf::Event event;
        while (winDialog.pollEvent(event)){                // Gestion des évènements de la fenêtre principale
    	       	switch (event.type){
		        case sf::Event::Closed:            // Fermeture de la fenêtre
			        onClose();
			break;
			case sf::Event::TextEntered:
           			newText(event);
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
	winDialog.setActive(true);
	winDialog.pushGLStates();
        //winDialog.clear(dialogBkg);
	winDialog.draw(alertFichier);
	winDialog.draw(nText);
	winDialog.display();
	winDialog.popGLStates();
	winDialog.setActive(false);
    }
    return txt;
}
void Dialog::onClose(){
  txt="";
  winDialog.close();
}

void Dialog::onMouseMove(sf::Event e){
   std::cout << "mouse x: " << e.mouseMove.x << std::endl;
   std::cout << "mouse y: " << e.mouseMove.y << std::endl;
  
}
void Dialog::onClick(sf::Event e){
   if (e.mouseButton.button == sf::Mouse::Left){
      std::cout << "the left button was pressed" << std::endl;
      std::cout << "mouse x: " << e.mouseButton.x << std::endl;
      std::cout << "mouse y: " << e.mouseButton.y << std::endl;
   }
   if(e.mouseButton.x>130 && e.mouseButton.x<330 && e.mouseButton.y>54 && e.mouseButton.y<80){
    
   }
   if(e.mouseButton.x>202 && e.mouseButton.x<258 && e.mouseButton.y>98 && e.mouseButton.y<126){
     txt="";
     winDialog.close();
   }
   if(e.mouseButton.x>266 && e.mouseButton.x<316 && e.mouseButton.y>98 && e.mouseButton.y<126){
     winDialog.close();
   }
   
}

void Dialog::newText(sf::Event e){
     int key=e.key.code;
     if (e.text.unicode < 128){
	switch (key){
	     case 8:
	       	txt=txt.substr(0,txt.length()-1);
	       	nText.setString(txt);
	 	break;
	     case 13:
	         nText.setString("");
		break;
	     default:
		txt=txt+static_cast<char>(e.text.unicode);
		nText.setString(txt);
		break;
       	}
     }
}
