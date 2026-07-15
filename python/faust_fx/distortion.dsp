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



declare name "distortion";
declare version "0.0";
declare author "JOS, revised by RM";
declare description "Distortion demo application.";

import("stdfaust.lib");

process = ef.cubicnl_nodc(drive:si.smoo,offset:si.smoo)
with{
    cnl_group(x)  = vgroup("CUBIC NONLINEARITY cubicnl [tooltip: Reference:
        https://ccrma.stanford.edu/~jos/pasp/Cubic_Soft_Clipper.html]", x);
    drive = cnl_group(hslider("[1] Drive [tooltip: Amount of distortion]",
        0, 0, 1, 0.01));
    offset = cnl_group(hslider("[2] Offset [tooltip: Brings in even harmonics]",
        0, 0, 1, 0.01));
};
