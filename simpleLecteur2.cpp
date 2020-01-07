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
  SF_INFO sfinfo;
  const char *path=argv[argc-1];
  SNDFILE* sndfile=sf_open(path, SFM_READ, &sfinfo) ;
  sf_close(sndfile) ;
  cout << "canaux : " << sfinfo.channels<<"Sample rate = "<< sfinfo.samplerate<<"frames : "<<sfinfo.frames<< endl;
  string nameFile="simplePlayer.dsp";
  
  string prog;
	 prog=prog+"import(\"stdfaust.lib\");";
	 prog=prog+"import(\"soundfiles.lib\");";
	 prog=prog+"ds=soundfile(\"[url:{\'"+argv[argc-1]+"\'}]\","+to_string(sfinfo.channels)+");";
	 
	 prog=prog+"vmeter(x)= attach(x, envelop(x) : vbargraph(\"[2][unit:dB]\", -70, +5));";
	 prog=prog+"envelop = abs : max ~ -(1.0/ma.SR) : max(ba.db2linear(-70)) : ba.linear2db;";
	 prog=prog+"sample1 = so.sound(ds, 0);";
	 prog=prog+"gain = vslider(\"[0]gain\",0.1,0,1,0.01) : si.smoo;";
    prog=prog+"gate = button(\"[1]gate\");";
    prog=prog+"lect(x)=hgroup(\""+to_string(sfinfo.samplerate) +"\",x);";
    prog=prog+"tdec(x) = lect(vgroup(\"Player\",x));";
    
    prog=prog+"lgain=tdec(gain);";
    prog=prog+"lgate=tdec(gate);";
    prog=prog+"lmet=lect(par(j,"+to_string(sfinfo.channels)+",hgroup(\"c%2j\",vmeter)));";
    prog=prog+"smp1 = sample1.play(lgain, lgate):lmet;";
    prog=prog+"process = smp1;";
  
  cout << "player : " << prog<< endl;
  
  char name[256];
  char nameAudio[256];
  char rcfilename[256];
  char* home = getenv("HOME");
  string s = "simplePlayer.dsp";
  char filename[s.length() + 1];
  strcpy(filename, s.c_str());
  snprintf(name, 255, "%s", basename(argv[0]));
  snprintf(nameAudio, 255, "%s", basename(argv[argc-1]));
  snprintf(rcfilename, 255, "%s/.%s-rc", home, name);
  std::cout << "name : " << name<<"filename "<<filename<<"rcfilename "<<rcfilename<< std::endl;
  
  bool is_osc = isopt(argv, "-osc");
    
  dsp_factory* factory = nullptr;
  dsp* DSP = nullptr;
  MidiUI* midiinterface = nullptr;
  jackaudio_midi audio;
  GUI* oscinterface = nullptr;
  string error_msg;
    
  cout << "Libfaust version : " << getCLibFaustVersion () << endl;
  
  factory = createDSPFactoryFromString("simplePlayer",prog, 0,NULL, "", error_msg, -1);
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
  GUI* interface = new GTKUI(nameAudio,0,NULL);
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
  	 argv1[2]=(char*)"0";
  	 //cout <<argv1[0]<<argv1[1] << " : " << argv1[2]<< endl;
  	 //argv[3]=(char*)"-port";
  	 //argv[4]=(char*)"5540";
    oscinterface = new OSCUI(filename,argc1,argv1);
    DSP->buildUserInterface(oscinterface);
  }

  if (!audio.init(basename(filename), DSP)) {
     cout << "audio.init : " << 0<< endl;
  }
  finterface->recallState(rcfilename);
  audio.start();

  if (is_osc) {
  		oscinterface->run();
  }
  
  interface->run();
  
  audio.stop();
  
  finterface->saveState(rcfilename);
  
  delete DSP;
  delete interface;
  delete finterface;
  delete oscinterface;
  delete soundinterface;
  
  deleteDSPFactory(static_cast<llvm_dsp_factory*>(factory));
  
  return 0;
 
}
