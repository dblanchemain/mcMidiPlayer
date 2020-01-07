#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <libgen.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>
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

#include <sndfile.h>

using namespace std;

list<GUI*> GUI::fGuiList;
ztimedmap GUI::gTimedZoneMap;


int main(int argc, char* argv[]){
  char name[256];
  char filename[256];
  char rcfilename[256];
  char* home = getenv("HOME");
  
  snprintf(name, 255, "%s", basename(argv[0]));
  snprintf(filename, 255, "%s", argv[argc-1]);
  snprintf(rcfilename, 255, "%s/.%s-rc", home, basename(argv[argc-1]));
  std::cout << "name : " << name<<" filename "<<filename<<" rcfilename "<<rcfilename<< std::endl;
  
  bool is_osc = isopt(argv, "-osc");
  bool is_midi = isopt(argv, "-midi");
    
  dsp_factory* factory = nullptr;
  dsp* DSP = nullptr;
  MidiUI* midiinterface = nullptr;
  jackaudio_midi audio;
  GUI* oscinterface = nullptr;
  string error_msg;
    
  cout << "Libfaust version : " << getCLibFaustVersion () << endl;
  
  factory = createDSPFactoryFromFile(filename, 0,NULL, "", error_msg, -1);
  if (!factory) {
     cerr << "Cannot create factory : " << error_msg;
     exit(EXIT_FAILURE);
  }
  std::cout << "Factory : " << factory<< std::endl;
  cout << "getCompileOptions " << factory->getCompileOptions() << endl;
  
  DSP = factory->createDSPInstance();
  if (!DSP) {
     cerr << "Cannot create instance "<< endl;
     exit(EXIT_FAILURE);
  }
  GUI* interface = new GTKUI(name,0,NULL);
  DSP->buildUserInterface(interface);
  FUI* finterface = new FUI();
  DSP->buildUserInterface(finterface);
  SoundUI* soundinterface = new SoundUI();
  DSP->buildUserInterface(soundinterface);
  
  if (is_osc) {
  	int argc1=3;
  	char* argv1[64];
  	argv1[0]=filename;
  	argv1[1]=(char*)"-xmit";
  	argv1[2]=(char*)"1";
  	//cout <<argv1[0]<<argv1[1] << " : " << argv1[2]<< endl;
  	//argv[3]=(char*)"-port";
  	//argv[4]=(char*)"5540";
    oscinterface = new OSCUI(filename,argc1,argv1);
    DSP->buildUserInterface(oscinterface);
  }
  if (is_midi) {
        midiinterface = new MidiUI(&audio);
        DSP->buildUserInterface(midiinterface);
  }
  if (!audio.init(basename(filename), DSP)) {
     cout << "audio.init : " << 0<< endl;
  }
  finterface->recallState(rcfilename);
  audio.start();

  if (is_osc) {
  	  oscinterface->run();
  }
  if (is_midi) {
     midiinterface->run();
  }
  interface->run();
  
  audio.stop();
  
  finterface->saveState(rcfilename);
  
  delete DSP;
  delete interface;
  delete finterface;
  delete oscinterface;
  delete midiinterface;
  delete soundinterface;
  
  deleteDSPFactory(static_cast<llvm_dsp_factory*>(factory));
  
  return 0;
  
}
