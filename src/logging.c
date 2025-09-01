#include<stdio.h>
#include<stdbool.h>

#include "logging.h"

FILE* logfile;
bool successfulOpen = false;

//opens a brand new logging file
void StartLog()
{
    if (!ENABLE_LOG) 
        return;

    logfile = fopen(LOG_NAME, "w");

    successfulOpen = (logfile != NULL);
}

//closes the logfile and reopens it in append mode
//used for updating the log file, so it can be monitored in real time
void UpdateLog()
{
    if (successfulOpen)
    {
        fclose(logfile);\
        logfile = fopen(LOG_NAME, "a");
    }
}

//closes and saves the content of the logging file
void CloseLog()
{
    if (successfulOpen)
        fclose(logfile);
}