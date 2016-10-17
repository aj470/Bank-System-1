#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <string.h>
#include "server.h"

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KWHT  "\x1B[37m"

#define PORT 9999

boolean isPrinting = FALSE;
int numAccounts = 0; 

void *printOut(void *accounts)
{	
	Account_Info *temp = (Account_Info *) accounts;
	
	while(1)
	{	
		int count = 0;
		int printTimer = 20;
		
		printf("\n***********************************Account Log************************************************\n");
		printf("Account Name\t\t\tCurrent Balance\t\t\t Session\n");
		while (count != 20)
		{
			isPrinting = TRUE;
			//check if empty
			if (strcmp(temp[count].AccName, "\0")== 0)
			{
				count++;
				continue;
			}
			printf("%s\t\t\t\t%.2f\t\t\t\t", temp[count].AccName, temp[count].CurrBalance);
			if (temp[count].flag == TRUE)
			{
			  printf("%sACTIVE%s\n",KGRN,KWHT);
			}
			else
			{
			  printf("%sINACTIVE%s\n",KRED,KWHT);
			}
			count++;
		}
		isPrinting= FALSE;
		while (printTimer != 0) 
		{
			printTimer = sleep(printTimer);
		}
	}
}




int searchAccMap(char *name, Account_Info* accounts)
{
	int count = 0;
	printf("HERE\n");
	while (count < 20)
	{
	  if (strcmp(accounts[count].AccName, name)== 0)
	    {
	      printf("searching...\n");
	      return count;
	    }
	  printf("%d\n", count);
	  count++;
	}
	printf("HERE\n");
	//account was not found
	return -1;
}

void readUserCommand(Account_Info *accounts, int clientfd)
{
	char buffer[106]; // max number or character should be accountname+" "+"start"
	char cmd[6];
	char strOrFloat[100];
	int temp = 0;

	char anotherBuffer[100];

	sem_t *sem = sem_open(SEM, 0);

	while (TRUE)
	{	
		bzero(cmd, sizeof(cmd));
		bzero(strOrFloat, sizeof(strOrFloat));
		bzero(buffer, sizeof(buffer));
		read(clientfd, buffer, 106);
		sscanf(buffer, "%s %s", cmd, strOrFloat);

		if(strcmp(cmd, "open") == 0)
		{
		  int i = 0;
		  if (numAccounts == 21) 
		    {
		      write(clientfd, "Maximum number of accounts has already been reached.\n", 50);
		    } 
		  while (isPrinting)
		    {
		      sleep(1);
		    }
		  sem_wait(sem);
		  i = searchAccMap(strOrFloat, accounts);
		  if (strcmp(accounts[i].AccName, "\0") != 0) 
		    {
		      write(clientfd, "Account name already exists.", 50);
		    } 
		  else 
		    {
		      write(clientfd, "Creating...\n", 20); 
		      strcpy(accounts[i].AccName, strOrFloat);
		      accounts[i].CurrBalance = 0;
		      accounts[i].flag = FALSE;
		      write(clientfd, "Account now open", 50);
		      write(clientfd, "Starting Session", 50);
		    }
		  sem_post(sem);
		}
		else if (strcmp(cmd, "start") == 0)
		  {
		    int currentAcct = searchAccMap(strOrFloat, accounts); // index of the current account
		    if (numAccounts == 21 || currentAcct == -1)
		      {
			write(clientfd, "Account does not exist.", 50);
			continue;
		      }
		    if (accounts[currentAcct].flag == TRUE)
		      {
			write(clientfd, "Account is already in session.", 60);
			continue;
		      }
		    accounts[currentAcct].flag = TRUE;
		    write(clientfd, "Account is now ", 50);
		    while (TRUE)
		      {
			bzero(anotherBuffer, sizeof(anotherBuffer));
			bzero(buffer, sizeof(buffer));
			read(clientfd, buffer, 106);
			sscanf(buffer, "%s %s", cmd, strOrFloat);
			double amt = atof(strOrFloat);
			if (strcmp(cmd, "credit") == 0)
			  {
				sem_wait(sem);
				if (amt <= 0)
				  {
				    write(clientfd, "Invalid input request.", 60);
				  }
				else 
				  {
				    //add requested credit to current balance
				    accounts[currentAcct].CurrBalance += amt;
				    //write fromatted string to buffer
				    int temp = snprintf(anotherBuffer, 100, "Updated Balance: $%.2f", accounts[currentAcct].CurrBalance);
				    //check for successful write to buffer
				    if(temp > 0)
				      {
					anotherBuffer[temp] = '\0';
					write(clientfd, anotherBuffer, sizeof(anotherBuffer));
				      }
				    else
				      {
					perror("error writing to anotherBuffer");
				      }
				  }
				sem_post(sem);
			  }
			else if (strcmp(cmd, "debit")== 0)
			  {
			    sem_wait(sem);
			    //check if debiting more than current balance
			    if (amt > accounts[currentAcct].CurrBalance)
			      {
				write(clientfd, "Insufficient funds", 100);
			      } 
			    else if (amt <= 0)
			      {
				write(clientfd, "Invalid input", 60);
			      }
			    else 
			      {
				//debit account by requested amount
				accounts[currentAcct].CurrBalance -= amt;
				//write formatted string to buffer
				int temp= snprintf(anotherBuffer, 100, "Updated Balance : $%.2f", accounts[currentAcct].CurrBalance);
				//check for successful write to buffer
				if(temp > 0)
				  {
				    anotherBuffer[temp] = '\0';
				    write(clientfd, anotherBuffer, sizeof(anotherBuffer));
				  }
				else
				  {
				    perror("error writing to anotherBuffer");
				  }
			      }
			    sem_post(sem);
			}
			else if (strcmp(cmd, "balance")== 0)
			  {
			    sem_wait(sem);
			    temp = snprintf(anotherBuffer, 100, "Current Balance : $%.2f", accounts[currentAcct].CurrBalance);
			    if(temp > 0)
			      {
				anotherBuffer[temp] = '\0';
				write(clientfd, anotherBuffer, sizeof(anotherBuffer));
			      }
			    else
			      {
				perror("error writing to anotherBuffer");
			      }
			    sem_post(sem);
			}
			else if (strcmp(cmd, "finish") == 0)
			  {
			    //change session flag to false
			    accounts[currentAcct].flag = FALSE;
			    write(clientfd, "Session ended. You can start a new session or exit.", 100);
			    break;
			}
			else if (strcmp(cmd, "exit")== 0)
			  {
				//change session flag to false
			    accounts[currentAcct].flag = FALSE;
			    write(clientfd, "Disconnected from server.", 50);
			    return;
			  }
			else
			  {
			    write(clientfd, "Invalid in-session command.", 50);
			  }
		      }
		  }		
		else if (strcmp(cmd, "exit")== 0) 
		  {
		write(clientfd, "Disconnecting from Server", 10);
		return;
		  } 
		else 
		  {
		    write(clientfd, "Invalid operation", 50);
		  }
		bzero(strOrFloat, strlen(strOrFloat));
		bzero(cmd, strlen(cmd));		
	}

	if (clientfd <= 0)
	{
		perror("error accepting\n");
	}
}

Account_Info *mapData()
{
  int fd;
	//open/create file with read and write permission and check return value
	if ((fd = open("accounts", O_RDWR|O_CREAT, 0644)) == -1)	
        {
		perror("Unable to open account list file.");
		exit(0);
	}
	ftruncate(fd, 100*(sizeof(Account_Info)));
	
	//map data to be shared with different processes 
	Account_Info *accounts = mmap((void*)0, (size_t) 100*(sizeof(Account_Info)), PROT_WRITE, MAP_SHARED, fd, 0);
	
	int count= 0;

	//loop to initialize values of Account_Info struct
	while (count != 20)
	{
		accounts[count].CurrBalance= 0;
		accounts[count].flag = 0;
		int i = 0;
		while (i != 100)
		{
			//place NULL terminator into each element of AccName
			accounts[count].AccName[i]= '\0'; 
			i++;
		}
		
		count++;
	}

	close(fd);
	return accounts;
}

int main(int argc, char **argv)
{
	

	pid_t cpid;
	int sockfd, clientfd;
	struct sockaddr_in server, client;
	int len= sizeof(struct sockaddr_in);

	//	sem_t *sem;
	//sem = sem_open(SEM, O_CREAT, 0644, 4); 
	
	sockfd = socket(AF_INET , SOCK_STREAM , 0);
	if (sockfd == -1)
		perror("Could not create socket");
	else
		printf("socket has been created!\n");

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	int bid = bind(sockfd, (struct sockaddr *)&server, sizeof(server));
	if (bid < 0)
	{
		perror("Error to Bid with client"); 
		return 0;
	}
	else
	{
		printf("binding successful with client!\n");
	}

	int option= 0;
	int h = setsockopt(sockfd, SOL_SOCKET, (SO_REUSEADDR), (char *)&option, sizeof(option)); 
	if (h < 0)
	{
		printf("Setting Socket failed\n");
	}
	
	if (listen(sockfd, 20) != 0)
	{
		perror("Listening Max threads");
		exit(0);
	} 
	else
	{
		puts("now listening...\n");
	}

	signal(SIGPIPE, SIG_IGN);
	Account_Info *actArr = mapData();
	pthread_t pAccounts;

		
      if (pthread_create(&pAccounts, NULL, printOut, (void *)actArr) < 0)
	  {
	    perror("could not create thread\n");
	  }

	while ((clientfd= accept(sockfd, (struct sockaddr*)&client, (socklen_t*)&len)))
	{
	  	cpid= fork();
	    if (cpid== 0)
	    {
	    	int newFD= clientfd;
	    	printf("\nFork was successful!  Child's PID is: %d\n", (int)getpid());
			readUserCommand(actArr, newFD);
		}
	}

	//parent process waits for child processes
	if (cpid != 0) 
	{
		int status;
		waitpid(cpid, &status, 0);
	}

	close(clientfd);
	close(sockfd);
	return 0;
}
