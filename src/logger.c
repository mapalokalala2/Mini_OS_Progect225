#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "../include/logger.h"

static const char* LOG_FILE = "OS_Logs.txt";
static FILE *log_fp = NULL;//log_fp means log file pointer. just incase i may forget what fp stands for in the future.

void log_init(void) {
    log_fp = fopen(LOG_FILE, "a");

    if( log_fp == NULL){
        perror("ERROR: COULD NOT INITIALIZE THE LOGS");
        return;
    }

    fprintf(log_fp, "==========Logger initalized==========\n");
    fflush(log_fp);
}

void log_event(const char *log_msg, ...) { // note to self: the .... thells the function that is will take a format sting followed by any other numberof extra variables. 
     if(log_fp == NULL) {//safty check. 
        return;
    }
    // Getting the currect time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[26];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H : %M : %S", t);

//Opening the file

        fprintf(log_fp, "[%s]" , time_str); 
        va_list args;// holds the list of extra variables
        va_start(args, log_msg);// starts the process if reading extra variables
        vfprintf(log_fp ,log_msg, args); // this is a version of printf designed to work with va_list
        fprintf(log_fp, "\n");
        fflush(log_fp);//keeps the data safe without needing to close the file. it forces the log event to move from ram to hard drive
        va_end(args);//cleans up the memory used to track the arguments 



}

void show_log(void) {
    char line[512];
    FILE *read_log = fopen(LOG_FILE, "r");

    if( !read_log){
        printf("=====No Logs Available======\n");
        return;
    }

    printf("============================\n");
    printf("    Dispalying logs\n");
    printf("============================\n");

    while(fgets(line, sizeof(line), read_log)){
        printf("%s", line);
    }
    printf("============================\n");
    fclose(read_log);
    
}

void log_dump(void){
    if(log_fp != NULL){
        fprintf(log_fp, "=====LOGGER SHUTTING DOWN=======\n\n");
        fclose(log_fp);
        log_fp = NULL;
    }

}