#ifndef SERVER_H
#define SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define SEM "/sem"

typedef enum {FALSE, TRUE} boolean;

typedef struct Account_Info_{
	char AccName[100];
	double CurrBalance;
	boolean flag;
}Account_Info;

void *printOut(void *accounts);
void readUserCommand(Account_Info *accounts, int clientfd);
int searchAccMap(char *name, Account_Info *accounts);
Account_Info *mapData();

//function pointer to use as thread start routine
void *printOut(void *);

#endif
