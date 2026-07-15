//
//						metaSurface libFaust
//
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



declare name "MoogLadder";
declare version "0.0";
declare author "Romain Michon";
declare description "MoogLadder.";

import("stdfaust.lib");

LadderMoog=hgroup("MoogLadder",ve.moogLadder(normFreq,Q)) with{
Q = vslider("Q",1,0.7072,25,0.01);
normFreq = vslider("freq",0.1,0,1,0.001):si.smoo;
};
process = LadderMoog;
