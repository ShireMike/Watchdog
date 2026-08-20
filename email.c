#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webcheck.h>
#include <errno.h>
#include <unistd.h> 
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>  
//
int logmsg(char* message);
//
char* generate_message_id(const char* domain)
{
    static char msg_id[300];
    struct timeval tv;
    pid_t pid = getpid();
    
    gettimeofday(&tv, NULL);
    
    unsigned long long ts = (unsigned long long)tv.tv_sec * 1000000ULL + tv.tv_usec;
    
    snprintf(msg_id, sizeof(msg_id), 
             "<%llu.%d.%d@%s>",
             ts,
             (int)pid,
             rand() % 100000,
             domain ? domain : "localhost");

    return msg_id;
}
//
int email(struct configparms * conf)
{
    time_t rawtime;
    struct tm * timeinfo;
    char datetime [80];
    char timestamp [30];
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    strftime (datetime,80,"%a, %d %b %Y %H:%M:%S",timeinfo);
    strftime (timestamp,20,"%H:%M:%S",timeinfo);
    if (conf->itest) {
        logmsg("Email start");
    }
    char failtime[30] ;
    char resettime[30] ;
    char suspendtime[30];
    timeinfo = localtime (&conf->failtime);
    strftime (failtime,30,"%a, %d %b %Y %H:%M:%S",timeinfo);
    timeinfo = localtime (&conf->resettime);
    strftime (resettime,30,"%a, %d %b %Y %H:%M:%S",timeinfo);
    time_t suspend ;
    suspend = rawtime + 43200 ;
    timeinfo = localtime (&suspend);
    strftime (suspendtime,30,"%a, %d %b %Y %H:%M:%S",timeinfo);
    //
    char payload_text [5000];
    int i ;
    strcpy(payload_text,"Date: ") ;
    strcat(payload_text,datetime) ;
    strcat(payload_text,"\r\n") ;
    for ( i = 0; i < conf->ito; i++) {
        if (i == 0) {
            strcat(payload_text,"To: ") ;
        }  else {
            strcat(payload_text, " , ") ;
        }
        strcat(payload_text,conf->sztoperson[i]);
        strcat(payload_text," ") ;
        strcat(payload_text,conf->szsmtpto[i]);
    }
    //
    strcat(payload_text,"\r\nFrom: ") ;
    strcat(payload_text,conf->szfromperson);
    strcat(payload_text," ") ;
    strcat(payload_text,conf->szsmtpfrom);
    //
    for ( i = 0; i < conf->icc; i++) {
        if (i == 0) {
            strcat(payload_text,"\r\nCc: ") ;
        }  else {
            strcat(payload_text, " , ") ;
        }
        strcat(payload_text,conf->szccperson[i]);
        strcat(payload_text," ") ;
        strcat(payload_text,conf->szsmtpcc[i]);
    }
    for ( i = 0; i < conf->ibcc; i++) {
        if (i == 0) {
            strcat(payload_text,"\r\nBcc: ") ;
        }  else {
            strcat(payload_text, " , ") ;
        }
        strcat(payload_text,conf->szbccperson[i]);
        strcat(payload_text," ") ;
        strcat(payload_text,conf->szsmtpbcc[i]);
    }
    //
    const char* domain = "gmail.com";     // Change to your domain
    char* msgid = generate_message_id(domain);
    printf("Message-ID: %s\n", msgid);
    strcat(payload_text,"\r\nMessage-ID: ") ;
    strcat(payload_text,msgid) ;
    strcat(payload_text,"\r\n") ;
    //strcat(payload_text,"X-Priority: 1 (Highest)\r\nX-MSMail-Priority: High\r\n") ;
    strcat(payload_text,"Subject: ") ;
    strcat(payload_text,"Comms recovered from failure at ");
    strcat(payload_text,timestamp);
    strcat(payload_text,"\r\n\r\n") ;
    strcat(payload_text,"\r\nComms lost      ") ;
    strcat(payload_text,failtime) ; 
    strcat(payload_text,"\r\nRouter reset    ") ;
    strcat(payload_text,resettime) ;
    strcat(payload_text,"\r\nComms restored  ") ;
    strcat(payload_text,datetime);  //
    strcat(payload_text,"\r\n\r\nEmail notifications now suspended for 12 hours until ") ;
    strcat(payload_text,suspendtime);
    //  
    if (conf->itest) {
        printf("Payload: \n%s\n",payload_text); 
    }
    //
    // 2. Open pipe to Postfix's sendmail binary
    // The -t flag tells sendmail to extract recipients from the To: header
    FILE *mail_pipe = popen("/usr/sbin/sendmail -t -f middleobservatory@gmail.com", "w");
    if (mail_pipe == NULL) {
        perror("Failed to open sendmail pipe");
        return 1;
    }
    // 3. Write headers and body to the pipe
    fprintf(mail_pipe, "%s\n", payload_text);
    // 4. Close the pipe and verify Postfix accepted the email
    int exit_code = pclose(mail_pipe);
    char message[200] ;
    if (exit_code == 0) {
        conf->lastemail = rawtime ;
        //printf("Email successfully queued in Postfix.\n");
        logmsg("email successfully queued in postfix") ;
    } else {
        sprintf(message,"error queueing email. Postfix exit code: %d", exit_code);
        logmsg(message) ;
    }
    return 0;
}

/*
    //
    // 2. Open pipe to Postfix's sendmail binary
    // The -t flag tells sendmail to extract recipients from the To: header
    FILE *mail_pipe = popen("/usr/sbin/sendmail -t -f noreply@aussiesky.net", "w");
    */
