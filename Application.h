#ifndef DEF_APPLICATION
#define DEF_APPLICATION

#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "selectFile.h"
#include "parametres.h"




/** Construction de l'application */

class Application{

 public :

/**
* Le constructeur définie les chemins de base de l'application en utilisant le premier paramètre de lancement de celle-ci.
* \param dossier
* \param dossierConfig path de référence de l'application
* \param fichierParametres path du fichier de paramètres (_parametres.conf_)
* \param appGui path du dossier Gui
*/
  Application(std::string dossier);
  ~Application();

/**
* Lecture des préférences définies par défaut ou enregistrées dans le fichier parametres.conf
* \param DAW, OSC_Destination, OSC_Port, OSC_Réception, OSC_Port_récep.,Thème,Langue 
*/
  void appPreferences();
/**
* Lecture des paramètres du thème (police de caractères, couleurs etc...)
*/
  void appTheme();
/**
* Construction de la fenêtre principale de l'application
*/ 
  void appWindow();
/**
* Construction du menu principal
*/
  void appInitMenu();
/**
* Intégration du menu dans la fenêtre
*/
  void appDrawMenu();
  
/** 
* Méthodes associées au menu Dossier
*
* Nouvelle session
*/ 
  void menuSession();
/**
* Méthodes associées au menu Dossier
*
* Ouverture d'une session
*/
  void menuOpen();
/**
* Méthodes associées au menu Dossier
*
* Enregistrement de la session courante
*/
  void menuSave();

/**
* Méthodes associées au menu Dossier
*
* Éditeur associée à l'application. Cet éditeur est défini dans les préférences.
*/
  void menuComment();
/** 
* Méthodes associées au menu Dossier Édition
*
* Annuler la dernière action
*/ 
  void menuDel();
/**
* Méthodes associées au menu Édition
*
* Refaire l'action
*/
  void menuAllDel();

/**
* Méthodes associées au menu Édition
*
* Définir les préférences de l'application
*/
  void menuPreferences();

  void appShapWindow();
  void newDefText(sf::Event e);
  
  std::string getLang() const;
  std::string getGui() const;
  
  void onClose();
  void onEventResized(sf::Event);
  void mouseWheel(sf::Event);
  void onKeyPressed(sf::Event e);
  void onClick(sf::Event e);
  void onMouseUp(sf::Event e);
  void onMouseMove(sf::Event e);
  void onMouseButtonUp(sf::Event e);
  
  void saveBank();
  void readBank();
  void genDSP();
  void deleteInstrument();
  void newInstrument();
  void deleteKey();

 private :

  
  float winWxScale;
  float winWyScale;
  sf::RenderWindow window;
  std::string appName;

  std::string dossierConfig;
  std::string appGui;
  std::string fichierParametres;
  std::string fichierTheme;
  std::string fichierLang;
  std::string diversLang;
  std::string usrSessions;
  std::string DAW; 		      // Structure configuration        
  std::string Destination;
  int PortD;
  std::string Reception; 
  int PortR; 
  std::string Theme;
  std::string Lang;
  std::string editeur;
  std::string simpleLecteur;

  int windowAppWidth;
  int windowAppHeight;
  sf::Color AppBackGround;
  sf::Color AppBackGroundMenuBar;
  sf::Color bkgWindowColor;
  sf::Color keyFileColor;
  int menuBarHeight;
  sf::RectangleShape winSliderDroit;
  sf::RectangleShape winSliderMDroit;
  int offsetWorkSpace;
  bool flagSlider=0;
  int lastPosX;
  int lastPosY;
  sf::Sprite work;
  sf::RectangleShape bkgWindow;
  sf::RectangleShape bankNameShape;
  sf::RectangleShape fileDef;
  sf::RectangleShape bankCanauxShape;
  sf::RectangleShape defEnveloppe;
  sf::Color menu1BkgColor;
  sf::Color menu1bBkgColor;
  sf::Color itemShapBkgColor;
  sf::Color itemFontColor;
  sf::Color itemShapFontColor; 
  sf::Text menuItems[5][10];
  sf::Font font;
  bool selectBarMenu=0;
  bool selectM1=0;
  bool selectM2=0;
  bool selectM3=0;
  bool selectM4=0;
  bool selectM5=0;
  
  int selectM1I=0;   
  int selectM2I=0;
  
  sf::RectangleShape menu;
  sf::RectangleShape itemShap;
  sf::RectangleShape menu1;
  sf::RectangleShape menu1b;
  sf::RectangleShape menu2;
  sf::RectangleShape menu2b;
  
  std::string labelNameBk;
  std::string labelNbchannels;
  std::string labelDefKey;
  std::string labelNfiles;

  selectFile fileSelector;
  sf::Color selectorBarColor;
  sf::Color selectorBkgColor;
  sf::Color selectorFontColor;
  sf::Color dialogFontColor;
  int dialogFontSize;

  Parametres newParametres;
  sf::Color paramOngSelectColor;
  sf::Color paramOngColor;
  sf::Color paramFontColorOng;
  int paramFontSizeOng;
  
  int textDef=0;
  std::string apptxt;
  sf::Text bkgName;
  sf::Text nbCanaux;
  sf::Text fileName[128];
  std::string midiPlayer;
  
  int keyActive=0;
  bool flagActive=0;
  sf::Texture selectb7;
  sf::Texture selectb8;
  sf::Sprite boutonActiver;
  std::string fileNameF[128];
  int keyBoucleFlag[128];
  int keyGlobalFlag[128];
  Info newInfo;
  bool flagBoucle=0;
  bool flagGlobal=0;
};

#endif
