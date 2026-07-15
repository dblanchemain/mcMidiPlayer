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



declare name "peakEQ";
declare version "0.0";
declare description "peakEqualizer.";

import("stdfaust.lib");

peakEqualizer=vgroup("PeakEq",fi.peak_eq(level,freq,Q))
    with {
        level = hslider("[2]Level[unit:dB][style:knob][acc:2 1 -10 0 10][tooltip: boost Level>0 or cut Level<0)", 0, -40, 32, 0.01):min(32):max(-40);
        freq = hslider("[1]Peak Frequency[unit:Hz][acc:0 1 -10 0 10][scale:log]", 440, 50, 11000, 0.01):si.smooth(0.999);
        Q = hslider("Q[unit:Hz][acc:2 0 -10 0 10]", 50, 20, 200, 1):si.smooth(0.999):min(200):max(20);
    };
process = peakEqualizer;
