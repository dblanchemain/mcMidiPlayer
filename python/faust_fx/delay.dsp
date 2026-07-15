ma = library("maths.lib");
ba = library("basics.lib");
de = library("delays.lib");
si = library("signals.lib");
an = library("analyzers.lib");
fi = library("filters.lib");
os = library("oscillators.lib");
no = library("noises.lib");
ef = library("misceffects.lib");
co = library("compressors.lib");
ve = library("vaeffects.lib");
pf = library("phaflangers.lib");
re = library("reverbs.lib");
en = library("envelopes.lib");
ro = library("routes.lib");

import("stdfaust.lib");
//---------------------------------DELAY----------------------------
//
//  A stereo smooth delay with a feedback control
//  
//  This example shows how to use sdelay, a delay that doesn't
//  click and doesn't transpose when the delay time is changed
//
//------------------------------------------------------------------

delay = ba.bypass1(cbp,voice)
	with {
		delay_group(x) = vgroup("DELAY", x);
		cbp =delay_group(vgroup("[0]",checkbox("Bypass	[tooltip: When this is checked, Delay
		has no effect]")));
		voice   = delay_group(vgroup("[1]",(+ : de.sdelay(N, interp, dtime)) ~ *(fback)));
		N = int(2^19); 
		interp  = hslider("interpolation[unit:ms][style:knob]",10,1,100,0.1)*ma.SR/1000.0; 
		dtime   = hslider("delay[unit:ms][style:knob]", 0, 0, 5000, 0.1)*ma.SR/1000.0;
		fback   = hslider("feedback[style:knob]",0,0,100,0.1)/100.0; 
	};

process = _<:delay:>_;
