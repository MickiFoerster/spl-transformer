#include<assert.h>

extern "C" void 
dummy(const char* format, ...)
{
	/* This function serves only as a dummy to prevent compiler optimization. */
	assert(format);
}
