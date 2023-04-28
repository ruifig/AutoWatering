#pragma once

// Dummy so other headers can detect if Config.h was include first
#define CONFIG_H

#include <Arduino.h>
#include "crazygaze/micromuc/czmicromuc.h"
#include "utility/PinTypes.h"


#include "PreConfig.h"


/**
 * How many bits to set the ADC readings to.
 * E.g: The RP2040 has a 12-bit ADC
 */
#define ADC_NUM_BITS 12

/**
 * Note. Internally it adds 0x20, which is the base address
 */
#define IO_EXPANDER_ADDR 0x0

/**
 * What arduino pin we are using to communicate with the multiplexer.
 * Also known as the multiplexer Z pin
 * This needs to be an analog capable pin (to use with analogRead(...)), so we can do sensor readings
 */
#define MCU_TO_MUX_ZPIN cz::MCUPin(28)



/**
 * Pins of the IO expander to use to set the s0..s2 pins of the mux
 */
#define IO_EXPANDER_TO_MUX_S0 cz::IOExpanderPin(0+0)
#define IO_EXPANDER_TO_MUX_S1 cz::IOExpanderPin(0+1)
#define IO_EXPANDER_TO_MUX_S2 cz::IOExpanderPin(0+2)

/**
 * Pins of the IO Expander used to turn on/off the motors
 */
#define IO_EXPANDER_MOTOR0 cz::IOExpanderPin(0+3)
#define IO_EXPANDER_MOTOR1 cz::IOExpanderPin(0+4)
#define IO_EXPANDER_MOTOR2 cz::IOExpanderPin(0+5)
#define IO_EXPANDER_MOTOR3 cz::IOExpanderPin(0+6)
#define IO_EXPANDER_MOTOR4 cz::IOExpanderPin(0+7)
#define IO_EXPANDER_MOTOR5 cz::IOExpanderPin(8+6)

/**
 * Pins of the IO Expander used to power the capacitive soil moisture sensors
 */
#define IO_EXPANDER_VPIN_SENSOR0 cz::IOExpanderPin(8+5)
#define IO_EXPANDER_VPIN_SENSOR1 cz::IOExpanderPin(8+4)
#define IO_EXPANDER_VPIN_SENSOR2 cz::IOExpanderPin(8+3)
#define IO_EXPANDER_VPIN_SENSOR3 cz::IOExpanderPin(8+2)
#define IO_EXPANDER_VPIN_SENSOR4 cz::IOExpanderPin(8+1)
#define IO_EXPANDER_VPIN_SENSOR5 cz::IOExpanderPin(8+0)

/*
 * Pin from the IO Expander to the Mux's E pin.
 * This allows us to turn off the Mux, so multiple sensor boards can share a single sensor reading pin.
 * LOW - Mux enabled
 * HIGH - Mux disabled
 */
#define IO_EXPANDER_TO_MUX_ENABLE cz::IOExpanderPin(8+7)

/**
 * Multiplexer pins used to read the soil moisture sensors
 */
#define MUX_MOISTURE_SENSOR0 cz::MultiplexerPin(7) // Y0
#define MUX_MOISTURE_SENSOR1 cz::MultiplexerPin(5) // ...
#define MUX_MOISTURE_SENSOR2 cz::MultiplexerPin(3)
#define MUX_MOISTURE_SENSOR3 cz::MultiplexerPin(0)
#define MUX_MOISTURE_SENSOR4 cz::MultiplexerPin(1)
#define MUX_MOISTURE_SENSOR5 cz::MultiplexerPin(2)

#define MAX_NUM_I2C_BOARDS 2

/**
 * How many sensor/motor pairs to support
 * Each i2c board has 6 pairs
 */
#define MAX_NUM_PAIRS (6*MAX_NUM_I2C_BOARDS)

/**
 * How many sensor/motor pairs fit on the screen
 */
#define VISIBLE_NUM_PAIRS 4

/**
 * Time in seconds for the program to revert to normal operating mode if no buttons are press
 * This is so that if for example the user leaves the system in the menu, after a while it will revert to running normally
 */
// #TODO : This is not being used yet
#define DEFAULT_REVERT_TO_NORMAL_TIMEOUT 60.0f

/**
 * How long to show the intro for when powering up
 */
#if AW_FASTER_ITERATION
	#define INTRO_DURATION 0.25f
#else
	#define INTRO_DURATION 1.0f
#endif


/**
 * How long the boot menu should stay up before defaulting to "Load"
 */
#define BOOTMENU_COUNTDOWN 2.0f


/**
 * Default screen: 0..100.
 * Actual screen classes scale this accordingly to whatever range they use 
 */
#define SCREEN_DEFAULT_BRIGHTNESS 100

/**
 * How long to wait (in seconds) until turning off the screen backlight, if there are no touch events detected
 * If 0, screen timeout is considered disabled (as-in, screen is always on)
 */
#define SCREEN_OFF_TIMEOUT 30

/**
 * How fast to dim the screen to 0.
 * This is in percentiles per second. E.g: a value of 10 means the brightness will go down at 10% per second
 */
#define SCREEN_OFF_DIM_SPEED 10

#define SCREEN_BKG_COLOUR Colour_Black

#define INTRO_TEXT_COLOUR Colour_Green

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Width in pixels for the group number label (shown left of the group history plot)
#define GROUP_NUM_WIDTH 16
#define GRAPH_NUMPOINTS (SCREEN_WIDTH-GROUP_NUM_WIDTH-32-2)
#define GRAPH_VALUES_TEXT_COLOUR Colour_LightGrey
#define GRAPH_VALUES_BKG_COLOUR Colour_Black
#define GRAPH_BORDER_COLOUR Colour_LightGrey
#define GRAPH_SELECTED_BORDER_COLOUR Colour_Green
#define GRAPH_NOTSELECTED_BORDER_COLOUR Colour_Black
#define GRAPH_BKG_COLOUR Colour_Black
#define GRAPH_NOTRUNNING_TEXT_COLOUR Colour_Red
#define GRAPH_MOTOR_ON_COLOUR Colour_Yellow
#define GRAPH_MOTOR_OFF_COLOUR Colour_Black
#define GRAPH_MOISTURELEVEL_COLOUR Colour_Cyan
#define GRAPH_MOISTURELEVEL_ERROR_COLOUR Colour_Red
#define GRAPH_THRESHOLDMARKER_COLOUR Colour(0x52ab)
#define TEMPERATURE_LABEL_TEXT_COLOUR Colour_Yellow
#define HUMIDITY_LABEL_TEXT_COLOUR Colour_Cyan
#define RUNNINGTIME_LABEL_TEXT_COLOUR Colour_Yellow

#define BATTERYLEVEL_LABEL_TEXT_COLOUR Colour_White

//
// * Top line is used for the motor on/off info
// * Rest of the lines are for the moisture level
#define GRAPH_HEIGHT (1+32)
#define GRAPH_POINT_NUM_BITS 5
static_assert(1<<GRAPH_POINT_NUM_BITS < GRAPH_HEIGHT, "Reduce number of bits, or increase graph height");
#define GRAPH_POINT_MAXVAL ((1<<GRAPH_POINT_NUM_BITS) - 1)

#include "Secrets.h"

#include "PostConfig.h"
