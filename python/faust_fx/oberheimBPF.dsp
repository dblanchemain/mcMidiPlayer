declare name "oberheimBPF";
declare version "0.0";
declare author "Eric Tarr";
declare description "Band-Pass Oberheim filter.";

import("stdfaust.lib");

BPFOberheim = hgroup("oberheimBPF",ve.oberheimBPF(normFreq,Q)) with{
Q = vslider("Q",1,0.5,10,0.01);
normFreq = vslider("freq",0.5,0,1,0.001):si.smoo;
};
process =BPFOberheim;
