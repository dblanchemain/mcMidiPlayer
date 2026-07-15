declare name "crybaby";
declare version "0.0";
declare author "JOS, revised by RM";
declare description "Crybaby demo application";

import("stdfaust.lib");

process = _,_ : dm.crybaby_demo, dm.crybaby_demo: _,_;