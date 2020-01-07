#ifndef DEF_SELECTFILE
#define DEF_SELECTFILE

#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdlib.h>
#include <string>

#include "info.h"

class selectFile{

 public :
  
  selectFile();
  ~selectFile();
  void initSelector(int type, std::string dossier,std::string fG, std::string fT,std::string Th,std::string fL, std::string simpleLecteur);
  std::string  selector();

  void newDir(sf::Event e);
  int createDir(std::string dir);
  int listDirs(std::string cdir);
  void drawDirs();
  int readFiles();
  void listFiles();
  void drawFiles();
  void removeEventFileDir();
  void drawDirs2();
  void drawFiles2();
  Info newInfo;
  
  void onClose();
  void onMouseWheel(sf::Event e);
  void onKeyPressed(sf::Event e);
  void onClick(sf::Event e);
  void onMouseUp(sf::Event e);
  void onMouseMove(sf::Event e);
  

 private :
  
  bool config=0;
  int type=0;
  std::string fGui;
  std::string fTheme;
  std::string dossier;
  sf::Text dossierPath;
  std::string fLang;
  char simplePlayer[255];
  sf::RenderWindow winSelector;
  float winSelectorWidth=500;
  float winSelectorHeight=436;
  int menuBarHeight;
  std::string titreSelector;
  std::string Annuler;
  std::string Valider;
  sf::Font font;
  sf::Color selectorBarColor;
  sf::Color selectorBkgColor;
  sf::Color selectorFontColor;
  int selectFontSize;
  sf::Color dirFontColor;
  sf::Color fileFontColor;
  int selectorFontSize;
  
  sf::RectangleShape selectBkgSlider;
  sf::RectangleShape selectBkgSlider2;
  sf::RectangleShape selectMSlider;  
  sf::RectangleShape selectMSlider2;
  int buttonMouse=0;
  int lastPos1;
  int lastPos2;
  
  std::string txt;
  sf::Text nText;
  std::string newSelect;
  std::vector <std::string> vecDirs;
  std::vector <std::string> vecFs;
  sf::Text selectTextDir;
  int listDirsIndex=0;
  int listDirsIndex2=0;
  int listFilesIndex2=0;
  int selectType;
  sf::RectangleShape selectShap;
  
  sf::Sprite speaker;
  std::string rtf="";
};
#endif
