#include <SFML/Graphics.hpp>
#include <libgen.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <ctgmath>
#include <string>
#include <cstring>
#include <boost/filesystem.hpp>
#include <vector>

#include "faust/audio/jack-dsp.h"
#include "faust/dsp/llvm-dsp.h"
#include "faust/dsp/interpreter-dsp.h"
#include "faust/dsp/dsp-adapter.h"
#include "faust/dsp/proxy-dsp.h"
#include "faust/dsp/poly-dsp.h"
#include "faust/gui/meta.h"
#include "faust/gui/FUI.h"
#include "faust/gui/GTKUI.h"
#include "faust/gui/MidiUI.h"
#include "faust/gui/OSCUI.h"
#include "faust/gui/OSCControler.h"
#include "faust/gui/SoundUI.h"
#include "faust/misc.h"



#include "Application.h"
#include "selectFile.h"
#include "parametres.h"
#include "info.h"


using namespace std;
list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;

Application::Application(string dossier){
  std::stringstream adr;
  string s=getenv("HOME");
  string ds="/.mcMidiPlayer/parametres.conf";
  fichierParametres=s+ds;
  adr.clear();
  adr.str("");
  dossierConfig="/usr/local/share/mcMidiPlayer";
  adr << std::fixed <<dossierConfig<< "/gui";
  appGui=adr.str();
  adr.clear();
  adr.str("");
  std::cout << "parametres : " << fichierParametres<<std::endl;
  std::cout << "GUI" << appGui<<std::endl;
  newInstrument();
}

Application::~Application(){
}

void Application::appPreferences(){
  ifstream fichier(fichierParametres, ios::in);
  string contenu;
  std::stringstream adr;
  string pf[2];
  int i=0;
  if(fichier){ 
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
			  adr << std::fixed << dossierConfig <<"/Themes" << "/"<< Theme;
			  fichierTheme=adr.str();
			  adr.clear();
			  adr.str("");
       }
       if(pf[0]=="Lang"){
			  Lang=pf[1];
			  adr << std::fixed << dossierConfig <<"/Lang/" << Lang;
			  fichierLang=adr.str();
			  adr.clear();
			  adr.str("");
			  adr << std::fixed << dossierConfig <<"/Lang/divers." << Lang;
			  diversLang=adr.str();
			  adr.clear();
			  adr.str("");
       }
       if(pf[0]=="Editeur"){
	  		editeur=pf[1];
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
  std::cout << "Theme: " << fichierTheme<<std::endl;
  std::cout << "Lang : " << Lang<<std::endl;
  std::cout << "midiPlayer : " << midiPlayer<<std::endl;
  adr << std::fixed << appGui <<"/"<<Theme;
  appGui=adr.str();
  adr.clear();
  adr.str("");
  ifstream fichierL(diversLang, ios::in); 
    contenu="";
    sf::String sfstr;
    i=0;
    if(fichierL){  
      while(getline(fichierL, contenu)){
			char * cstr = new char [contenu.length()+1];
			std::strcpy (cstr, contenu.c_str());
			char * p = std::strtok (cstr,"=");
			i=0;
			while (p!=0){
	 	     pf[i]=p;
	 	     i++;
		     p = std::strtok(NULL,"=");
			}
			if(pf[0]=="labelNameBk"){
		  	//sfstr = sf::String::fromUtf8(pf[1].begin(), pf[1].end());
		   labelNameBk=pf[1];
			}
			
			if(pf[0]=="labelNbchannels"){
		  	sfstr = sf::String::fromUtf8(pf[1].begin(), pf[1].end());
		   labelNbchannels=sfstr;
			}
			if(pf[0]=="labelDefKey"){
		  	sfstr = sf::String::fromUtf8(pf[1].begin(), pf[1].end());
		   labelDefKey=sfstr;
			}
			if(pf[0]=="labelNfiles"){
		  	sfstr = sf::String::fromUtf8(pf[1].begin(), pf[1].end());
		   labelNfiles=sfstr;
			}
      }
    }else{  // sinon
		cerr << "Erreur à l'ouverture du fichier Lang!" << endl;
    }
}
void Application::appTheme(){
  ifstream fichier(fichierTheme, ios::in); 
  string contenu;
  string pf[2];
  int i=0;
  if(fichier){ 
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
       if(pf[0]=="windowAppWidth"){
	 windowAppWidth=stoi(pf[1]);
       }
        if(pf[0]=="windowAppHeight"){
	 windowAppHeight=stoi(pf[1]);
       }
       if(pf[0]=="AppBackGround"){
	 AppBackGround=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="AppBackGroundMenuBar"){
         AppBackGroundMenuBar=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="menuBarHeight"){
         menuBarHeight=stoi(pf[1]);
       }
       if(pf[0]=="menu1BkgColor"){
         menu1BkgColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="bkgWindowColor"){
         bkgWindowColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]==" keyFileColor"){
          keyFileColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="menu1bBkgColor"){
         menu1bBkgColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="itemShapBkgColor"){
         itemShapBkgColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="itemFontColor"){
         itemFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="itemShapFontColor"){
         itemShapFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="selectorBkgColor"){
         selectorBkgColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="selectorBarColor"){
         selectorBarColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="selectorFontColor"){
         selectorFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="dialogFontColor"){
         dialogFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="dialogFontSize"){
         dialogFontSize=stoi(pf[1]);
       }
       if(pf[0]=="paramOngSelectColor"){
         paramOngSelectColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="paramOngColor"){
         paramOngColor=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="paramFontColorOng"){
         paramFontColorOng=sf::Color(std::stoul(pf[1],nullptr,16));
       }
       if(pf[0]=="paramFontSizeOng"){
         paramFontSizeOng=stoi(pf[1]);
       } 
     }		
  }else{  // sinon
       cerr << "Erreur à l'ouverture Theme!" << endl;
  }

}
string Application::getLang() const {
  return Lang;
}
string Application::getGui() const {
  return appGui;
}
void Application::appInitMenu(){
  stringstream adr;
 
  adr << std::fixed << appGui<<"/"<<"Arial.ttf";
  font.loadFromFile(adr.str());
  adr.clear();
  adr.str("");
  menu1.setSize(sf::Vector2f(112,144));
  menu1.setFillColor(menu1BkgColor);
  menu1b.setSize(sf::Vector2f(110,142));
  menu1b.setFillColor(menu1bBkgColor);
  menu1.setPosition(sf::Vector2f(0, 34));
  menu1b.setPosition(sf::Vector2f(0, 34));
  
  menu2.setSize(sf::Vector2f(112,116));
  menu2.setFillColor(menu1BkgColor);
  menu2.setPosition(sf::Vector2f(56, 34));
  menu2b.setSize(sf::Vector2f(110,114));
  menu2b.setFillColor(menu1bBkgColor);
  menu2b.setPosition(sf::Vector2f(56, 34));
  
  itemShap.setSize(sf::Vector2f(108,28));
  itemShap.setFillColor(itemShapBkgColor);
 
  ifstream fichier(fichierLang, ios::in); 
  string contenu;
  sf::String sfstr;
  
  if(fichier){
    int k=42;
    getline(fichier,contenu);
    sfstr = sf::String::fromUtf8(contenu.begin(), contenu.end());
    menuItems[0][0].setFont(font);
    menuItems[0][0].setString(sfstr);
    menuItems[0][0].setCharacterSize(13);
    menuItems[0][0].setPosition(sf::Vector2f(6, 8));
    menuItems[0][0].setFillColor(itemFontColor);
    for(int i=1;i<6;i++){
       getline(fichier,contenu);

       sfstr = sf::String::fromUtf8(contenu.begin(), contenu.end());
       menuItems[0][i].setFont(font);
       menuItems[0][i].setString(sfstr);
       menuItems[0][i].setCharacterSize(13);
       menuItems[0][i].setPosition(sf::Vector2f(6, k));
       menuItems[0][i].setFillColor(itemFontColor);
       k=k+28;
    }
    k=42;
    getline(fichier,contenu);
    sfstr = sf::String::fromUtf8(contenu.begin(), contenu.end());
    menuItems[1][0].setFont(font);   
    menuItems[1][0].setString(sfstr);
    menuItems[1][0].setCharacterSize(13);
    menuItems[1][0].setPosition(sf::Vector2f(60, 8));
    menuItems[1][0].setFillColor(itemFontColor);
    for(int i=1;i<5;i++){
       getline(fichier,contenu);
       sfstr = sf::String::fromUtf8(contenu.begin(), contenu.end());
       menuItems[1][i].setFont(font);
       menuItems[1][i].setString(sfstr);
       menuItems[1][i].setCharacterSize(13);
       menuItems[1][i].setPosition(sf::Vector2f(60, k));
       menuItems[1][i].setFillColor(itemFontColor);
       k=k+28;
    }
    		
  }else{  // sinon
       cerr << "Erreur à l'ouverture du fichier Lang!" << endl;
  }
  
}
void Application::appDrawMenu(){
  window.draw(menuItems[0][0]);
  window.draw(menuItems[1][0]);
  if(selectM1){
    window.draw(menu1);
    window.draw(menu1b);
    for(int i=1;i<6;i++){
      window.draw(menuItems[0][i]);
    }
    for(int i=1;i<6;i++){
    menuItems[0][i].setFillColor(itemFontColor);
    }
    itemShap.setPosition(sf::Vector2f(0,36+(selectM1I*28)));
    window.draw(itemShap);
    menuItems[0][selectM1I+1].setFillColor(itemShapFontColor);
    window.draw(menuItems[0][selectM1I+1]);

  }
  if(selectM2){
    window.draw(menu2);
    window.draw(menu2b);
    for(int i=0;i<10;i++){
      window.draw(menuItems[1][i]);
    }
    for(int i=1;i<10;i++){
    menuItems[1][i].setFillColor(itemFontColor);
    }
    itemShap.setFillColor(sf::Color(103,113,117));
    itemShap.setPosition(sf::Vector2f(56,36+(selectM2I*28)));
    window.draw(itemShap);
    menuItems[1][selectM2I+1].setFillColor(itemShapFontColor);
    window.draw(menuItems[1][selectM2I+1]);
    
  }

  
}
void Application::menuSession(){
  newInstrument();
}
void Application::menuOpen(){
  readBank();
}
void Application::menuSave(){
  saveBank();
}

void Application::menuComment(){
  system(const_cast<char*>(editeur.c_str()));
}
void Application::menuDel(){
  deleteKey();
}
void Application::menuAllDel(){
  deleteInstrument();
}

void Application::menuPreferences(){
  string rt=newParametres.drawParametres(0, appGui, fichierParametres, "Nouvelle session",font,dialogFontColor,dialogFontSize,paramOngSelectColor,paramOngColor,paramFontColorOng,paramFontSizeOng);
}

void Application::appShapWindow(){
	stringstream adr;
	sf::RectangleShape workBordure;
	sf::RectangleShape workLBordure;
	
	sf::Texture selectb1;
   adr << std::fixed << appGui<<"/boucle.png";
   selectb1.loadFromFile(adr.str());
   sf::Sprite boutonBoucle(selectb1);
   adr.clear();
   adr.str("");
   sf::Texture selectb2;
   adr << std::fixed << appGui<<"/boucleb.png";
   selectb2.loadFromFile(adr.str());
   adr.clear();
   adr.str("");
   sf::Texture selectb3;
   adr << std::fixed << appGui<<"/fDroite.png";
   selectb3.loadFromFile(adr.str());
   sf::Sprite boutonGlobal(selectb3);
   adr.clear();
   adr.str("");
   sf::Texture selectb4;
   adr << std::fixed << appGui<<"/fDroiteb.png";
   selectb4.loadFromFile(adr.str());
   adr.clear();
   adr.str("");
	
   sf::Texture selectb5;
   adr << std::fixed << appGui<<"/haut.png";
   selectb5.loadFromFile(adr.str());
   sf::Sprite boutonLast(selectb5);
   boutonLast.setPosition(930,350);
   adr.clear();
   adr.str("");
   sf::Texture selectb6;
   adr << std::fixed << appGui<<"/bas.png";
   selectb6.loadFromFile(adr.str());
   sf::Sprite boutonNext(selectb6);
   boutonNext.setPosition(930,374);
   adr.clear();
   adr.str("");

   adr << std::fixed << appGui<<"/bActiver.png";
   selectb7.loadFromFile(adr.str());
   boutonActiver.setPosition(785,50);
   boutonActiver.setTexture(selectb7);
   adr.clear();
   adr.str("");
	
   bkgWindow.setSize(sf::Vector2f(940, 683));
   bkgWindow.setFillColor(bkgWindowColor);
   bkgWindow.setPosition(0,37);
   
   workBordure.setSize(sf::Vector2f(150, 2));
   workBordure.setFillColor(sf::Color(170, 170, 170));
   workLBordure.setSize(sf::Vector2f(2, 30));
   workLBordure.setFillColor(sf::Color(170, 170, 170));
   
   
   fileDef.setSize(sf::Vector2f(150, 30));
   fileDef.setFillColor(sf::Color(255, 255, 255));
   bankNameShape.setSize(sf::Vector2f(150, 30));
   bankNameShape.setPosition(130,50);
   bankCanauxShape.setSize(sf::Vector2f(40, 30));
   bankCanauxShape.setPosition(444,50);
   bankCanauxShape.setFillColor(sf::Color(255, 255, 255));
   
   sf::Text labelName;
   labelName.setString(labelNameBk);
   labelName.setFont(font);
   labelName.setCharacterSize(13);
   labelName.setPosition(10,56);
   labelName.setFillColor(sf::Color(0, 0, 0));
   sf::Text labelCanaux;
   labelCanaux.setString(labelNbchannels);
   labelCanaux.setFont(font);
   labelCanaux.setCharacterSize(13);
   labelCanaux.setPosition(290,56);
   labelCanaux.setFillColor(sf::Color(0, 0, 0));
   sf::Text labelFile;
   labelFile.setString(labelNfiles);
   labelFile.setFont(font);
   labelFile.setCharacterSize(13);
   labelFile.setPosition(80,90);
   labelFile.setFillColor(sf::Color(0, 0, 0));
   sf::Text labelKey;
   labelKey.setString(labelDefKey);
   labelKey.setFont(font);
   labelKey.setCharacterSize(13);
   labelKey.setPosition(4,90);
   labelKey.setFillColor(sf::Color(0, 0, 0));
   
   sf::Text key;
   key.setFont(font);
   key.setCharacterSize(14);
   key.setFillColor(keyFileColor);
   nbCanaux.setFont(font);
   nbCanaux.setCharacterSize(13);
   nbCanaux.setPosition(460,56);
   nbCanaux.setFillColor(sf::Color(0, 0, 0));
   bkgName.setFont(font);
   bkgName.setCharacterSize(13);
   bkgName.setPosition(148,56);
   bkgName.setFillColor(sf::Color(0, 0, 0));
   for(int i=0;i<128;i++){
   	fileName[i].setFont(font);
   	fileName[i].setCharacterSize(13);
   	fileName[i].setFillColor(sf::Color(0, 0, 0));
   }
   
   window.draw(bkgWindow);
   window.draw(labelName);
   window.draw(labelCanaux);
   window.draw(labelKey);
   window.draw(labelFile);
	window.draw(bankNameShape);
	window.draw(boutonActiver);
	
	
   workBordure.setSize(sf::Vector2f(150, 2));
   workBordure.setPosition(130,50);
   workLBordure.setPosition(130,50);
   window.draw(workBordure); 
   window.draw(workLBordure);   
 
   window.draw(bankCanauxShape);
   workBordure.setSize(sf::Vector2f(40, 2));
   workBordure.setPosition(444,50);
   workLBordure.setPosition(444,50);
   window.draw(workBordure); 
   window.draw(workLBordure);
   
   int k=0;
   for(int j=0;j<700;j=j+230){
	   for(int i=0;i<16;i++){
	   	key.setPosition(20+j,130+(35*i));
	   	key.setString(to_string(offsetWorkSpace+i+k));
	   	fileDef.setPosition(50+j,125+(35*i));
	   	window.draw(key); 
	   	window.draw(fileDef);
	   	workBordure.setSize(sf::Vector2f(150, 2));
	   	workBordure.setPosition(50+j,125+(35*i));
	   	workLBordure.setSize(sf::Vector2f(2, 30));
	   	workLBordure.setPosition(50+j,125+(35*i));
	   	window.draw(workBordure); 
	   	window.draw(workLBordure); 
	   	fileName[i+k+offsetWorkSpace].setPosition(60+j,132+(35*i));
	   	window.draw(fileName[i+k+offsetWorkSpace]); 
	   	
	   	if(keyBoucleFlag[i+k+offsetWorkSpace]==0){
	   		boutonBoucle.setTexture(selectb1);
	   	}else{
	   		boutonBoucle.setTexture(selectb2);
	   	}
	   	boutonBoucle.setPosition(204+j,124+(35*i));
	   	window.draw(boutonBoucle);
	   	if(keyGlobalFlag[i+k+offsetWorkSpace]==0){
	   		boutonGlobal.setTexture(selectb3);
	   	}else{
	   		boutonGlobal.setTexture(selectb4);
	   	}
	   	boutonGlobal.setPosition(204+j,142+(35*i));
	   	window.draw(boutonGlobal);
	   }
	   k=k+16;
   }
	
	winSliderDroit.setFillColor(sf::Color(209, 218, 222));
	winSliderDroit.setSize(sf::Vector2f(18.f, 713.f));
	winSliderDroit.setPosition(932,37);
	window.draw(winSliderDroit);
	window.draw(boutonLast);
	window.draw(boutonNext);
}

void Application::appWindow(){

  winWxScale = 1;
  winWyScale = 1;
  
  window.create(sf::VideoMode(windowAppWidth, windowAppHeight), "embedFaust");
  sf::RectangleShape menuBar(sf::Vector2f( windowAppWidth,menuBarHeight));
  menuBar.setFillColor(AppBackGroundMenuBar);
  
  while (window.isOpen()) {
        sf::Event event1;
        sf::Event event2;
        while (window.pollEvent(event1)){ 
    	      switch (event1.type){
		        case sf::Event::Closed:
			        onClose();
			     break;
              case sf::Event::Resized:
            	   onEventResized(event1);
                	break;
              case sf::Event::KeyPressed:
			         onKeyPressed(event1);
						break;
              case sf::Event::TextEntered:
           			newDefText(event1);
						break;
              case sf::Event::MouseButtonPressed:
			  			onClick(event1);
                	break;
              case sf::Event::MouseButtonReleased:
           			onMouseButtonUp(event1);
                	break;
              case sf::Event::MouseMoved:
			         onMouseMove(event1);
                	break;
        				  					
		        default: 
                	break;
  	       	}            
        
        }
        // Clear screen
	window.setActive(true);
	window.pushGLStates();
   window.clear(AppBackGround);
	window.draw(menuBar);

   Application::appShapWindow();
   
   window.draw(bkgName);
   window.draw(nbCanaux);

	appDrawMenu();
   
	
   window.display();
	window.popGLStates();
	window.setActive(false);
    }
}
void Application::onClose(){
  window.close();
}
void Application::onEventResized(sf::Event e){
  //winWxScale=e.size.width/winWidth;
  //  winWyScale=e.size.height/winHeight;
    std::cout << "winWxScale: " << winWxScale<< std::endl;
}
void Application::onKeyPressed(sf::Event e){
	
}

void Application::onMouseMove(sf::Event e){
   std::cout << "mouse x: " << e.mouseMove.x << std::endl;
   std::cout << "mouse y: " << e.mouseMove.y << std::endl;
   int calcy=0;
   if(e.mouseMove.x>0 && e.mouseMove.x<800 && e.mouseMove.y>0 && e.mouseMove.y<400){
     if(selectBarMenu==1 && e.mouseMove.x>6 && e.mouseMove.x<32){ // menu principal
	selectM1=1;
	selectM2=0;
     }
     if(selectM1==1 && (e.mouseMove.x>116 || e.mouseMove.y>200)){
	selectM1=0;
	selectBarMenu=0;
     }
     if(selectM1==1 && e.mouseMove.x>0 && e.mouseMove.x<116 && e.mouseMove.y>36 &&  e.mouseMove.y<200){
       selectM1I=(int)(e.mouseMove.y-36)/30;
     }
     if(selectBarMenu==1 && e.mouseMove.x>70 && e.mouseMove.x<140){ // menu principal
	selectM1=0;
	selectM2=1;
     }
     if(selectM2==1 && (e.mouseMove.x<56 || e.mouseMove.x>156  ||  e.mouseMove.y>150)){
	selectM2=0;
        selectBarMenu=0;
     }
     if(selectM2==1 && e.mouseMove.x>56 && e.mouseMove.x<156 && e.mouseMove.y>36 &&  e.mouseMove.y<150){
       selectM2I=(int)(e.mouseMove.y-36)/30;
     }
   }
   if(e.mouseMove.x>8 && e.mouseMove.x<940 && e.mouseMove.y>14 && e.mouseMove.y<950){
		//drag_mouseMove(e.mouseMove.x, e.mouseMove.y );
   }
}

void Application::onClick(sf::Event e){
	string rtf2;
  	string extension;
  	string extension2;
  	string s="/";
   if (e.mouseButton.button == sf::Mouse::Left){
      std::cout << "the left button was pressed" << std::endl;
      std::cout << "mouse x: " << e.mouseButton.x << std::endl;
      std::cout << "mouse y: " << e.mouseButton.y << std::endl;
   }
   if(e.mouseButton.x>0 && e.mouseButton.x<800 && e.mouseButton.y>0 && e.mouseButton.y<36){
     if(e.mouseButton.x>6 && e.mouseButton.x<32 && e.mouseButton.y>6 && e.mouseButton.y<30){ // menu principal
	selectM1=1;
	selectM2=0;
	selectBarMenu=1;
     }
     if(e.mouseButton.x>70 && e.mouseButton.x<140 && e.mouseButton.y>6 && e.mouseButton.y<30){
	selectM1=0;
	selectM2=1;
	selectBarMenu=1;
     }
   }
   if(selectM1==1 && e.mouseButton.x>6 && e.mouseButton.x<116 && e.mouseButton.y>36 && e.mouseButton.y<200){
     if(selectM1==1 && e.mouseButton.x>6 && e.mouseButton.x<116 && e.mouseButton.y>36 && e.mouseButton.y<64){
       menuSession();
     }
     if(selectM1==1 && e.mouseButton.x>6 && e.mouseButton.x<116 && e.mouseButton.y>64 && e.mouseButton.y<92){
       menuOpen();
     }
     if(selectM1==1 && e.mouseButton.x>6 && e.mouseButton.x<116 && e.mouseButton.y>92 && e.mouseButton.y<120){
       menuSave();
     }
     if(selectM1==1 && e.mouseButton.x>6 && e.mouseButton.x<116 && e.mouseButton.y>118 && e.mouseButton.y<150){
       menuComment();
     }
     if(selectM1==1 && e.mouseButton.x>6 && e.mouseButton.x<116 && e.mouseButton.y>150 && e.mouseButton.y<178){
       onClose();
     }
   }
   if(selectM2==1 && e.mouseButton.x>44 && e.mouseButton.x<160 && e.mouseButton.y>36 && e.mouseButton.y<274){
     if(e.mouseButton.x>44 && e.mouseButton.x<160 && e.mouseButton.y>36 && e.mouseButton.y<64){
       menuDel();
     }
     if(e.mouseButton.x>44 && e.mouseButton.x<160 && e.mouseButton.y>64 && e.mouseButton.y<92){
       menuAllDel();
     }
     if(e.mouseButton.x>44 && e.mouseButton.x<160 && e.mouseButton.y>123 && e.mouseButton.y<150){
       menuPreferences();
     }
   }
   if(e.mouseButton.x>830 && e.mouseButton.x<844 && e.mouseButton.y>winSliderMDroit.getPosition().y && e.mouseButton.y<winSliderMDroit.getPosition().y+winSliderMDroit.getSize().y){
   	flagSlider=1;
   	lastPosX=e.mouseButton.x;
   	lastPosY=e.mouseButton.y;
   }
   if(e.mouseButton.x>932 && e.mouseButton.x<950 && e.mouseButton.y>356 && e.mouseButton.y<370){
  		offsetWorkSpace=0;
   }
   if(e.mouseButton.x>932 && e.mouseButton.x<950 && e.mouseButton.y>382 &&e.mouseButton.y<394){
  		offsetWorkSpace=64;
   }
   if(e.mouseButton.x>130 && e.mouseButton.x<280 && e.mouseButton.y>52 && e.mouseButton.y<82){
       textDef=0;
       apptxt="";
   }
   if(e.mouseButton.x>448 && e.mouseButton.x<484 && e.mouseButton.y>52 && e.mouseButton.y<82){
       textDef=1;
       apptxt="";
   }
   if(e.mouseButton.x>672 && e.mouseButton.x<710 && e.mouseButton.y>52 && e.mouseButton.y<82){
       textDef=2;
       apptxt="";
   }
   
   if(selectM1==0 && selectM2==0 && e.mouseButton.x>52 && e.mouseButton.x<910 && e.mouseButton.y>124 ){
   	float hp=((e.mouseButton.x-52)/858.0)*4;
   	float vp=((e.mouseButton.y-127)/555.0)*16;
   	string s;
   	sf::String vs;
   	int id=(int)vp;
   	int idh=(int)hp*16;
	   if(e.mouseButton.x<((int)hp*230)+190){
	     string path=getcwd(NULL,0);
	     fileSelector.initSelector(0,path,appGui,fichierTheme,Theme,diversLang,simpleLecteur);
	  	  string rtf2=fileSelector.selector();
	  	  if(rtf2!=""){
	  	  	if(rtf2.length()>5){
		  	  string extension=rtf2.substr(rtf2.length()-4,4);
		  	  string extension2=rtf2.substr(rtf2.length()-5,5);
		  	  if(extension2==".flac" || extension2==".aiff" || extension==".wav" || extension==".ogg" ){
		  	  	 keyActive=id+idh+offsetWorkSpace;
		  	  	 fileNameF[keyActive]=getcwd(NULL,0)+s+rtf2;
		  	  	 s=rtf2.substr(0,20);
		  	  	 vs = sf::String::fromUtf8(s.begin(), s.end());
		  	  	 fileName[keyActive].setString(vs);
		  	  }
	  	   }
	  	  }
	  	}
	}
	
   if(e.mouseButton.x>202 && e.mouseButton.x<910){
   	float vp=((e.mouseButton.y-127)/555.0)*16;
   	float hp=((e.mouseButton.x-50)/858.0)*4;
   	int id=(int)vp;
   	int idh=(int)hp*16;
   	if(e.mouseButton.x>((int)hp*230)+190){
	   	if(e.mouseButton.y>(id*35)+127 && e.mouseButton.y<(id*35)+143){
	   		if(keyBoucleFlag[id+idh+offsetWorkSpace]==0){
	   		   keyBoucleFlag[id+idh+offsetWorkSpace]=1;
	   		   keyGlobalFlag[id+idh+offsetWorkSpace]=0;
	   		}else{
	   			keyBoucleFlag[id+idh+offsetWorkSpace]=0;
	   		}
	   	}
	   	if(e.mouseButton.y>(id*35)+144 && e.mouseButton.y<(id*35)+160){
	   		if(keyGlobalFlag[id+idh+offsetWorkSpace]==0){
	   		   keyGlobalFlag[id+idh+offsetWorkSpace]=1;
	   		   keyBoucleFlag[id+idh+offsetWorkSpace]=0;
	   		}else{
	   			keyGlobalFlag[id+idh+offsetWorkSpace]=0;
	   		}
	   	}
      }
   }
   
   if(e.mouseButton.x>762 && e.mouseButton.x<830 && e.mouseButton.y>50 && e.mouseButton.y<80){
     if(flagActive==0){
     	 genDSP();
     	 boutonActiver.setTexture(selectb8);
     }
   }
}
void Application::onMouseButtonUp(sf::Event e){
	
}
void Application::newDefText(sf::Event e){
   int key=e.key.code;
   if (e.text.unicode < 128){
	switch (key){
	   case 8:
	       	apptxt=apptxt.substr(0,apptxt.length()-1);
	       	switch (textDef){
	       		case 0:
	       			bkgName.setString(apptxt);
	       		break;
	       		case 1:
	       			nbCanaux.setString(apptxt);
	       		break;
	       		default:
	       		break;
	       	}
	 	break;
	   case 13:
	     		switch (textDef){
	       		case 0:
	       			bkgName.setString(apptxt);
	       		break;
	       		case 1:
	       			nbCanaux.setString(apptxt);
	       		break;
	       		default:
	       		break;
	       		apptxt="";
	       	}
		break;
	   default:
		apptxt=apptxt+static_cast<char>(e.text.unicode);
		switch (textDef){
	       		case 0:
	       			bkgName.setString(apptxt);
	       		break;
	       		case 1:
	       			nbCanaux.setString(apptxt);
	       		break;
	       		default:
	       		break;
	       	}
		break;
      }
   }
}

void Application::genDSP(){
	bool compileFlag=0;
	string nbcanaux;
	string listFile;
	string s1="'";
	string s2="';";
	string s3="_,";
	int keydef[128];
	if(bkgName.getString()==""){
		 bool rt=newInfo.drawInfo(2, appGui, "Alerte");
		compileFlag=1;
	}
	if(nbCanaux.getString()==""){
		bool rt=newInfo.drawInfo(3, appGui, "Alerte");
		compileFlag=1;
	}else{
		nbcanaux=nbCanaux.getString();
	}
	int countFile=0;
	for(int i=0;i<128;i++){
		if(fileNameF[i]!=""){
		  listFile=listFile+s1+fileNameF[i]+s2;
		  keydef[countFile]=i;
		  countFile++;
		}
   }
   listFile=listFile.substr(0,listFile.length()-1);
   if(countFile==0){
		bool rt=newInfo.drawInfo(4, appGui, "Alerte");
		compileFlag=1;
	}
	if(compileFlag==0){
		string exts="Player.dsp";
      string name2=bkgName.getString();
		string nameFile=name2+exts;
		string tabF[128];
		string destOutput;
		for(int i=0;i<stoi(nbcanaux);i++){
			destOutput=destOutput+s3;
		}
		destOutput=destOutput.substr(0,destOutput.length()-1);
		int ct=0;
		string path=getcwd(NULL,0);
		cout << "name file : " <<path<<"/"<< nameFile<< endl;
      ofstream fichier(nameFile, ios::out | ios::trunc);	
      if(fichier){  // si l'ouverture a réussi
        fichier<<"declare name 		\""<<name2<<"Player\";"<<endl;
        fichier<<"declare version 	\"1.0\";"<<endl;
        fichier<<"declare author 		\"D.Blanchemain\";"<<endl;
        fichier<<"declare license 	\"BSD\";"<<endl;
        fichier<<"declare copyright 	\"D.Blanchemain 2019\";\n"<<endl;
	     fichier<<"import(\"stdfaust.lib\");"<<endl;
	     fichier<<"import(\"soundfiles.lib\");\n"<<endl;
	     fichier<<"ds0=soundfile(\"[url:{"<<listFile<<"}]\","<<stoi(nbcanaux)<<");"<<endl;
	     fichier<<"sample(n)=so.sound(ds0,n);"<<endl;
	     
	     for(int i=0;i<96;i++){
	     	 if(fileNameF[i]!=""){
	     	  if(i<10){
	     	   fichier<<"tdec"<<i<<"(x) = vgroup(\"key0"<<i<<"\",x);"<<endl;
	     	  }else{
	     	  	fichier<<"tdec"<<i<<"(x) = vgroup(\"key"<<i<<"\",x);"<<endl;
	     	  }
	     	  if(keyGlobalFlag[i]==0){
		     	  fichier<<"gain"<<i<<" = vslider(\"gain[midi:key "<<i<<"]\",0.1,0,1,0.01) : si.smoo;"<<endl;
		     	  fichier<<"gate"<<i<<" = button(\"gate[midi:key "<<i<<"]\");"<<endl;
	     	  }else{
	     	  	  fichier<<"gain"<<i<<" = vslider(\"gain[midi:keyon "<<i<<"]\",0.1,0,1,0.01) : si.smoo;"<<endl;
		     	  fichier<<"gate"<<i<<" = button(\"gate[midi:keyon "<<i<<"]\");"<<endl;
	     	  }
	     	  fichier<<"lgain"<<i<<"=tdec"<<i<<"(gain"<<i<<");"<<endl;
	     	  fichier<<"lgate"<<i<<"=tdec"<<i<<"(gate"<<i<<");"<<endl;
	     	  if(keyBoucleFlag[i]==0){
		     	  if(i<10){
		     	  	fichier<<"smp0"<<ct<<" = sample("<<ct<<").play(lgain"<<i<<"*lgate"<<i<<", lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }else{
		     	  	fichier<<"smp"<<ct<<" = sample("<<ct<<").play(lgain"<<i<<"*lgate"<<i<<", lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }
	     	  }else{
	     	  	  if(i<10){
		     	  	fichier<<"smp0"<<ct<<" = sample("<<ct<<").loop_speed_level(1,lgain"<<i<<"*lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }else{
		     	  	fichier<<"smp"<<ct<<" = sample("<<ct<<").loop_speed_level(1,lgain"<<i<<"*lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }
	     	  }
	     	  ct++;
	     	 }
	     }
	     for(int i=96;i<100;i++){
	     	 if(fileNameF[i]!=""){
	     	  
	     	  	fichier<<"tdec"<<i<<"(x) = vgroup(\"key0"<<i<<"\",x);"<<endl;
	     	 
	     	  if(keyGlobalFlag[i]==0){
		     	  fichier<<"gain"<<i<<" = vslider(\"gain[midi:key "<<i<<"]\",0.1,0,1,0.01) : si.smoo;"<<endl;
		     	  fichier<<"gate"<<i<<" = button(\"gate[midi:key "<<i<<"]\");"<<endl;
	     	  }else{
	     	  	  fichier<<"gain"<<i<<" = vslider(\"gain[midi:keyon "<<i<<"]\",0.1,0,1,0.01) : si.smoo;"<<endl;
		     	  fichier<<"gate"<<i<<" = button(\"gate[midi:keyon "<<i<<"]\");"<<endl;
	     	  }
	     	  fichier<<"lgain"<<i<<"=tdec"<<i<<"(gain"<<i<<");"<<endl;
	     	  fichier<<"lgate"<<i<<"=tdec"<<i<<"(gate"<<i<<");"<<endl;
	     	  if(keyBoucleFlag[i]==0){
		     	  	fichier<<"smp0"<<ct<<" = sample("<<ct<<").play(lgain"<<i<<"*lgate"<<i<<", lgate"<<i<<")<:"<<destOutput<<";"<<endl;
	     	  }else{
		     	  	fichier<<"smp0"<<ct<<" = sample("<<ct<<").loop_speed_level(1,lgain"<<i<<"*lgate"<<i<<")<:"<<destOutput<<";"<<endl;
	     	  }
	     	  ct++;
	     	 }
	     }
	     for(int i=100;i<128;i++){
	     	 if(fileNameF[i]!=""){
	     	  if(i<10){
	     	   fichier<<"tdec"<<i<<"(x) = vgroup(\"key0"<<i<<"\",x);"<<endl;
	     	  }else{
	     	  	fichier<<"tdec"<<i<<"(x) = vgroup(\"key"<<i<<"\",x);"<<endl;
	     	  }
	     	  if(keyGlobalFlag[i]==0){
		     	  fichier<<"gain"<<i<<" = vslider(\"gain[midi:key "<<i<<"]\",0.1,0,1,0.01) : si.smoo;"<<endl;
		     	  fichier<<"gate"<<i<<" = button(\"gate[midi:key "<<i<<"]\");"<<endl;
	     	  }else{
	     	  	  fichier<<"gain"<<i<<" = vslider(\"gain[midi:keyon "<<i<<"]\",0.1,0,1,0.01) : si.smoo;"<<endl;
		     	  fichier<<"gate"<<i<<" = button(\"gate[midi:keyon "<<i<<"]\");"<<endl;
	     	  }
	     	  fichier<<"lgain"<<i<<"=tdec"<<i<<"(gain"<<i<<");"<<endl;
	     	  fichier<<"lgate"<<i<<"=tdec"<<i<<"(gate"<<i<<");"<<endl;
	     	  if(keyBoucleFlag[i]==0){
		     	  if(i<10){
		     	  	fichier<<"smp0"<<ct<<" = sample("<<ct<<").play(lgain"<<i<<"*lgate"<<i<<", lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }else{
		     	  	fichier<<"smp"<<ct<<" = sample("<<ct<<").play(lgain"<<i<<"*lgate"<<i<<", lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }
	     	  }else{
	     	  	  if(i<10){
		     	  	fichier<<"smp0"<<ct<<" = sample("<<ct<<").loop_speed_level(1,lgain"<<i<<"*lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }else{
		     	  	fichier<<"smp"<<ct<<" = sample("<<ct<<").loop_speed_level(1,lgain"<<i<<"*lgate"<<i<<")<:"<<destOutput<<";"<<endl;
		     	  }
	     	  }
	     	  ct++;
	     	 }
	     }
	     string smplayer[countFile];
	     string snd0;
	     string snd1;
	     if(countFile<17){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(i<10){
	     	     		fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     		fichier<<"lg0"<<i<<"=g1(lsmp0"<<i<<");"<<endl;
	     	     		snd0=snd0+"lg0"+to_string(i)+",";
	     	     	}else{
	        		fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        		fichier<<"lg"<<i<<"=g1(lsmp"<<i<<");"<<endl;
	        		snd0=snd0+"lg"+to_string(i)+",";
	        		}
	        		ct++;
	        	  }
	     		}
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     }
	     if(countFile>16 && countFile<33){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"lect1(x)=hgroup(\"l2\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(ct<16){
	     	     		if(i<10){
	     	     			fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     			fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	     	     			snd0=snd0+"lg0"+to_string(i)+",";
	     	     		}else{
	        			fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        			ct++;
	        	  	}else{
	        	  		fichier<<"lsmp"<<i<<"=lect1(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			ct++;
	        	  	}
	        	  }
	     		}
	     		
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     }
	     if(countFile>32 && countFile<49){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"lect1(x)=hgroup(\"l2\",x);"<<endl;
	     	   fichier<<"lect2(x)=hgroup(\"l3\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(ct<16){
	     	     		if(i<10){
	     	     			fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     			fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	     	     			snd0=snd0+"lg0"+to_string(i)+",";
	     	     		}else{
	        			fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        	  	}
	        	  	if(ct>=16 && ct<32){	
	        	  		fichier<<"lsmp"<<i<<"=lect1(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  
	        	  	if(ct>=32 && ct<49){	
	        	  		fichier<<"lsmp"<<i<<"=lect2(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  }
	        	  ct++;
	     		}
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     }
	     if(countFile>48 && countFile<65){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"lect1(x)=hgroup(\"l2\",x);"<<endl;
	     	   fichier<<"lect2(x)=hgroup(\"l3\",x);"<<endl;
	     	   fichier<<"lect3(x)=hgroup(\"l4\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(ct<16){
	     	     		if(i<10){
	     	     			fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     			fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	     	     			snd0=snd0+"lg0"+to_string(i)+",";
	     	     		}else{
	        			fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        	  	}
	        	  	if(ct>=16 && ct<32){	
	        	  		fichier<<"lsmp"<<i<<"=lect1(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  
	        	  	if(ct>=32 && ct<48){	
	        	  		fichier<<"lsmp"<<i<<"=lect2(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=48 && ct<65){	
	        	  		fichier<<"lsmp"<<i<<"=lect3(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  }
	        	  ct++;
	     		}
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     }
	     if(countFile>64 && countFile<81){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"lect1(x)=hgroup(\"l2\",x);"<<endl;
	     	   fichier<<"lect2(x)=hgroup(\"l3\",x);"<<endl;
	     	   fichier<<"lect3(x)=hgroup(\"l4\",x);"<<endl;
	     	   fichier<<"lect4(x)=hgroup(\"l5\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(ct<16){
	     	     		if(i<10){
	     	     			fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     			fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	     	     			snd0=snd0+"lg0"+to_string(i)+",";
	     	     		}else{
	        			fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        	  	}
	        	  	if(ct>=16 && ct<32){	
	        	  		fichier<<"lsmp"<<i<<"=lect1(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  
	        	  	if(ct>=32 && ct<48){	
	        	  		fichier<<"lsmp"<<i<<"=lect2(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=48 && ct<64){	
	        	  		fichier<<"lsmp"<<i<<"=lect3(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=64 && ct<81){	
	        	  		fichier<<"lsmp"<<i<<"=lect4(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  }
	        	  ct++;
	     		}
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     		}
	        	  if(countFile>80 && countFile<97){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"lect1(x)=hgroup(\"l2\",x);"<<endl;
	     	   fichier<<"lect2(x)=hgroup(\"l3\",x);"<<endl;
	     	   fichier<<"lect3(x)=hgroup(\"l4\",x);"<<endl;
	     	   fichier<<"lect4(x)=hgroup(\"l5\",x);"<<endl;
	     	   fichier<<"lect5(x)=hgroup(\"l6\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(ct<16){
	     	     		if(i<10){
	     	     			fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     			fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	     	     			snd0=snd0+"lg0"+to_string(i)+",";
	     	     		}else{
	        			fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        	  	}
	        	  	if(ct>=16 && ct<32){	
	        	  		fichier<<"lsmp"<<i<<"=lect1(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  
	        	  	if(ct>=32 && ct<48){	
	        	  		fichier<<"lsmp"<<i<<"=lect2(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=48 && ct<64){	
	        	  		fichier<<"lsmp"<<i<<"=lect3(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=64 && ct<80){	
	        	  		fichier<<"lsmp"<<i<<"=lect4(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  	if(ct>=80 && ct<96){	
	        	  		fichier<<"lsmp"<<i<<"=lect5(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  }
	        	  ct++;
	     		}
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     }
	     if(countFile>96 && countFile<112){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"lect1(x)=hgroup(\"l2\",x);"<<endl;
	     	   fichier<<"lect2(x)=hgroup(\"l3\",x);"<<endl;
	     	   fichier<<"lect3(x)=hgroup(\"l4\",x);"<<endl;
	     	   fichier<<"lect4(x)=hgroup(\"l5\",x);"<<endl;
	     	   fichier<<"lect5(x)=hgroup(\"l6\",x);"<<endl;
	     	   fichier<<"lect6(x)=hgroup(\"l7\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(ct<16){
	     	     		if(i<10){
	     	     			fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     			fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	     	     			snd0=snd0+"lg0"+to_string(i)+",";
	     	     		}else{
	        			fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        	  	}
	        	  	if(ct>=16 && ct<32){	
	        	  		fichier<<"lsmp"<<i<<"=lect1(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  
	        	  	if(ct>=32 && ct<48){	
	        	  		fichier<<"lsmp"<<i<<"=lect2(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=48 && ct<64){	
	        	  		fichier<<"lsmp"<<i<<"=lect3(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=64 && ct<80){	
	        	  		fichier<<"lsmp"<<i<<"=lect4(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  	if(ct>=80 && ct<96){	
	        	  		fichier<<"lsmp"<<i<<"=lect5(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  	if(ct>=96 && ct<112){	
	        	  		fichier<<"lsmp"<<i<<"=lect6(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  }
	        	  ct++;
	     		}
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     }
	     if(countFile>112 && countFile<129){
	     	   fichier<<"lect0(x)=hgroup(\"l1\",x);"<<endl;
	     	   fichier<<"lect1(x)=hgroup(\"l2\",x);"<<endl;
	     	   fichier<<"lect2(x)=hgroup(\"l3\",x);"<<endl;
	     	   fichier<<"lect3(x)=hgroup(\"l4\",x);"<<endl;
	     	   fichier<<"lect4(x)=hgroup(\"l5\",x);"<<endl;
	     	   fichier<<"lect5(x)=hgroup(\"l6\",x);"<<endl;
	     	   fichier<<"lect6(x)=hgroup(\"l7\",x);"<<endl;
	     	   fichier<<"lect7(x)=hgroup(\"l8\",x);"<<endl;
	     	   fichier<<"g1(x) = tgroup(\"0-15\",x);"<<endl;
	     	   ct=0;
	     	   for(int i=0;i<128;i++){
	     	     if(fileNameF[i]!=""){
	     	     	if(ct<16){
	     	     		if(i<10){
	     	     			fichier<<"lsmp0"<<i<<"=lect0(smp0"<<ct<<");"<<endl;
	     	     			fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	     	     			snd0=snd0+"lg0"+to_string(i)+",";
	     	     		}else{
	        			fichier<<"lsmp"<<i<<"=lect0(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        	  	}
	        	  	if(ct>=16 && ct<32){	
	        	  		fichier<<"lsmp"<<i<<"=lect1(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  
	        	  	if(ct>=32 && ct<48){	
	        	  		fichier<<"lsmp"<<i<<"=lect2(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=48 && ct<64){	
	        	  		fichier<<"lsmp"<<i<<"=lect3(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        			
	        	  	}
	        	  	if(ct>=64 && ct<80){	
	        	  		fichier<<"lsmp"<<i<<"=lect4(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  	if(ct>=80 && ct<96){	
	        	  		fichier<<"lsmp"<<i<<"=lect5(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  	if(ct>=96 && ct<112){
	        	  		if(ct<100){	
	        	  			fichier<<"lsmp0"<<i<<"=lect6(smp0"<<ct<<");"<<endl;
	        				fichier<<"lg0"<<i<<"=g1(lsmp0"<<ct<<");"<<endl;
	        				snd0=snd0+"lg0"+to_string(i)+",";
	        			}else{
	        				fichier<<"lsmp"<<i<<"=lect6(smp"<<ct<<");"<<endl;
	        				fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        				snd0=snd0+"lg"+to_string(i)+",";
	        			}
	        	  	}
	        	  	if(ct>=112 && ct<128){	
	        	  		fichier<<"lsmp"<<i<<"=lect7(smp"<<ct<<");"<<endl;
	        			fichier<<"lg"<<i<<"=g1(lsmp"<<ct<<");"<<endl;
	        			snd0=snd0+"lg"+to_string(i)+",";
	        	  	}
	        	  }
	        	  ct++;
	     		}
	     		snd0=snd0.substr(0,snd0.length()-1);
	     		fichier<<"process = "<<snd0<<":>"<<destOutput<<";"<<endl;
	     }
    fichier.close();  // on referme le fichier
   }else{  // sinon
     cerr << "Erreur à l'ouverture dsp!" << endl;
	}
	char commande[255];
	char* ns = new char [nameFile.length()+1];
	std::strcpy (ns, nameFile.c_str());
	char* pl = new char [midiPlayer.length()+1];
	std::strcpy (pl, midiPlayer.c_str());
	snprintf(commande, 255, "%s %s %s/%s", pl," -midi ",getcwd(NULL,0), ns);
	std::cout << "player : " << commande << std::endl;
	system(commande);
  }
}
void Application::saveBank(){
	bool alerteFlag=0;
	string exts=".bnk";
	string keyFileName;
   string name=bkgName.getString();
	string nameFile=name+exts;
	string nbcanaux=nbCanaux.getString();
	if(bkgName.getString()==""){
		 bool rt=newInfo.drawInfo(2, appGui, "Alerte");
		 alerteFlag=1;
	}
	if(nbCanaux.getString()==""){
		bool rt=newInfo.drawInfo(3, appGui, "Alerte");
		alerteFlag=1;
	}else{
		nbcanaux=nbCanaux.getString();
	}
	if(alerteFlag==0){
		string path=getcwd(NULL,0);
		fileSelector.initSelector(0,path,appGui,fichierTheme,Theme,diversLang,simpleLecteur);
		string rtf2=fileSelector.selector();
		path=getcwd(NULL,0);
		nameFile=path+"/"+name+exts;
		cout << "rtf2 : " << rtf2 <<" path" <<getcwd(NULL,0)<<endl;
		ofstream fichier(nameFile, ios::out | ios::trunc);
		if(fichier){  // si l'ouverture a réussi
		  fichier<<"name="<< name<<endl;
		  fichier<<"nbcanaux="<< nbcanaux<<endl;
		  for(int i=0;i<128;i++){
		  	if(fileName[i].getString()==""){
		  	  fichier<<i<<"="<<endl;
		  	}else{
		  	  fichier<<i<<"="<<fileNameF[i]<<endl;
		  	}
		  }
		  for(int i=0;i<128;i++){
		  	  fichier<<"b"<<i<<"="<<keyBoucleFlag[i]<<endl;
		  }
		  for(int i=0;i<128;i++){
		  	  fichier<<"d"<<i<<"="<<keyGlobalFlag[i]<<endl;
		  }
		}else{  // sinon
	     cerr << "Erreur à l'ouverture !" << endl;
		}
	}
}
void Application::readBank(){
	newInstrument();
	string path=getcwd(NULL,0);
	fileSelector.initSelector(0,path,appGui,fichierTheme,Theme,diversLang,simpleLecteur);
	string name=fileSelector.selector();
	path=getcwd(NULL,0);
	char * cstr = new char [name.length()+1];
	string nameFile=path+"/"+name;
	ifstream fichier(nameFile, ios::in);  // on ouvre
   string contenu;
	string pf[2];
	string s;
	string ns;
	int i=0;
	int k=0;
	int l=0;
	int m=0;
	if(fichier){  // si l'ouverture a réussi   
		while(getline(fichier, contenu)){
	     	cstr = new char [contenu.length()+1];
	     	std::strcpy (cstr, contenu.c_str());
	     	char * p = std::strtok (cstr,"=");
			i=0;
			while (p!=0){
 				pf[i]=p;
 				i++;
				p = std::strtok(NULL,"=");
  			}
  			
  			if(pf[0]=="name"){
	  			bkgName.setString(pf[1]);
	  			pf[1]="";
  			}
  			if(pf[0]=="nbcanaux"){
	  			nbCanaux.setString(pf[1]);
	  			pf[1]="";
  			}
  			if(pf[0]==to_string(k)){
	  			boost::filesystem::path pathObj(pf[1]);
	  			fileName[k].setString(pathObj.filename().string());
	  			fileNameF[k]=pf[1];
	  			pf[1]="";
	  			k++;
		  	}
		  	if(pf[0]=="b"+to_string(l)){
		  		keyBoucleFlag[l]=stoi(pf[1]);
		  		l++;
		  	}
		  	if(pf[0]=="d"+to_string(m)){
		  		keyGlobalFlag[m]=stoi(pf[1]);
		  		m++;
		  	}
		}
 		
	}else{  // sinon
     cerr << "Erreur à l'ouverture Préférences!" << endl;
	} 			
}
void Application::deleteInstrument(){
	for(int i=0;i<128;i++){
		fileName[i].setString("");
		fileNameF[i]="";
		keyBoucleFlag[i]=0;
      keyGlobalFlag[i]=0;
	}
}
void Application::deleteKey(){
	fileName[keyActive].setString("");
	fileNameF[keyActive]="";
	keyBoucleFlag[keyActive]=0;
   keyGlobalFlag[keyActive]=0;
}
void Application::newInstrument(){
	for(int i=0;i<128;i++){
		fileName[i].setString("");
		fileNameF[i]="";
		keyBoucleFlag[i]=0;
      keyGlobalFlag[i]=0;
	}
	bkgName.setString("");
   nbCanaux.setString("");
}
