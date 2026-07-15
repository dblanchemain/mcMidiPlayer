
declare name "combfilter";

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

//----------------------------comb filter-------------------------
//
// #### Usage
//
// ```
// _: combfilter : _;
// ```
//------------------------------------------------------------
combfilter= vgroup("CombFilter",fi.fb_fcomb(maxdel,del,b0,aN))
 with {
	   maxdel = 1<<16;
	   freq = 1/(hslider("Frequency[acc:0 1 -10 0 10]", 2500,100,20000,0.001)):si.smooth(0.99);
	   del = freq *(ma.SR) : si.smooth(0.99);
	   b0 = 1;
	   aN = hslider("Intensity[acc:1 0 -10 0 10]", 80,0,100,0.01)*(0.01):si.smooth(0.99):min(0.999):max(0);
};
process =combfilter;
