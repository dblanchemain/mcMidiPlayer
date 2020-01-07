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

#include "selectFile.h"
#include "info.h"

using namespace std;

selectFile::selectFile(){
  
}

selectFile::~selectFile(){
}

void selectFile::initSelector(int ftype, string fdir,string fG, string fT, string Th, string fL, string simpleLecteur){
  std::stringstream adr;
  type=ftype;
  dossier=fdir;
  dossierPath.setString(dossier);
  listDirs(dossier);
  readFiles();
  if(config==0){
    fGui=fG;
    fTheme=fT;
    fLang=fL;
    //simplePlayer=simpleLecteur;
    strcpy(simplePlayer,simpleLecteur.c_str());
    std::cout << "Theme : "<<fTheme<< std::endl;
    std::cout << "Lang : "<<fLang<< std::endl;
     ifstream fichierT(fTheme, ios::in); 
     string contenu;
     string pf[2];
     int i=0;
     if(fichierT){
       while(getline(fichierT, contenu)){
			 char * cstr = new char [contenu.length()+1];
			 std::strcpy (cstr, contenu.c_str());
			 char * p = std::strtok (cstr,"=");
			 i=0;
			 while (p!=0){
		 	     pf[i]=p;
		 	     i++;
			     p = std::strtok(NULL," ");
			 }
			 if(pf[0]=="selectorBarColor"){
			   selectorBarColor=sf::Color(std::stoul(pf[1],nullptr,16));
			 }
			 if(pf[0]=="selectorBkgColor"){
			   selectorBkgColor=sf::Color(std::stoul(pf[1],nullptr,16));
			 }
			 if(pf[0]=="selectorFontColor"){
			   selectorFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
			 }
			 if(pf[0]=="selectFontSize"){
			   selectorFontSize=stoi(pf[1]);
			 }
			 if(pf[0]=="selectorFontColor"){
			   selectorFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
			 }
			 if(pf[0]=="selectFontSize"){
			   selectFontSize=stoi(pf[1]);
			 }
			 if(pf[0]=="dirFontColor"){
			   dirFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
			 }
			 if(pf[0]=="fileFontColor"){
			   fileFontColor=sf::Color(std::stoul(pf[1],nullptr,16));
			 }
       }		
    }else{  // sinon
       cerr << "Erreur à l'ouverture Préférences!" << endl;
    }
   
    ifstream fichierL(fLang, ios::in); 
    contenu="";
    sf::String sfstr;
    i=0;
    if(fichierL){  
      std::stringstream adr;
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
		if(pf[0]=="titreSelector"){
	  	sfstr = sf::String::fromUtf8(pf[1].begin(), pf[1].end());
	  	titreSelector=sfstr;
		}
      }
      adr << std::fixed << fGui<<"/Arial.ttf";
      font.loadFromFile(adr.str());
      adr.clear();
      adr.str("");
      config=1;
      winSelector.setTitle(titreSelector);
    }else{  // sinon
	cerr << "Erreur à l'ouverture du fichier Lang!" << endl;
    }
  }
  nText.setString("");
}

string selectFile::selector(){
  stringstream adr;
  sf::RectangleShape selectBarH(sf::Vector2f(winSelectorWidth, 36));
  selectBarH.setFillColor(selectorBarColor);
  selectBarH.setPosition(sf::Vector2f(0,0));
  sf::RectangleShape selectDefPath(sf::Vector2f(430, 24)); // Path
  selectDefPath.setFillColor(sf::Color(255,255,255));
  selectDefPath.setPosition(sf::Vector2f(46,6));
  sf::RectangleShape selectBarB(sf::Vector2f(winSelectorWidth, 36)); 
  selectBarB.setFillColor(sf::Color(selectorBarColor));
  selectBarB.setPosition(sf::Vector2f(0,400));
  sf::RectangleShape selectFileS(sf::Vector2f(300, 24));   // Fichier
  selectFileS.setFillColor(sf::Color(255,255,255));
  selectFileS.setPosition(sf::Vector2f(50, 406));
  
  selectShap.setSize(sf::Vector2f(172, 18));
  selectShap.setFillColor(selectorBarColor);
  selectShap.setPosition(sf::Vector2f(2, 40));
  
  selectBkgSlider.setSize(sf::Vector2f(20, 364));
  selectBkgSlider.setFillColor(selectorBarColor);
  selectBkgSlider.setPosition(sf::Vector2f(182,36));
  sf::RectangleShape lineBloc(sf::Vector2f(2,364));
  lineBloc.setFillColor(sf::Color(100,100,100));
  lineBloc.setPosition(sf::Vector2f(180,36));
  sf::RectangleShape lineBloch(sf::Vector2f(22,2));
  lineBloch.setFillColor(sf::Color(100,100,100));
  lineBloch.setPosition(sf::Vector2f(180,36));
  
  selectBkgSlider2.setSize(sf::Vector2f(18, 364));
  selectBkgSlider2.setFillColor(selectorBarColor);
  selectBkgSlider2.setPosition(sf::Vector2f(482,36));
  sf::RectangleShape lineBloc2(sf::Vector2f(2,364));
  lineBloc2.setFillColor(sf::Color(100,100,100));
  lineBloc2.setPosition(sf::Vector2f(480,36));
  sf::RectangleShape lineBloch2(sf::Vector2f(22,2));
  lineBloch2.setFillColor(sf::Color(100,100,100));
  lineBloch2.setPosition(sf::Vector2f(480,36));
  
  selectMSlider.setFillColor(sf::Color(200,200,200));
  selectMSlider.setPosition(sf::Vector2f(184,38));
  selectMSlider.setOutlineThickness(1.f);
  selectMSlider.setOutlineColor(sf::Color(0, 0, 0));
  selectMSlider2.setFillColor(sf::Color(200,200,200));
  selectMSlider2.setPosition(sf::Vector2f(484,38));
  selectMSlider2.setOutlineThickness(1.f);
  selectMSlider2.setOutlineColor(sf::Color(0, 0, 0));

  sf::Texture selectF1;
  adr << std::fixed << fGui<<"/trashF.png";
  selectF1.loadFromFile(adr.str());
  sf::Sprite selectTrashF(selectF1);
  selectTrashF.setPosition(sf::Vector2f(4, 6));
  adr.clear();
  adr.str("");
  sf::Texture selectF2;
  adr << std::fixed << fGui<<"/folder-new.png";
  selectF2.loadFromFile(adr.str());
  sf::Sprite selectNew(selectF2);
  selectNew.setPosition(sf::Vector2f(10, 406));
  adr.clear();
  adr.str("");
  sf::Texture selectF3;
  adr << std::fixed << fGui<<"/go-down.png";
  selectF3.loadFromFile(adr.str());
  sf::Sprite selectDown(selectF3);
  selectDown.setPosition(sf::Vector2f(42, 404));
  adr.clear();
  adr.str("");
  sf::Texture selectF4;
  adr << std::fixed << fGui<<"/go-up.png";
  selectF4.loadFromFile(adr.str());
  sf::Sprite selectUp(selectF4);
  selectUp.setPosition(sf::Vector2f(72, 404));
  adr.clear();
  adr.str("");
  sf::Sprite selectFileDown(selectF3);
  selectFileDown.setPosition(sf::Vector2f(226, 404));     
  sf::Sprite selectFileUp(selectF4);
  selectFileUp.setPosition(sf::Vector2f(252, 404));
  sf::Texture selectF5;
  adr << std::fixed << fGui<<"/bAnnuler.png";
  selectF5.loadFromFile(adr.str());
  sf::Sprite selectAnnuler(selectF5);
  selectAnnuler.setPosition(sf::Vector2f(360, 402));
  adr.clear();
  adr.str("");
  sf::Texture selectF6;
  adr << std::fixed << fGui<<"/bValider.png";
  selectF6.loadFromFile(adr.str());
  sf::Sprite selectValider(selectF6);
  selectValider.setPosition(sf::Vector2f(420, 402));
  adr.clear();
  adr.str("");
  sf::Texture selectF7;
  adr << std::fixed << fGui<<"/audio-speakers.png";
  selectF7.loadFromFile(adr.str());
  speaker.setTexture(selectF7);
  adr.clear();
  adr.str("");

  

  dossierPath.setFont(font);
  dossierPath.setCharacterSize(selectorFontSize);
  dossierPath.setPosition(sf::Vector2f(60, 8));
  dossierPath.setFillColor(selectorFontColor);
  nText.setFont(font);
  nText.setCharacterSize(selectorFontSize);
  nText.setPosition(sf::Vector2f(60, 408));
  nText.setFillColor(selectorFontColor);

  selectTextDir.setFont(font);	
  selectTextDir.setCharacterSize(selectFontSize);
  selectTextDir.setFillColor(dirFontColor);
 
  
  winSelector.create(sf::VideoMode(winSelectorWidth, winSelectorHeight), titreSelector);


  
  while (winSelector.isOpen()) {                                       // Événements
        sf::Event event;
        while (winSelector.pollEvent(event)){                         // Gestion des évènements de la fenêtre principale
    	       switch (event.type){
		       case sf::Event::Closed:                        // Fermeture de la fenêtre
			        onClose();
				     winSelector.close();
					  break;
             case sf::Event::KeyPressed:                    // touche pressée
			        //newDir(event);
			        break;
             case sf::Event::TextEntered:
           			newDir(event);
			         break;
             case sf::Event::MouseButtonPressed:
			  	      onClick(event);
                	break;
             case sf::Event::MouseWheelScrolled:
			 			onMouseWheel(event);
                	break;
             case sf::Event::MouseButtonReleased:
           			onMouseUp(event);
                	break;
             case sf::Event::MouseMoved:
			         onMouseMove(event);
                	break;
        				  					
		       default:                                      // on ne traite pas les autres types d'évènements
             break;
  	       	}            
        
        }
        // Clear screen
	winSelector.setActive(true);
	winSelector.pushGLStates();
   winSelector.clear(selectorBkgColor);

	winSelector.draw(selectBarH);
	winSelector.draw(selectDefPath);
   winSelector.draw(selectBarB);
	winSelector.draw(selectFileS);
	
	winSelector.draw(dossierPath);
	winSelector.draw(selectTrashF);
	winSelector.draw(selectNew);
	/*
   winSelector.draw(selectDown);
	winSelector.draw(selectUp);
	winSelector.draw(selectFileUp);
	winSelector.draw(selectFileDown);
	*/
   winSelector.draw(selectAnnuler);
	winSelector.draw(selectValider);
	winSelector.draw(nText);
	winSelector.draw(selectShap);
	winSelector.draw(selectBkgSlider);
	winSelector.draw(lineBloc);
	winSelector.draw(lineBloch);
	winSelector.draw(selectBkgSlider2);
	winSelector.draw(lineBloc2);
	winSelector.draw(lineBloch2);
	winSelector.draw(selectMSlider);
	winSelector.draw(selectMSlider2);

	drawDirs2();
	drawFiles();
        
	
   winSelector.display();
	winSelector.popGLStates();
	winSelector.setActive(false);
    }
  return rtf;
}
void selectFile::onClose(){
  if(nText.getString()!=""){
  	rtf=nText.getString();
  }else{
   rtf="";
  }
}

void selectFile::onKeyPressed(sf::Event e){
	
}
void selectFile::onMouseWheel(sf::Event e){
	int nposy;
	int dt;
	if (e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel && e.mouseWheelScroll.x>selectMSlider.getPosition().x && e.mouseWheelScroll.x<selectMSlider.getPosition().x+14 && e.mouseWheelScroll.y>selectMSlider.getPosition().y && e.mouseWheelScroll.y<selectMSlider.getPosition().y+selectMSlider.getSize().y){
		 if(e.mouseWheelScroll.delta==1){
		 	nposy=selectMSlider.getPosition().y-1;
		 	if(nposy<selectBkgSlider.getPosition().y+2){
		 		nposy=selectBkgSlider.getPosition().y+2;
		 		listDirsIndex2=listDirsIndex2=0;
		 	}else{
		 		listDirsIndex2=listDirsIndex2-1;
		 		selectMSlider.setPosition(sf::Vector2f(selectMSlider.getPosition().x,nposy));
		 	}
		 }else{
		 	nposy=selectMSlider.getPosition().y+1;
		 	if(nposy+selectMSlider.getSize().y>selectBkgSlider.getPosition().y+selectBkgSlider.getSize().y){
		 	   nposy=selectBkgSlider.getPosition().y+selectBkgSlider.getSize().y;
		 	}else{
		 		selectMSlider.setPosition(sf::Vector2f(selectMSlider.getPosition().x,nposy));
		 		listDirsIndex2=listDirsIndex2+1;
		 	}
		 }
	}
	if (e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel && e.mouseWheelScroll.x>selectMSlider2.getPosition().x && e.mouseWheelScroll.x<selectMSlider2.getPosition().x+14 && e.mouseWheelScroll.y>selectMSlider2.getPosition().y && e.mouseWheelScroll.y<selectMSlider2.getPosition().y+selectMSlider2.getSize().y){
		 if(e.mouseWheelScroll.delta==1){
		 	nposy=selectMSlider2.getPosition().y-1;
		 	if(nposy<selectBkgSlider2.getPosition().y+2){
		 		nposy=selectBkgSlider2.getPosition().y+2;
		 		listFilesIndex2=listFilesIndex2=0;
		 	}else{
		 		listFilesIndex2=listFilesIndex2-1;
		 		selectMSlider2.setPosition(sf::Vector2f(selectMSlider2.getPosition().x,nposy));
		 	}
		 }else{
		 	nposy=selectMSlider2.getPosition().y+1;
		 	if(nposy+selectMSlider2.getSize().y>selectBkgSlider2.getPosition().y+selectBkgSlider2.getSize().y){
		 	   nposy=selectBkgSlider2.getPosition().y+selectBkgSlider2.getSize().y;
		 	}else{
		 		selectMSlider2.setPosition(sf::Vector2f(selectMSlider2.getPosition().x,nposy));
		 		listFilesIndex2=listFilesIndex2+1;
		 	}
		 }
	}
	
}
void selectFile::onMouseMove(sf::Event e){
   std::cout << "mouse x: " << e.mouseMove.x << std::endl;
   std::cout << "mouse y: " << e.mouseMove.y << std::endl;
   if(e.mouseMove.x>0 && e.mouseMove.x<180 && e.mouseMove.y>36 && e.mouseMove.y<380){
   int cy=(((e.mouseMove.y-40)/18)*18)+40;
   selectShap.setSize(sf::Vector2f(172, 18));
   selectShap.setPosition(sf::Vector2f(4, cy));
   }
   if(e.mouseMove.x>200 && e.mouseMove.x<winSelectorWidth && e.mouseMove.y>36 && e.mouseMove.y<380){
   int cy=(((e.mouseMove.y-40)/18)*18)+40;
   selectShap.setSize(sf::Vector2f(272, 18));
   selectShap.setPosition(sf::Vector2f(204, cy));
   }
   if(buttonMouse==1 && e.mouseMove.x>selectMSlider.getPosition().x && e.mouseMove.x<selectMSlider.getPosition().x+14 && e.mouseMove.y>selectMSlider.getPosition().y && e.mouseMove.y<selectMSlider.getPosition().y+selectMSlider.getSize().y){
     int nposy=selectMSlider.getPosition().y+(e.mouseMove.y-lastPos1);
     if(nposy<selectBkgSlider.getPosition().y+2){
   		nposy=selectBkgSlider.getPosition().y+2;
     }
     if(nposy+selectMSlider.getSize().y>34+selectBkgSlider.getSize().y){
   		nposy=selectBkgSlider.getSize().y-selectMSlider.getSize().y+34;
     }
     selectMSlider.setPosition(selectMSlider.getPosition().x,nposy);
     listDirsIndex2=(int)nposy-38;
     drawDirs2();
     lastPos1=e.mouseMove.y;   
   }
   if(buttonMouse==1 && e.mouseMove.x>selectMSlider2.getPosition().x && e.mouseMove.x<selectMSlider2.getPosition().x+14 && e.mouseMove.y>selectMSlider2.getPosition().y && e.mouseMove.y<selectMSlider2.getPosition().y+selectMSlider2.getSize().y){
     int nposy=selectMSlider2.getPosition().y+(e.mouseMove.y-lastPos2);
     if(nposy<selectBkgSlider2.getPosition().y+2){
   		nposy=selectBkgSlider2.getPosition().y+2;
     }
     if(nposy+selectMSlider2.getSize().y>34+selectBkgSlider2.getSize().y){
   		nposy=selectBkgSlider2.getSize().y-selectMSlider2.getSize().y+34;
     }
     selectMSlider2.setPosition(selectMSlider2.getPosition().x,nposy);
     listFilesIndex2=(int)nposy-38;
     drawFiles();
     lastPos2=e.mouseMove.y;   
   }
}
void selectFile::onClick(sf::Event e){
   stringstream adr;
   if (e.mouseButton.button == sf::Mouse::Left){
      std::cout << "the left button was pressed" << std::endl;
      std::cout << "mouse x: " << e.mouseButton.x << std::endl;
      std::cout << "mouse y: " << e.mouseButton.y << std::endl;
   }
   if(e.mouseButton.x>0 && e.mouseButton.x<178 && e.mouseButton.y>36 && e.mouseButton.y<380){
     int id=((e.mouseButton.y-40)/18)+listDirsIndex2;
     selectType=0;
     adr << std::fixed <<getcwd(NULL,0) <<"/"<<vecDirs[id];
     dossierPath.setString(adr.str());
     listDirs(adr.str());
     readFiles();
     std::cout << "index dir: " <<vecDirs[id]<< "path :"<<adr.str()<<"listDirIndex "<<listDirsIndex2 << std::endl;
     adr.clear();
     adr.str("");
   }
   if(e.mouseButton.x>200 && e.mouseButton.x<winSelectorWidth && e.mouseButton.y>36 && e.mouseButton.y<400){
     int id=((e.mouseButton.y-40)/18)+listFilesIndex2;
     nText.setString(vecFs[id]);
     rtf=vecFs[id];
     newSelect=vecFs[id];
     selectType=1;
     if(vecFs[id].length()>5){
	     string extension=vecFs[id].substr(vecFs[id].length()-4,4);
	     string extension2=vecFs[id].substr(vecFs[id].length()-5,5);
		  if((extension2==".flac" || extension2==".aiff" || extension==".wav" || extension==".ogg" ) && e.mouseButton.x<232){
		  	 char file[vecFs[id].length() + 1];
	       strcpy(file, vecFs[id].c_str());
	       char commande[255];
	       snprintf(commande, 255, "%s %s/%s", simplePlayer,getcwd(NULL,0), file);
	       std::cout << "player : " << commande << std::endl;
		    system(commande);
		  }
	  }
   }
   if(e.mouseButton.x>362 && e.mouseButton.x<418 && e.mouseButton.y>406 && e.mouseButton.y<432){
     winSelector.close();
   }
   if(e.mouseButton.x>422 && e.mouseButton.x<470 && e.mouseButton.y>406 && e.mouseButton.y<432){
     winSelector.close();
   }
   if(e.mouseButton.x>2 && e.mouseButton.x<26 && e.mouseButton.y>8 && e.mouseButton.y<30){
     std::cout << "delete :"<<selectType<< std::endl;
     removeEventFileDir();
   }
   if(e.mouseButton.x>10 && e.mouseButton.x<34 && e.mouseButton.y>406 && e.mouseButton.y<430){
     createDir(txt);
   }
   if(e.mouseButton.x>selectMSlider.getPosition().x && e.mouseButton.x<selectMSlider.getPosition().x+14 && e.mouseButton.y>selectMSlider.getPosition().y && e.mouseButton.y<selectMSlider.getPosition().y+selectMSlider.getSize().y){
     buttonMouse=1;
     lastPos1=e.mouseButton.y;
   }
   if(e.mouseButton.x>selectMSlider2.getPosition().x && e.mouseButton.x<selectMSlider2.getPosition().x+14 && e.mouseButton.y>selectMSlider2.getPosition().y && e.mouseButton.y<selectMSlider2.getPosition().y+selectMSlider2.getSize().y){
     buttonMouse=1;
     lastPos2=e.mouseButton.y;
   }
}
void selectFile::onMouseUp(sf::Event e){
	buttonMouse=0;
}

void selectFile::newDir(sf::Event e){
     int key=e.key.code;
     int rtn=0;
     if (e.text.unicode < 128){
	switch (key){
	     case 8:
	       	txt=txt.substr(0,txt.length()-1);
	       	nText.setString(txt);
	 	break;
	     case 13:
	         rtn=createDir(txt);
	         nText.setString("");
		break;
	     default:
		txt=txt+static_cast<char>(e.text.unicode);
		nText.setString(txt);
		break;
       	}
     }
}

void selectFile::removeEventFileDir(){
  bool rt=newInfo.drawInfo(0, fGui, "Alerte");
  std::cout << "Deleted " << rt <<"\n";
  if(rt==1){
   if(selectType==0){
     string ddir=dossierPath.getString();
     chdir("..");
     std::uintmax_t n = boost::filesystem::remove_all(ddir);
     int rd=listDirs(getcwd(NULL,0));
     int rf=readFiles();
     std::cout << "Deleted " << n << "directories \n";
   }else{
     std::uintmax_t n = boost::filesystem::remove(newSelect);
     int rf=readFiles();
     nText.setString("");
     drawFiles();
     std::cout << "Deleted " << n << " files\n";
   }
  }  
}
int selectFile::createDir(string dir){
  int rt=0;
    if(dir.size()>0){
       if(boost::filesystem::create_directory(dir)) {
         std::cout << "Success" << "\n";
         int rd=listDirs(".");
       }else{
	 rt=1;
	 bool rt=newInfo.drawInfo(1, fGui, "Alerte");
	 // std::cout << "Le dossier n'a pas pu être créé" << "\n";
       }
    }
    nText.setString("");
    txt="";
    return rt;
}

int selectFile::listDirs(string cdir){
  char * cstr = new char [cdir.length()+1];
  std::strcpy (cstr, cdir.c_str());
  int ncwd=chdir(cstr);
  listDirsIndex2=0;
  listFilesIndex2=0;
  nText.setString("");
  vecDirs.clear();
  DIR * rep = opendir(getcwd(NULL,0));
  if (rep != NULL){
     struct dirent * ent;
     string dname;
     while ((ent = readdir(rep)) != NULL){
         dname=ent->d_name;
         if(boost::filesystem::is_directory(dname) && dname!="."){
           vecDirs.push_back (dname);
         }
     }
  }
  std::sort (vecDirs.begin(), vecDirs.end());
  closedir(rep);
  if(vecDirs.size()<20){
   	selectMSlider.setSize(sf::Vector2f(14, 360));
  }else{
   	int nbs=360-(vecDirs.size()-19);
   	selectMSlider.setSize(sf::Vector2f(14, nbs));
  }
  selectMSlider.setPosition(selectMSlider.getPosition().x,selectBkgSlider.getPosition().y+2);
  drawDirs2();
  return 0;
}
void selectFile::drawDirs(){
    int maxf;
    string vs;
    int bf=listDirsIndex*19;
    if(bf+19>vecDirs.size()){
		maxf=vecDirs.size();
    }else{
		maxf=bf+19;
    }
    int j=0;
    int i=bf;
    dossierPath.setString(getcwd(NULL,0));
    while(i<maxf){
      vs=vecDirs[i].substr(0,20);
    	if(vs.length()<vecDirs[i].size()){
    	  vs=vs+"...";
    	}
	selectTextDir.setString(vs);
	selectTextDir.setPosition(sf::Vector2f(8, 40+(j*18)));
	selectTextDir.setFillColor(dirFontColor);
	winSelector.draw(selectTextDir);
	j++;
	i++;
   }
   
}
void selectFile::drawDirs2(){
    int maxf;
    string vs;
    sf::String vs2;
    int bf=listDirsIndex2;
    
    if(bf+19>vecDirs.size()){
		maxf=vecDirs.size();
    }else{
		maxf=bf+19;
    }
    
    int j=0;
    int i=bf;
    dossierPath.setString(getcwd(NULL,0));
    while(i<maxf){
      vs=vecDirs[i].substr(0,20);
    	if(vs.length()<vecDirs[i].size()){
    	  vs=vs+"...";
    	}
   vs2 = sf::String::fromUtf8(vs.begin(), vs.end());
	selectTextDir.setString(vs2);
	selectTextDir.setPosition(sf::Vector2f(8, 40+(j*18)));
	selectTextDir.setFillColor(dirFontColor);
	winSelector.draw(selectTextDir);
	j++;
	i++;
   }
}

int selectFile::readFiles(){
    DIR * rep = opendir(getcwd(NULL,0));
    vecFs.clear();
    int i=0;
    if (rep != NULL){
        struct dirent * ent;
        string vname;
        while ((ent = readdir(rep)) != NULL){
          vname=ent->d_name;
          if(boost::filesystem::is_regular_file(vname) && vname.substr(vname.length()-1,1)!="~" ){
            vecFs.push_back (vname);
            i++;
          }
        }
     std::sort (vecFs.begin(), vecFs.end());
     closedir(rep);
    }
    if(vecFs.size()<20){
   	selectMSlider2.setSize(sf::Vector2f(14, 360));
    }else{
   	int nbs=360-(vecFs.size()-19);
   	selectMSlider2.setSize(sf::Vector2f(14, nbs));
    }
    selectMSlider2.setPosition(selectMSlider2.getPosition().x,selectBkgSlider2.getPosition().y+2);  
    return 0;
}
void selectFile::drawFiles(){
    int maxf=19;
    string vs;
    sf::String vs2;
    int bf=listFilesIndex2;
    if(bf+19>vecFs.size()){
      maxf=vecFs.size();
    }else{
    maxf=bf+19;
    }
    int j=0;
    int i=bf;
    while(i<maxf){
	vs=vecFs[i].substr(0,30);
	vs2 = sf::String::fromUtf8(vs.begin(), vs.end());
	selectTextDir.setString(vs2);
   selectTextDir.setPosition(sf::Vector2f(234, 40+(j*18)));
	selectTextDir.setFillColor(fileFontColor);
	if(vecFs[i].length()>5){
		string extension=vecFs[i].substr(vecFs[i].length()-4,4);
		string extension2=vecFs[i].substr(vecFs[i].length()-5,5);
		if(extension2==".flac" || extension2==".aiff" || extension==".wav" || extension==".ogg"){
		  speaker.setPosition(sf::Vector2f(210, 40+(j*18)));
		  winSelector.draw(speaker);
		}
	}
	winSelector.draw(selectTextDir);
	j++;
	i++;
   }
}
