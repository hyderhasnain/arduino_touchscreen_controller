**Arduino Touchscreen LCD Wireless Controller Project** 
=========================================================

##Background

I designed and built this project during summer 2013, as an upgrade to a previous wireless controller I built the previous summer to control a small robot. That controller used an LED ring to display orientation and driving direction, communicating via XBee, but I needed a wireless controller with more functionality and input options.
So I found this LCD via Adafruit (). Perfect!

  >Turns out a newer version came out soon afterwards, that uses SPI instead of a parallel connection. Much fewer pins, easier to use, etc. Bummer, *lots* of my design decisions and compromises revolved around the limited IO on the Arduino... Oh well. 

I wanted the board to have many features, including: accelerometer control, wireless communication, wireless programming, real time clock, onboard battery charging, joysticks, microSD card, light sensor, advanced math computation, etc. Of course, this meant I had to design a custom PCB.

##Design

During this process, I made a number of Eagle library parts for many components, including the Adafruit LCD, microSD card breakout, ADXL345 accelerometer, MCP3008, uM-FPU chip, and Arduino Fio by transplanting designs from datasheets and PCB designs. I've included my library because I personally couldn't find these parts online, and maybe these will help somebody. I planned to design the entire schematic/board in Cadsoft Eagle, but after completing the schematic realized that the free version of Eagle limits the maximum size of the board, and my board wouldn't fit. After much frustration and fiddling, I decided to use DipTrace to layout the PCB. I exported my schematic and un-routed board from Eagle into DipTrace, and after managed to get all the parts showing up properly. The only huge problem was that I wouldn't be able to modify my circuit because the schematic was in Eagle and the PCB was in DipTrace. My first "real" PCB I designed, pretty good job I guess. Manufactured by OSHPark, great quality and price. 

##Code

###Graphing Program

Adafruit wrote awesome driver and graphics libraries for their LCDs, which you can find on the LCD product pages.
I wrote many programs for this board, the highlight of which is currently an accelerometer graphing program. Unfortunately, there isn't enough flash memory to use both the SD library and TouchScreen library. So you can't both log data *and* have on-screen controls. The program reads accelerometer data from the ADXL345, and displays it on the LCD via an interactive, color-coded interface. 
The graphing code works by storing two variables for each accelerometer axis(XYZ), a "old" value and "new" value. A line is drawn between the values, which are updated each time data is read. 
The code is heavily commented.
The code also contains snippets to enable drawing circles. Lots of other stuff is commented out, for example if you want to enable SD card logging, comment out the touchscreen code and un-comment the SD card code. Read Adafruit's writeup of their graphics library! :)

There are *two* versions of the code, one without any input from the MCP3008 (joysticks, voltage sensor, light sensor) and one with. The one without is faster.

This code probably contains lots of stuff that I need specifically for this board, but you can probably figure it out.

