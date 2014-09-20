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
    #define MICRO_LOG_ACTIVE
#endif

// Minimum log levels for specific code areas:

#ifndef MICRO_LOG_MIN_LEVEL
    #define MICRO_LOG_MIN_LEVEL nolog
#endif

namespace uLog {
    static const int
        logGPSolver = detail,
        logQSExperiment = info,
        logQSSolverForCompleteCarModel = detail,
        logRootNewton = info,
        logInfo = nolog;
}

#ifndef MICRO_LOG_DETAIL
    #define MICRO_LOG_DETAIL 2
#endif

#ifndef MICRO_LOG_FILE_NAME
    //#define MICRO_LOG_FILE_NAME "default.log"
#endif

/// Specify one threading library to be used
#ifndef MICRO_LOG_THREADING
	#define MICRO_LOG_THREADING MICRO_LOG_SINGLE_THREAD
	//#define MICRO_LOG_THREADING MICRO_LOG_CPP11_THREAD
	//#define MICRO_LOG_THREADING MICRO_LOG_BOOST_THREAD
	//#define MICRO_LOG_THREADING MICRO_LOG_PTHREAD
#endif

/// Set this if the logger is in a DLL (to avoid static variables)
#ifdef WIN32
    #define MICRO_LOG_DLL
#endif


#endif	// MICRO_LOG_CONFIG_HPP
