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

#include "Secrets.h"

#include "PostConfig.h"
