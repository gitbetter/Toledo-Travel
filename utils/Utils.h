#ifndef Utils_h
#define Utils_h

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define CHAT_PORT       4983
#define MAX_PORTS       8
#define MAX_READ_SIZE  	2048 
#define LISTEN_BACKLOG  50

typedef enum {FALSE=0,TRUE=1} boolean;

void sprintheader(char *, char *);
void srepeatprint(char *, char, int);
void die(char *);
void printusage(char *, char *);
int isnumeric(const char *);
void strtolower(char *);
int containsstr(const char *[], size_t, char *);
char *trimws(char *);
int charcount(char *, char);
char *strjoin(char **, int); 

#endif
