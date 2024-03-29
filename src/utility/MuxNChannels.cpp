#include "MuxNChannels.h"

namespace cz
{

namespace detail
{
	// Table for a 16 channels mux
	// For the 8 channel mux, we only use the first 8 lines and first 3 columns
	const uint8_t muxChannel[16][4] = {
		{0, 0, 0, 0},  // channel 0
		{1, 0, 0, 0},  // channel 1
		{0, 1, 0, 0},  // channel 2
		{1, 1, 0, 0},  // channel 3
		{0, 0, 1, 0},  // channel 4
		{1, 0, 1, 0},  // channel 5
		{0, 1, 1, 0},  // channel 6
		{1, 1, 1, 0},  // channel 7
		{0, 0, 0, 1},  // channel 8
		{1, 0, 0, 1},  // channel 9
		{0, 1, 0, 1},  // channel 10
		{1, 1, 0, 1},  // channel 11
		{0, 0, 1, 1},  // channel 12
		{1, 0, 1, 1},  // channel 13
		{0, 1, 1, 1},  // channel 14
		{1, 1, 1, 1}   // channel 15
	};
}

} // namespace cz
