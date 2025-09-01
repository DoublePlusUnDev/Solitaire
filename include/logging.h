#ifndef LOGGING_H
#define LOGGING_H

#define LOG_NAME "solitaire-log.txt"
#define ENABLE_LOG true
#define CRASH_LOGGING true

extern FILE* logfile;
extern bool successfulOpen;

//writes a string into the log file
#define LOG(message) \
if (successfulOpen)\
{\
    fprintf(logfile, message);\
    if (CRASH_LOGGING)\
        UpdateLog();\
}\

//writes a formatted string into the log file, requires arguments!
#define LOG_ARGS(message, ...) \
if (successfulOpen)\
{\
    fprintf(logfile, message, __VA_ARGS__);\
    if (CRASH_LOGGING)\
        UpdateLog();\
}\

//tries to open the log file, if unable to do so will disable logging
void StartLog();

void UpdateLog();

//closes the log file, if it had been opened before
void CloseLog();

#endif