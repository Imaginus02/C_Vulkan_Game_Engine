#pragma once

#include "defines.h"

#define KASSERTIONS_ENABLED

#ifdef KASSERTIONS_ENABLED
#if _MSC_VER
#include <intrin.h>
#define debugBreak() __debugbreak()
#else
#define debugBreak() __builtin_trap()
#endif

KAPI void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line);

#define KASSERT(expression) 													\
	{																			\
		if (expression) { 													\
		} else { 																\
			report_assertion_failure(#expression, "", __FILE__, __LINE__); \
			debugBreak(); 														\
		} 																		\
	}

#define KASSERT_MSG(expression, message) 										\
	{																			\
		if (expression) {													\
		} else {  																\
			report_assertion_failure(#expression, message, __FILE__, __LINE__); \
			debugBreak(); 														\
		} 																		\
	}

#ifdef _DEBUG
#define KASSERT_DEBUG(expression) 												\
	{																			\
		if (expression) { 													\
		} else { 																\
			report_assertion_failure(#expression, "", __FILE__, __LINE__); \
			debugBreak(); 														\
		} 																		\
	}
#else
#define KASSERT_DEBUG(expr)
#endif

#else
#define KASSERT(expression)
#define KASSERT_MSG(expression, message)
#define KASSERT_DEBUG(expression)
#endif