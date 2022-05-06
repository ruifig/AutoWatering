* GroupMonitor should make use of a mutex style thing like the sensors, to keep maximum current usage low. - DONE
	* Make sure the mutex is released when turning off the motor - DONE

* Instead of the threshold little marker, use a grey horizontal line across the entire graph - DONE
	* Need to add a SensorTresholdChangedEvent - DONE
	* Graph then can react to the event, and trigger a full redraw of the plot history - DONE


* SettingsMenu
	* Calibration sub-menu
		* Have the labels reflect the current sensor state
			* air value needs to be updated
			* water value needs to be updated
			* current sensor reading shown in the middle as %
		* Have the "set threshold" button react and apply the calibration settings
	* Implement m_configIsDirty
		* Saving button needs to update
		* Have the saving button perform a save and close

* Save the group config when we start or stop, so it remembers that when rebooting, otherwise there is no way to start/stop a group and get that saved

BUGS
====

* The history plot line draws red dots inconsistently 
	* Repro: Start and stop a group while getting stable readings of say 30%
	* When restarting a group, and the plot starts moving, spots with red dots will move to the left as expected, but without properly erasing the ones on the right, causing a read line to incorrectly grow in the plot

* When a group is stopped, and we go into the calibration menu, the calibration doesn't work. Probably we are not getting any events.