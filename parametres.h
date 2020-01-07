#ifndef DEF_PARAMETRES
#define DEF_PARAMETRES

#include <SFML/Graphics.hpp>

#include <iostream>
#include <stdlib.h>
#include <string>
#include "selectFile.h"

class Parametres{

 public :
  
  Parametres();
  ~Parametres();
  
  void initParametres(std::string parametres);
  std::string drawParametres(int ftype, std::string fG, std::string fParametres, std::string titreWDialog, sf::Font font,sf::Color fColor, int fSize, sf::Color ongSelColor, sf::Color ongColor, sf::Color fOng, int fOngSize);
  void newText1(sf::Event e);
  void valider();
  void defaut();
  void saveParametres();
  
  void onClose();
  void onEventResized(sf::Event);
  void mouseWheel(sf::Event);
  void onKeyPressed(sf::Event e);
  void onClick(sf::Event e);
  void onMouseUp(sf::Event e);
  void onMouseMove(sf::Event e);
  

 private :
  
  int winParamWidth=500;
  int winParamHeight=430;
  int ParamBarHeight=36;
  int type=0;
  std::string fGui;
  std::string fTheme;
  std::string Theme;
  std::string diversLang;
  std::string simpleLecteur;
  std::string dossierConfig;

  sf::RenderWindow winParam;
  sf::Font font;
  std::string titreWinParam;

  int ongletActif=0;
  sf::CircleShape circle;
  sf::CircleShape circle1;
  sf::CircleShape circle2;
  sf::CircleShape circle3;
  
  std::string Lang="Fr";
  //std::string Theme="base";
  std::string Editeur="openoffice";
  std::string Banques="";
  sf::Text nEditeur;
  sf::Text nBanques;
  std::string midiPlayer;
  int flagTxt=0;
  
  std::string txt="";
  sf::Text nText;

};
#endif
