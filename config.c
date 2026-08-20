//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <webcheck.h>
#include <regex.h>
#include <ctype.h>
//
//#define MAXADDR 5
int logmsg(char* message) ;
//
//
char *rtrim(char *s)
{
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}//
int config(struct configparms *conf ) {
    strcpy(conf->szpingurl,"8.8.8.8") ;
    strcpy(conf->szsmtpfrom,"middleobservatory@gmail.com") ;
    conf->failtime  = 0 ;
    //conf->recovertime = 0 ;
    conf->resettime = 0 ;
    conf->lastemail = 0 ;
    conf->webstatus = 0 ; 
    conf->delay = 1821 ;    
    conf->interval = 3601 ;
    regex_t rxsmtpto ;
    regex_t rxsmtpfrom ;
    regex_t rxsmtpcc ;
    regex_t rxsmtpbcc ;
    regex_t rxdelay ;
    regex_t rxinterval ;
    regex_t rxpingurl ;
    //
	

    char szdelay [10] ;
    strcpy(szdelay,"1800") ;
    char szinterval [10] ;
    strcpy(szinterval,"3601") ;   
    char *rzsmtpto     = "^smtpto\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    //char *rzsmtpto     = "^smtpto\\s+(.*)$" ;
    char *rzsmtpfrom   = "^smtpfrom\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    char *rzsmtpcc     = "^smtpcc\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    char *rzsmtpbcc    = "^smtpbcc\\s+(\\S+.*)(<\\S+@\\S+>)" ;
    //char *rzpingurl   = "^pingurl\\s+([A-Za-z0-9_ ]+)\\s.$" ;
    char *rzpingurl    = "^pingip\\s+(.*)$" ;
    //char *rzdelay      = "^delay\\s+([0-9]+)$" ;
    char *rzdelay      = "^delay\\s+([0-9]+)" ;
    char *rzinterval   = "^interval\\s+([0-9]+)" ;
    //
    regcomp(&rxsmtpto,    rzsmtpto, REG_EXTENDED);
    regcomp(&rxsmtpfrom,  rzsmtpfrom, REG_EXTENDED);
    regcomp(&rxsmtpcc,    rzsmtpcc, REG_EXTENDED);
    regcomp(&rxsmtpbcc,   rzsmtpbcc, REG_EXTENDED);
    regcomp(&rxpingurl,   rzpingurl, REG_EXTENDED);
    regcomp(&rxdelay,     rzdelay, REG_EXTENDED);
    regcomp(&rxinterval,  rzinterval, REG_EXTENDED);
    size_t maxGroups = 3;
    regmatch_t groupArray[maxGroups];
    FILE *fpconf ;
    char data[200] ;
    fpconf = fopen( "/etc/webcheck.conf", "r" ) ;
    if ( fpconf == NULL ){
        printf( "Could not open file /etc/webcheck.conf\n" ) ;
        logmsg("could not open file /etc/webcheck.conf");
        return 1;
    }
    conf->ito = 0, conf->icc = 0, conf->ibcc = 0 ;
    while( fgets ( data, 200, fpconf ) != NULL ){
        if ((!regexec(&rxsmtpto, data, maxGroups, groupArray, 0))  && (conf->ito < MAXADDR)){
            sprintf(conf->sztoperson[conf->ito],"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpto[conf->ito],"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]);
            rtrim(conf->szsmtpto[conf->ito]);
            conf->ito++;
        }
        if (!regexec(&rxsmtpfrom, data, maxGroups, groupArray, 0)){
            sprintf(conf->szfromperson,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpfrom,"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]); 
            rtrim(conf->szsmtpfrom);            
        }
        if ((!regexec(&rxsmtpcc, data, maxGroups, groupArray, 0))  && (conf->icc < MAXADDR)){
            sprintf(conf->szccperson[conf->icc],"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpcc[conf->icc],"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]);    
            rtrim(conf->szsmtpcc[conf->icc++]); 
            //printf("%s,%s\n",conf->szsmtpcc[conf->icc-1],conf->szccperson[conf->icc-1]);  
        }
        if ((!regexec(&rxsmtpbcc, data, maxGroups, groupArray, 0))   && (conf->ibcc < MAXADDR)){
            sprintf(conf->szbccperson[conf->ibcc],"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            sprintf(conf->szsmtpbcc[conf->ibcc],"%.*s",groupArray[2].rm_eo - groupArray[2].rm_so, &data[groupArray[2].rm_so]);
            rtrim(conf->szsmtpbcc[conf->ibcc++]);           
        }
        if (!regexec(&rxpingurl, data, maxGroups, groupArray, 0)){
            sprintf(conf->szpingurl,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(conf->szpingurl);
        }
         if (!regexec(&rxdelay, data, maxGroups, groupArray, 0)){
            sprintf(szdelay,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(szdelay);
            conf->delay=atoi(szdelay) ;
        }   
         if (!regexec(&rxinterval, data, maxGroups, groupArray, 0)){
            sprintf(szinterval,"%.*s",groupArray[1].rm_eo - groupArray[1].rm_so, &data[groupArray[1].rm_so]);
            rtrim(szinterval);
            conf->interval=atoi(szinterval) ;
        }   
    }
    fclose(fpconf) ;
    regfree(&rxpingurl) ;
    regfree(&rxsmtpto) ;
    regfree(&rxsmtpfrom) ;
    regfree(&rxsmtpcc) ;
    regfree(&rxdelay) ;
    regfree(&rxinterval) ;
    //
    int i ;
    char message[500];
    sprintf(message, "From:     %s %s",conf->szfromperson,conf->szsmtpfrom) ;
    logmsg(message) ;
    for(i = 0; i < conf->ito; ++i){
        sprintf(message, "To:       %s %s", conf->sztoperson[i], conf->szsmtpto[i]) ;
        logmsg(message) ;
    }
    for(i = 0; i < conf->icc; ++i){ 
        sprintf( message,"Cc:       %s %s",conf->szccperson[i], conf->szsmtpcc[i]) ;
        logmsg(message) ;
    }
    for(i = 0; i < conf->ibcc; ++i){
        sprintf(message, "Bcc:      %s %s",conf->szbccperson[i],conf->szsmtpbcc[i]) ;
        logmsg(message) ;
    }
    sprintf(message, "Ping IP:  %s",conf->szpingurl) ;
    logmsg(message) ;
    sprintf(message, "Delay:    %d",conf->delay) ;
    logmsg(message) ;
    sprintf(message, "Interval: %d",conf->interval) ;
    logmsg(message) ;   return 0 ;
}
