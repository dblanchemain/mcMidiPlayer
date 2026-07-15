//----------------------------`(dm.)compressor_demo`-------------------------
// Limitor.
//
// #### Usage
//
// ```
// _,_ :limitor: _,_
// ```
//------------------------------------------------------------
declare limitor author "Julius O. Smith III";
declare limitor licence "MIT";
import("stdfaust.lib");
limitor_stereo = ba.bypass2(cbp,limitor_stereo)
with{
    comp_group(x) = vgroup("LIMITOR [tooltip: Reference:
        http://en.wikipedia.org/wiki/Dynamic_range_compression]", x);

    meter_group(x) = comp_group(hgroup("[0]", x));
    knob_group(x) = comp_group(hgroup("[1]", x));

    cbp = meter_group(checkbox("[0] Bypass    [tooltip: When this is checked, the compressor
        has no effect]"));
    gainview = co.compression_gain_mono(4,-6,0.008,0.5) : ba.linear2db :
    meter_group(hbargraph("[1] Limitor Gain [unit:dB] [tooltip: Current gain of
    the compressor in dB]",-50,+10));

    displaygain = _,_ <: _,_,(abs,abs:+) : _,_,gainview : _,attach;

    limitor_stereo =
    displaygain(co.compressor_stereo(4,-6,0.008,0.5)) :
    *(makeupgain), *(makeupgain);

    ctl_group(x) = knob_group(hgroup("[3] Compression Control", x));

    

    makeupgain = comp_group(hslider("[5] Makeup Gain [unit:dB]
    [tooltip: The compressed-signal output level is increased by this amount
    (in dB) to make up for the level lost due to compression]",
    40, -96, 96, 0.1)) : ba.db2linear;
};


process = _<:limitor_stereo:>_;
