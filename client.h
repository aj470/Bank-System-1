#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>

#ifndef CLIENT_H
#define CLIENT_H

typedef enum {FALSE, TRUE} boolean;

void *sendM(void *sockfd);
void *recT(void *sockfd);

void *sendM(void *);
void *recT(void *);

#endif


