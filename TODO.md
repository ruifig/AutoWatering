* Go through the console commands list and cleanup/remove what's should not be there, like test stuff I left behind.

* Add watchdog timer component

* Add date/time reading from https://learn.adafruit.com/adafruit-magtag/getting-the-date-time

* GroupMonitor should make use of a mutex style thing like the sensors, to keep maximum current usage low. - DONE
	* Make sure the mutex is released when turning off the motor - DONE

* Instead of the threshold little marker, use a grey horizontal line across the entire graph - DONE
	* Need to add a SensorTresholdChangedEvent - DONE
	* Graph then can react to the event, and trigger a full redraw of the plot history - DONE


* SettingsMenu
	* Calibration sub-menu - DONE
		* Have the labels reflect the current sensor state - DONE
			* air value needs to be updated - DONE
			* water value needs to be updated - DONE
			* current sensor reading shown in the middle as % - DONE
		* Have the "set threshold" button react and apply the calibration settings - DONE
	* Implement m_configIsDirty - DONE
		* Saving button needs to update - DONE
		* Have the saving button perform a save and close - DONE

* Save the group config when we start or stop, so it remembers that when rebooting, otherwise there is no way to start/stop a group and get that saved - DONE
* When a group is stopped, and we go into the calibration menu, the calibration doesn't work. Probably we are not getting any events. - DONE

* Add a confirmation sub-menu for the user initiated shot - DONE
	* It's probably enough to show under the shot button, a label with "SURE?", and a button with the check mark, and also show the close button like in the settings menu. - DONE
* Add blue marker (along side the error red dot), to mark when the motor is on
	* This helps knowing when the board is trying telling the motor to turn on, and for some reason it is NOT turning on


* GroupMonitor:
	* DONE - Need to put the active slot "release" somewhere. As-in, every time we call tryAcquire, we need to remember that and make sure "release" is called when we are no longer trying to acquire (or the motor is turned off)
	* DONE - Manual shots (and the console command shots) need to go through the GroupMonitor::tick function, so it respects the allowed number of active motors
		* Probably need another field (e.g: bool m_doExplicitShot) in GroupMonitor, and then in the tick function where we check the last sensor reading, we also check that bool


CLEAN UP
========

* Delete lib\Adafruit_DHT-sensor-library if not used
* Remove the "setmuxenabled" and "setmuxchannel"


BUGS
====

* Motor will turn on before time. Repro steps:
	1. Set a sensor interval to say 2minutes and save.
	2. Open the settings for the group again, and open the calibration menu
	3. Squeeze the sensor so it goes over the threshold.
	4. Close the settings without saving.
	5. The motor will turn on as soon as we close the settings menu.
* The history plot line draws red dots inconsistently 
	* Repro: Start and stop a group while getting stable readings of say 30%
	* When restarting a group, and the plot starts moving, spots with red dots will move to the left as expected, but without properly erasing the ones on the right, causing a read line to incorrectly grow in the plot
