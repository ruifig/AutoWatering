# AutoWatering

Arduino automatic plant watering.

Further documentation will be provided in the future, once I have tested in my own greenhouse and I'm happy with the firmware.

Although this initially started as a Mega2650 project, it has been through a few iterations, and now it's setup for the Raspberry Pi Pico W.

Features summary (some still in early stages):

* Rich touchscreen UI to control sensor intervals, how long motors turn on for, etc.
* Can control a flexible number of sensor/motor pairs. Personally I have it setup for 12 pairs.
* Temperature & Humidity readings
* Wifi enabled This is work in progresss.
	* Only Adafruit IO integration is planned
* Power down to save battery
	* This is a work in progress, and at the moment using a TPL5110 IC, for true power down.

I intend to organize the code and configuration so that certain features can easily be disabled. E.g:

* Remove Wifi use
	* Useful if you intend to control the system through the UI only
* Remove the touchscreen and UI
	* Useful if you intend to control the system through Wifi only
* Remove the temperature/humidity readings
	* You don't really need it, or maybe you already have that info coming from Wifi because you already have another system publishing that info.
* Remove the need for the TPL5510 IC.
	* If you plan to have the system always plugged in

# Useful links

https://thecavepearlproject.org/2020/10/27/hacking-a-capacitive-soil-moisture-sensor-for-frequency-output/
https://www.youtube.com/watch?v=IGP38bz-K48&t=306s

