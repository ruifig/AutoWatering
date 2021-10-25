#pragma once

/*
There is a good explanation of the memory layout at : https://www.nongnu.org/avr-libc/user-manual/malloc.html
*/

#include "crazygaze/micromuc/Logging.h"

/*
Setups heap and stack things
- For the heap, it setups the top end limit
- For the stack it setups the canary, that allows stack usage detection
*/
void setupMemoryAreas(uint16_t heapSize);

/*
Logs a summary of heap and stack usage.
If heap or stack usage hasn't changed since the last call, it will do nothing.
*/
void logMemory();

