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



declare name "ringmodulator";
declare version "0.0";
declare description "ringModulator.";

import("stdfaust.lib");

//-------------------------Ring modulator---------------------------
// Ring Modulator effect  application.
//
// #### Usage
//
// ```
// _:ringModulator : _;
// ```
//------------------------------------------------------------
ringModulator = ba.bypass1(rmbp,ringM)
with{
	groupRM(x)=vgroup("RING MODULATOR",x);
	rmbp = groupRM(checkbox("[0] Bypass	[tooltip: When this is checked, the phaser
		has no effect]"));
	ringM=groupRM(hgroup("[1]",*(1-rmdepth*(os.osc(rmfreq)*0.5+0.5))));
	
	rmfreq = vslider("frequency",5,0.1,1000,0.01) : si.smooth(0.999);
	rmdepth = vslider("depth",0,0,1,0.01) : si.smooth(0.999);
};

process = ringModulator;
