#include "MemorySetup.h"
#include "crazygaze/micromuc/Logging.h"
#include "memdebug/memdebug.h"
#include "stackmon/stackmon.h"

//////////////////////////////////////////////////////////////////////////
// Make sure the compiler is using 1-byte alignment to save space
//////////////////////////////////////////////////////////////////////////
class AlignmentCheck
{
	char* a;
	uint8_t b;
	char* c;
	uint8_t d;
};
static_assert(sizeof(AlignmentCheck) == 2*2 + 1*2, "Default struct alignment is not 1");

//
// #RVF : is this needed ?
void operator delete(void* ptr, unsigned int size)
{
	free(ptr);
}

//
// Using uint8_t instead of unsigned int, so I can do pointer arithmetic to get the result in bytes
//
extern uint8_t __data_start;
extern uint8_t __data_end;
extern uint8_t __bss_start;
extern uint8_t __bss_end;
extern uint8_t __heap_start;
extern uint8_t __heap_end;
extern char* __malloc_heap_start;
extern char* __malloc_heap_end;
extern void *__brkval;

//////////////////////////////////////////////////////////////////////////
/*
Setups heap and stack things
- For the heap, it setups the top end limit
- For the stack it setups the canary, that allows stack usage detection
*/
void setupMemoryAreas(uint16_t heapSize)
{
	__malloc_heap_end = __malloc_heap_start + heapSize;

	unsigned int dataSize = &__data_end - &__data_start;
	unsigned int bssSize = &__bss_end - &__bss_start;

	CZ_LOG(logDefault, Log, F("__data_start=%u, __data_end=%u, size=%u"), &__data_start, &__data_end, &__data_end - &__data_start);
	CZ_LOG(logDefault, Log, F("__bss_start=%u, __bss_end=%u, size=%u"), &__bss_start, &__bss_end, &__bss_end - &__bss_start);
	CZ_LOG(logDefault, Log, F("__malloc_heap_start=%u, __malloc_heap_end=%u, size=%u"), __malloc_heap_start, __malloc_heap_end, heapSize);
	logMemory();
}

void logMemory()
{
	static uint16_t previous[4];
	static uint16_t stats[4];
	stats[0] = getFreeMemory();
	stats[1] = getMemoryUsed();
	calcStack(&stats[2], &stats[3]);

	if (memcmp(previous, stats, sizeof(stats)) != 0)
	{
		CZ_LOG(logDefault, Log, F("Heap (free=%u, used=%u), Stack (free=%u, used=%u)"), stats[0], stats[1], stats[2], stats[3]);
		memcpy(previous, stats, sizeof(stats));
	}

}

