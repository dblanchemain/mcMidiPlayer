ba = library("basics.lib");
ef = library("misceffects.lib");
import("stdfaust.lib");

lfc = vslider("LPF Freq",1000,20,20000,1);

process = fi.lowpass(3,lfc);
