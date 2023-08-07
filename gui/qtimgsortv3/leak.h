#if defined(_WIN32)	// note also defined on win64... 

#if defined(_DEBUG)

#if !defined(_LEAK_H)
	#define _LEAK_H

	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
	#define IS_DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)

	#define new IS_DEBUG_NEW

#endif

#else
	#define DEBUG_NEW
#endif

#endif