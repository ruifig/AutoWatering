* GroupMonitor should make use of a mutex style thing like the sensors, to keep maximum current usage low. - DONE
	* Make sure the mutex is released when turning off the motor - DONE

* Instead of the threshold little marker, use a grey horizontal line across the entire graph - DONE
	* Need to add a SensorTresholdChangedEvent - DONE
	* Graph then can react to the event, and trigger a full redraw of the plot history - DONE

