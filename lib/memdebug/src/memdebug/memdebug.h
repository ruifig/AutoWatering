// From https://andybrown.me.uk/2011/01/01/debugging-avr-dynamic-memory-allocation/

/*
 * memdebug.h
 *
 *  Created on: 15 Dec 2010
 *      Author: Andy Brown
 *
 *  Use without attribution is permitted provided that this 
 *  header remains intact and that these terms and conditions
 *  are followed:
 *
 *  http://andybrown.me.uk/ws/terms-and-conditions
 */
 
#ifdef __cplusplus
extern "C" {
#endif
 
#include <stddef.h>
 
size_t getMemoryUsed();
size_t getFreeMemory();
size_t getLargestAvailableMemoryBlock();
size_t getLargestBlockInFreeList();
int getNumberOfBlocksInFreeList();
size_t getFreeListSize();
size_t getLargestNonFreeListBlock();
 
 
#ifdef __cplusplus
}
#endif

