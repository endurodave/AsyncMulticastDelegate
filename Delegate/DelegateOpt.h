#ifndef _DELEGATE_OPT_H
#define _DELEGATE_OPT_H

// Define either USE_WIN32_THREADS or USE_STD_THREADS to specify WIN32 or std::thread threading model.
// Define USE_CPLUSPLUS_11 if the compiler supports C++ 11 features.
// Define USE_XALLOCATOR to use fixed block memory allocation.

// Define USE_CPLUSPLUS_11 if using a C++11 compliant compiler. Using Visual Studio, if the _MSC_VER 
// is >= 1700 then C++11 features are available. 
#if _MSC_VER >= 1700
	#define USE_CPLUSPLUS_11 1
	#define USE_STD_THREADS	1
	//#define USE_WIN32_THREADS 1 
#elif _MSC_VER >= 1500
	#define USE_WIN32_THREADS 1
#endif

#if __GNUC__ >= 5
	#define USE_CPLUSPLUS_11 1
	#define USE_STD_THREADS	1
#elif __GNUC__ == 4 
	#define USE_STD_THREADS	1
#endif

#if defined(USE_STD_THREADS) && defined(USE_WIN32_THREADS)
	#error Define only one. Either USE_STD_THREADS or USE_WIN32_THREADS
#endif

#if !defined(USE_STD_THREADS) && !defined(USE_WIN32_THREADS)
	#error Must define either USE_STD_THREADS or USE_WIN32_THREADS
#endif

#if defined(__GNUC__) && defined(USE_WIN32_THREADS)
	#error GCC does not support WIN32 API. Define USE_STD_THREADS.
#endif

#if defined(USE_STD_THREADS) && !defined(USE_CPLUSPLUS_11)
	#error C++11 compiler required to use std::thread API
#endif

// To make the delegate library use a fixed block memory allocator uncomment the include
// line below and the XALLOCATOR line. This could speed new/delete operations and eliminates
// the possibility of a heap fragmentation fault. Use is completely optional. 
// @see https://github.com/endurodave/xallocator
//#define USE_XALLOCATOR 1

#endif
