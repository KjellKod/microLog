# microLog

### Single header logger.
### Two log level thresholds, to tune verbosity on a global and local level.

A minimalistic logging utility.

    - A single file project (this one).
	- There are no implementation files, so makefiles do not have to be touched.
    - Thread safe (C++11 threads, Boost threads or pthread).
	- It can be used when no debugger nor other logging system is available.
	- No objects to be defined.
	- To activate it, #define MICRO_LOG_ACTIVE.
	- When not activated, or when a log message level is below the threshold, there is no generated binary code.
    - uLOG(level) only logs if level >= MICRO_LOG_MIN_LEVEL and level >= ULog::minLogLevel.
    - uLOG(level, localLevel) logs if level >= MICRO_LOG_MIN_LEVEL and (level >= ULog::minLogLevel or level >= localLevel).
    - To flush the message use the uLOGE manipulator (stands for log-end) instead of std::endl!
	- The output log file can be:
		- Unique for this executable, with a fixed preprocessor defined file name (MICRO_LOG_FILE_NAME).
		- Unique for this executable, with a global static variable file stream (microLog_ofs).
		- Custom, with a stream passed as a parameter to every log message.
	- Multithreading: the MICRO_LOG_LOCK/MICRO_LOG_UNLOCK macros delimit a critical section.
		- Their values depend on the adopted threading library, and can be defined in microLog_config.hpp.
		- Predefined values available for: C++11 threads, Boost threads, pthread.
	- For better performance, consider logging to a ramdisk.

	Usage: See microLog_test.cpp as an example.
