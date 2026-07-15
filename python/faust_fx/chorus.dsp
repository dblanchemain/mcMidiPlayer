
declare name "chorus";

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



//-----------------------------------------------
// 			Chorus
//-----------------------------------------------
levelChorus	= vslider("level[name:Level]", 0.5, 0, 1, 0.01);
freqChorus	= vslider("freq[name:Freq]", 3, 0, 10, 0.01);
dtimeChorus	= vslider("delay[name:Delay]", 0.02, 0, 0.2, 0.01): si.smooth(0.999);
depthChorus	= vslider("depth[name:Depth]", 0.02, 0, 1, 0.01);

tbloscChorus(n,f,freqChorus,mod)	= (1-d)*rdtable(n,wform,i&(n-1)) +
			  d*rdtable(n,wform,(i+1)&(n-1))
with {
	wform 	= ba.time*(2.0*ma.PI)/n : f;
	phase		= freqChorus/ma.SR : (+ : ma.decimal) ~ _;
	modphase	= ma.decimal(phase+mod/(2*ma.PI))*n;
	i		= int(floor(modphase));
	d		= ma.decimal(modphase);
};

schorus(dtimeChorus,freqChorus,depthChorus,phase,x)
			= x+levelChorus*de.fdelay(1<<16, t, x)
with {
	t		= ma.SR*dtimeChorus/2*(1+depthChorus*tbloscChorus(1<<16, sin, freqChorus, phase));
};
chorus=_,_:hgroup("Chorus", (left, right)):_,_
with {
	left		= schorus(dtimeChorus,freqChorus,depthChorus,0);
	right		= schorus(dtimeChorus,freqChorus,depthChorus,ma.PI/2);
};

process = chorus;