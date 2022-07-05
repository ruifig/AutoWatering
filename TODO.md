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


CLEAN UP
========

* Delete lib\Adafruit_DHT-sensor-library if not used
* Remove the "setmuxenabled" and "setmuxchannel"


BUGS
====

* The history plot line draws red dots inconsistently 
	* Repro: Start and stop a group while getting stable readings of say 30%
	* When restarting a group, and the plot starts moving, spots with red dots will move to the left as expected, but without properly erasing the ones on the right, causing a read line to incorrectly grow in the plot
