#pragma once

/**
 * How many sensors to support
 */
#define NUM_SENSORS 4

/**
 * Default sensor sampling interval in seconds
 */
#define DEFAULT_SENSOR_SAMPLING_INTERVAL 5

/**
 * Time in seconds for the LCD's backlight to be turned if no buttons are pressed
 */

#define DEFAULT_LCD_BACKLIGHT_TIMEOUT 10 

/**
 * Time in seconds for the LCD to turn off if no buttons are pressed
 */
#define DEFAULT_LCD_IDLE_TIMEOUT 20

/**
 * Time in seconds for the progam to revert to normal operating mode if no buttons are press
 * This is so that if for example the user leaves the system in the menu, after a while it will revert to running normally
 */
#define DEFAULT_REVERT_TO_NORMAL_TIMEOUT 60


/**
 * How long to show the intro for when powering up
 */
#define INTRO_DURATION 5
