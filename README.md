# microLog

#### - Single header logger.
#### - Two log level thresholds, to tune verbosity on a global and local level.

Distinctive features:

- ...

Other features:

- A single file project.
- There are no implementation files, so makefiles do not have to be touched.
- Thread safe (C++11 threads, Boost threads or pthread).
- It can be used when no debugger nor other logging system is available.
- No objects to be defined.
- To activate it, #define MICRO_LOG_ACTIVE.
- When not activated, or when a log message level is below the threshold, there is no generated binary code.
- uLOG(level) only logs if level >= MICRO_LOG_MIN_LEVEL and level >= ULog::minLogLevel.
- uLOG(level, localLevel) logs if level >= MICRO_LOG_MIN_LEVEL and (level >= ULog::minLogLevel or level >= localLevel).

For better performance, consider logging to a ramdisk.

Usage: See microLog_test.cpp as an example.
