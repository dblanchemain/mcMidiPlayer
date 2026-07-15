ba = library("basics.lib");
ef = library("misceffects.lib");
import("stdfaust.lib");

hfc = vslider("HPF Freq",20,20,20000,1);
lfc = vslider("LPF Freq",1000,20,20000,1);

process = fi.highpass(3,hfc) : fi.lowpass(3,lfc);
