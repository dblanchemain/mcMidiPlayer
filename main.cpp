#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdlib.h>
#include <iomanip>
#include <locale>
#include <sstream>
#include <ctgmath>
#include <string>


#include "Application.h"


using namespace std;


int main (int argc, char* argv[]){
  
  Application nwApp(argv[1]);
  nwApp.appPreferences();
  nwApp.appTheme();
  nwApp.appInitMenu();
  
  nwApp.appWindow();


  
  
  
      
    
  
         return EXIT_SUCCESS;
  
}

	
