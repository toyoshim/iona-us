---
layout: default
title: mod
permalink: mod_en
---
# MOD
---
## If namco's system boards don't recognize IONA
This electronical characteristics issue wasn't resolved by firmware update.
You need to apply one of following two mods for JVS bus.

But, after this MOD, the device would not be able to coonect PC for firmware updates in usual ways.

1. Connect DATA- and GND over 0.1uF capacitor

2. Connect DATA+ and GND over 100Ω register

PCB's layout from the bottom side:

![pcb](mod_pcb.png)

Following picture adopt 2., as inserting a surface mounted register between pins in the yellow circle.

![mod example](mod_namco.jpg)

After this MOD, you will need USB Type A male to Type A male cable for firmware updates.

All you have to do is wire 1P USB connector to PC's USB port.
You should not connect micro USB for power supply because 1P port can do. If you connect micro USB, device or PC may be broken, or burned.
You also don't need to shorten pins for firmware updates.