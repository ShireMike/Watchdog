#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <gpiod.h>
#include <syslog.h>
//
#define CONSUMER    "Router Power Cycle"
//
#include <webcheck.h>
//
int config(struct configparms * conf) ;
int email(struct configparms * conf) ;
int logmsg(char* message);
int ping(struct configparms * conf) ;
//
// Volatile flag to ensure the compiler doesn't optimize it away in the loop
volatile sig_atomic_t keep_running = 1;
volatile sig_atomic_t reload_requested = 0;
struct configparms conf , *conptr;
//
// Signal handler callback
void handle_signal(int signal) {
    switch (signal) {
        case SIGTERM:
            logmsg("received SIGTERM. Cleaning up and shutting down");
            keep_running = 0;
            break;
        case SIGHUP:
            reload_requested = 1;
            break;
        default:
            break;
    }
}
//
int main(int argc, char* argv[]) {
    conf.itest = 0;
    if( argc == 2 ) {
        if (!strcmp(argv[1],"test")) {
            conf.itest = 1 ;
        }
    }
    struct sigaction act;
    // Set up the signal handler
    act.sa_handler = handle_signal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    // Register handlers for both shutdown (SIGTERM) and reload (SIGHUP)
    if (sigaction(SIGTERM, &act, NULL) < 0) {
        perror("Error registering SIGTERM handler");
        return EXIT_FAILURE;
    }
    if (sigaction(SIGHUP, &act, NULL) < 0) {
        perror("Error registering SIGHUP handler");
        return EXIT_FAILURE;
    } 
    //
    conptr = &conf;
    if (config(conptr)) { // get email configuration
        return 1  ;
    }    
    char message[200] ;
    sprintf(message,"----- Watchdog v4.0 compiled on: %s %s -----", __DATE__, __TIME__);
    logmsg(message);    
    //
    int chipnumber = 2;
    unsigned int line_num = 3;  // GPIO Pin P8_8
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;
    //
    chip = gpiod_chip_open_by_number(chipnumber);
    if (!chip) {
        logmsg("Open gpio chip failed");
        return 1;
    }
    //
    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        logmsg("Get gpio line failed");
        return 1 ;
    }
    //
    ret = gpiod_line_request_output(line, CONSUMER, 0);
    if (ret < 0) {
        logmsg("Request gpio line as output failed");
        return 1;
    }
    //
    ret = gpiod_line_set_value(line, 1);  // 1 = relax relay
    //
    int delay = 1 ;
    while ( keep_running ) {
        if (reload_requested) {
            logmsg("received SIGHUP - reloading configuration file");
            if (config(conptr)) { // get email configuration
                return 1  ;
            }
            reload_requested = 0;
        }
        //****************************
        //
        
        delay = 300 ;
        //
        // End of delay
        //****************************
        time_t rawtime;
        time (&rawtime);
        if (ping(conptr)) {
            //   ping failed
            char *pingfaillog = "/var/gco/pingfail" ;
            FILE *faillog= NULL;
            faillog = fopen (pingfaillog, "a");
            fprintf(faillog,"%ld \n", rawtime) ;
            fclose(faillog); 
            //          
            if (conf.webstatus) {
                // internet was down previously
                if (((conf.failtime + conf.delay) < rawtime)   // 
                    && (conf.resettime  < conf.failtime)
                    && ((conf.resettime + conf.interval) < rawtime)) {   // not repeat routrt reset before "interval" secounds
                    /// no internet for n minutes - power cycle router 
                    ret = gpiod_line_set_value(line, 0);  // 0 = energise relay, power router off.                   
                    sleep(30) ;
                    ret = gpiod_line_set_value(line, 1);  // 1 = relax relay , power router on                   
                    logmsg("----- router power cycle initiated -----") ;                     
                    conf.resettime = rawtime ;
                    delay = 300 - 32 ;
                    //
                    char *modemlog = "/var/gco/modemlog" ;
                    FILE *modemreset= NULL;
                    modemreset = fopen (modemlog, "a");
                    fprintf(modemreset,"%ld \n", rawtime) ;
                    fclose(modemreset); 
                }
            } else {
                // internet failed for first time
                conf.failtime = rawtime ;
            }
            conf.webstatus = 1 ;  // 1 = no internet
        } else {   
            // ping was successful
            if ((conf.webstatus) && (conf.resettime > conf.lastemail )) {
                // comms recovered from failure
                if (conf.lastemail < rawtime - 43200)  
                    email(conptr); // send email
                else 
                    logmsg("email notification suppressed (anti flooding)") ;
            }
            conf.webstatus =  0;
            conf.failtime = rawtime ;
        }
		sleep(delay) ;	
    }    
    gpiod_line_release(line);
    gpiod_chip_close(chip);
    logmsg("----- Watchdog v4.0 terminated -----") ;   
    return 0;
}
