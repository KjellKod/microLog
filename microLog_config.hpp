/* 
 * File:   microLog_config.hpp
 *
 * Configuration of the microLog logger.
 * Use this file to set the parameters related to the logger.
 * These macros can be defined in the makefile/build system.
 * 
 * Author: Pietro Mele
 */

#ifndef MICRO_LOG_CONFIG_HPP
#define	MICRO_LOG_CONFIG_HPP

#ifndef MICRO_LOG_ACTIVE
#define MICRO_LOG_ACTIVE 1
#endif

#ifndef MICRO_LOG_MIN_LEVEL
#define MICRO_LOG_MIN_LEVEL 0
#endif

#ifndef MICRO_LOG_DETAIL
#define MICRO_LOG_DETAIL 3
#endif

#ifndef MICRO_LOG_FILE_NAME
#define MICRO_LOG_FILE_NAME "default.log"
#endif

// Custom threading libraries (id > 4)
#define MICRO_LOG_MRF1_THREAD    5

/// Specify one threading library to be used
#ifndef MICRO_LOG_THREADING
	#define MICRO_LOG_THREADING MICRO_LOG_SINGLE_THREAD
	//#define MICRO_LOG_THREADING MICRO_LOG_CPP11_THREAD
	//#define MICRO_LOG_THREADING MICRO_LOG_BOOST_THREAD
	//#define MICRO_LOG_THREADING MICRO_LOG_PTHREAD
 	//#define MICRO_LOG_THREADING MICRO_LOG_MRF1_THREAD
#endif

// Custom threading libraries' definitions

#if(MICRO_LOG_THREADING == MICRO_LOG_MRF1_THREAD)
	#define MICRO_LOG_CRITICAL_SECTION \
		DFS::Infrastructure::CCriticalSection logCS; \
		DFS::Infrastructure::CCriticalSectionLock L(logCS);
	#define MICRO_LOG_CRITICAL_SECTION_END 
#endif


#endif	// MICRO_LOG_CONFIG_HPP
