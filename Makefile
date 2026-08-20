CC=gcc
CFLAGS=-c -Wall 
GPIOLIB=-lgpiod
MATHLIB=-lm
all:webmon

webmon:webmon.o  log.o config.o email.o ping.o
	$(CC)  webmon.o  log.o ping.o config.o email.o -o webmon $(GPIOLIB)  $(MATHLIB) 

webmon.o:webmon.c
	$(CC) $(CFLAGS) webmon.c -o webmon.o

log.o:log.c
	$(CC) $(CFLAGS) log.c -o log.o	

ping.o:ping.c
	$(CC) $(CFLAGS) ping.c -o ping.o		

email.o:email.c
	$(CC) $(CFLAGS) email.c -o email.o		
	
config.o:config.c
	$(CC) $(CFLAGS) config.c -o config.o	
		
clean:
	rm -rf *o webmon
	

