#pragma once

#include <string.h>
#include <Arduino.h>


//
// Taken from https://arduino.stackexchange.com/questions/65351/how-to-find-out-the-maximum-used-stack-space
//
static inline size_t stack_size()
{
    return RAMEND - SP;
}


