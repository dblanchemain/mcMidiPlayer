#ifndef DEF_DIALOG
#define DEF_DIALOG

#include <SFML/Graphics.hpp>

#include <iostream>
#include <stdlib.h>
#include <string>


class Dialog{

 public :
  
  Dialog();
  ~Dialog();
  
  void initDialog();
  std::string drawDialog(int ftype, std::string fG, std::string titreWDialog, sf::Font font,sf::Color fColor, int fSize);
  void newText(sf::Event e);

  
  
  void onClose();
  void onEventResized(sf::Event);
  void mouseWheel(sf::Event);
  void onKeyPressed(sf::Event e);
  void onClick(sf::Event e);
  void onMouseUp(sf::Event e);
  void onMouseMove(sf::Event e);
  

 private :
  
  int winDialogWidth=338;
  int winDialogHeight=141;
  int dialogBarHeight=36;
  int type=0;
  std::string fGui;
  std::string fTheme;
  std::string Theme;
  sf::RenderWindow winDialog;
  sf::Font font;
  std::string titreWinDialog;
  
  std::string txt="";
  sf::Text nText;

};
#endif
