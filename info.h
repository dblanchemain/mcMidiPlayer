#ifndef DEF_INFO
#define DEF_INFO

#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdlib.h>
#include <string>


class Info{

 public :
  
  Info();
  ~Info();
  
  void initInfo();
  bool drawInfo(int ftype, std::string fG, std::string titreWDialog);

  
  
  void onClose();
  void onEventResized(sf::Event);
  void mouseWheel(sf::Event);
  void onKeyPressed(sf::Event e);
  void onClick(sf::Event e);
  void onMouseUp(sf::Event e);
  void onMouseMove(sf::Event e);
  

 private :
  
  int winInfoWidth=338;
  int winInfoHeight=141;
  int infoBarHeight=36;
  int type=0;
  std::string fGui;
  std::string fTheme;
  std::string Theme;
  sf::RenderWindow winInfo;
  sf::Font font;
  std::string titreWinInfo;

 
  
  bool  rtf=0;
};
#endif
