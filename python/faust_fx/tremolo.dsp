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


//-------------------------Tremolo---------------------------
//
// #### Usage
//
// _:tremolo: _;
//
//------------------------------------------------------------

trem_R1 = 2700;
trem_Ra = 1e6;
trem_Rb = 300;
trem_b = exp(log(trem_Ra)/log(trem_Rb)) - exp(1);
trem_dTC = 0.06;
trem_minTC = log(0.005/trem_dTC);

trem_cds = ((_ <: _,_),_ : _+(1-alpha)*_) ~ (_<:*(alpha)) with {
    iSR = 1/ma.SR;
    dRC = trem_dTC * exp(*(trem_minTC));
    alpha = 1 - iSR / (dRC + iSR);
};

trem_vactrol = pow(_,1.9) : trem_cds : *(trem_b) + exp(1) : exp(log(trem_Ra)/log) : trem_R1/(_ +trem_R1);

/* triangle oscillator (not bandlimited, frequency is approximate) */

trem_trianglewave(freq) = _ ~ (_ <: _ + hyst) : /(periodsamps) with {
   if(c,t,e) = select2(c,e,t);
    hyst(x) = if(_ > 0, 2 * (x < periodsamps) - 1, 1 - 2 * (x > 0)) ~ _;
    periodsamps = int(ma.SR / (2*float(freq)));
};

/* tremolo unit, using triangle or sine oscillator as lfo */

stremolo(freq, depth) = lfo * depth + 1 - depth : trem_vactrol with {
    sine(freq) = (os.oscs(freq) + 1) / 2 : max(0); // max(0) because of numerical inaccuracy
    SINE=checkbox("SINE[enum:triangle|sine|square]");
    lfo = select3(SINE, trem_trianglewave(freq), sine(freq), os.lf_squarewavepos(freq));
};

trem_wet = vslider("wet_dry[name:Dry/Wet][tooltip:percentage of processed signal in output signal]",  100, 0, 100, 1) : /(100);
trem_dry = 1 - trem_wet;
tremolo=_,_:hgroup("Tremolo",*(trem_dry),(*(trem_wet):*(stremolo(vslider("freq[name:Freq]",5,0.1,50,0.1),vslider("depth[name:Depth]",0.5,0,1,0.01))))):_,_;


process = tremolo;
