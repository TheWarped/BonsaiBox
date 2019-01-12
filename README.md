# BonsaiBox
Smart solutions for biological systems.

  BonsaiBox v1 - "Enoki" 
  by Traverse Jurcisin
  GitHub - TheWarped

  BonsaiBox is an Arduino controlled growth chamber.
  The chamber is intended for the growth, maintenance, 
  and study of organsims like plants or fungi.
  The "Enoki" is fitted with hardware and software
  that is more catered to tropical species and fungi.
  
    Components of the BonsaiBox are:
    * 1x Sonic fogger (24VDC).  There's some cheap ones around that allow for replacing the perishable disks, some also blink red when         there isn't enough water.
    * 1x Air pump.
    * 1x AC wall plug cable.
    * 1x 12 VDC PC fan (with a built in 5 VDC ws2812 "NeoPixel" LED's).  
    * 1x 1k resistor for fan control circuit.
    * 1x irlb8721 MOSFET for fan control circuit.
    * 1+ capacitor (for current smoothing, PWM controlling a motor with "dirty" current makes for a noisey MOSFET).
    * 1x I2C Liquid Crystal Display (20x4).
    * 1x Relay (2 channel).  One channel for the mister, one for the air pump.
    * 1x RTC.  For clock and timer related functions.
    * 1x BMP 280.  For pressure, temperature, and humidity. 3.3v!
    * 1x 120 VAC -> 24 VDC transformer.  Make sure it can produce enough amps!
    * 1x 12 VDC buck converter.  Make sure it can produce enough amps!
    * 1x 5 VDC buck converter.  Make sure it can produce enough amps!

    Tips/ Notes: 
    * Shield the sonic fogger from debree, make sure there's only a few cm of water above it, but give the fog a clear path to spread         through the enclosure.
    * The fan speed CFM program is rough.  Make sure to check your fans CFM and enter it in, mine was rated for 30 CFM.
    * NeoPixels are sensitive, a few burned out during the project, be careful!
    * Below is a rough diagram made in tinkercad of how most of the wiring would go, note the relay load's are not shown, and several       of the componments in the diagram are representative of what was used, but not the actual parts.  The actual parts used have           been notated above.


![screenshot 38](https://user-images.githubusercontent.com/46633771/51078723-0ebad780-1688-11e9-8134-c8e9ff7da137.png)


    To code:
    * Scale light intensity with temperature.
    * If RH is ok, but it's too hot, increase fan speed more.
    * Scale fan with VPD.
