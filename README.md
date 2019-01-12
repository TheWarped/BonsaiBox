# BonsaiBox
Smart solutions for biological systems.

  BonsaiBox v1 - "Enoki" 
  by Traverse Jurcisin

  BonsaiBox is an Arduino controlled growth chamber.
  The chamber is intended for the growth, maintenance, 
  and study of organsims like plants or fungi.
  The "Enoki" is fitted with hardware and software
  that is more catered to tropical species and fungi.
  
    Components of the BonsaiBox are:
    * 1x Sonic fogger.
    * 1x Air pump.
    * 1x AC wall plug cable.
    * 1x 12 VDC PC fan (with a built in 5 VDC ws2812 "NeoPixel" LED's).  
    * 1x 1k resistor.
    * 1x irlb8721 MOSFET.
    * 1+ capacitor (for current smoothing, PWM controlling a motor with "dirty" current makes for a noisey MOSFET).
    * 1x Liquid Crystal Display (20x4).
    * 1x Relay (2 channel).  One channel for the mister, one for the air pump.
    * 1x RTC.  For clock and timer related functions.
    * 1x BMP 280.  For pressure, temperature, and humidity. 3.3v!
    * 1x 120 VAC -> 24 VDC transformer.  Make sure it can produce enough amps!
    * 1x 12 VDC buck converter.  Make sure it can produce enough amps!
    * 1x 5 VDC buck converter.  Make sure it can produce enough amps!

    Tips/ notes: 
    * Shield the sonic fogger from debree, make sure there's only a few cm of water above it, but give the fog a clear path to spread           through the enclosure.
    * The fan speed CFM program is rough.  Make sure to check your fans CFM and enter it in, mine was rated for 30 CFM.
    * NeoPixels are sensitive, I burned a few out while getting to know them better, be careful!

    To code:
    * Scale light intensity with temperature.
    * If RH is ok, but it's too hot, increase fan speed more.
    * Scale fan with VPD.
