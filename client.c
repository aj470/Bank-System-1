#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <signal.h>
#include "client.h" 

#define PORT 9999

char message[200];
boolean isdone = FALSE;
int sockID;

void Client_Exit(int sig)
{
  strcpy(message, "exit");
  send(sockID , message , strlen(message) , 0);
  isdone = TRUE;
  exit(1);
}

void *sendToServer(void *sockID)
{
    int nsockfd = *(int*)sockID;
    printf("Enter message : \n");

    while (!isdone)
    { 
        bzero(message, strlen(message));
        fgets(message, sizeof(message), stdin);
        fseek(stdin, 0, SEEK_END);
        if (strlen(message) > 106)
        {
	  bzero(message, strlen(message));
	  
	  printf("Invalid command.\n");
	  printf("Re-Enter message : \n");
	  sleep(2);
	  continue;
        }
        if (strcmp(message, "exit\n") == 0)
        {
	  send(nsockfd , message , strlen(message) , 0);
	  isdone = TRUE;
	  break;
        } 
        if (send(nsockfd , message , strlen(message) , 0) == -1)
	  {
            perror("Message send failure.");
	    isdone = TRUE;
            break;
	  }
	
        sleep(2);
        printf("Enter message : \n");
    }
    pthread_exit(0);
}

void *receiveThread(void *sockID)
{
    int newID = *(int*)sockID;
    char serverMes[2000];
	
    while (!isdone)
    {    
        if (recv(newID , serverMes , 2000 , 0) <= 0)
        { 
            isdone = TRUE; 
            printf("Disconnected from server.\n");
			exit(0);
        }

		int check;
        check = puts("Server:");
		if(check > 0)
		{
			puts(serverMes);
		}
        else
		{
			perror("Error with puts()");
		}
    }
    pthread_exit(0);
}

int main(int argc , char *argv[])
{
    struct sockaddr_in server_addr;
    struct hostent *ht;
    pthread_t sendThread;
    pthread_t recThread;
    int setup = 0;
    int send,recieve;
    boolean ready = TRUE; 
     
    /*Get Host and Create Socket*/
    if(argc != 2)
      {
	printf("Insufficient Argument: Host Missing\n");
	exit(0);
      }

      ht= gethostbyname(argv[1]);
      if(ht == NULL)    
      {
        perror("Error Retreaving Hostname!\n\n");
        exit(0);
      }

    sockID = socket(AF_INET , SOCK_STREAM , 0);

    if (!sockID > 0)
      {
	printf("Failed to Create Socket! Try Again!\n\n");
	exit(0);
      }
  
    printf("Successfully Created Socket!\n\n");
   
    /*Connect*/
    memcpy(&server_addr.sin_addr, ht->h_addr, ht->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)(ht-> h_addr));
 
    setup = connect(sockID, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
    while (setup < 0)
    {
        ready = FALSE;
        setup = connect(sockID, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
        printf("Connection Failed, retrying in 3 seconds!\n");
        sleep(3);
    }

    if(ready == TRUE)
      {
	printf("Connection Successful to Server!\n\n");
	printf("Creating Threads to Server!\n\n");
	recieve = pthread_create(&recThread, NULL, receiveThread, (void *) &sockID);
        send = pthread_create(&sendThread, NULL, sendToServer, (void *) &sockID);
	  if(send != 0 || recieve !=0 )
	  {
	    perror("Error in Creating Threads!\n\n");
	    exit(0);
	   }
	  printf("Threads Created to Server!\n\n"); 
	  printf("Start talking to Server\n\n");
	  printf("Options:\n");
	  printf("# open accountname\n");
	  printf("# start accountname\n");
          printf("# credit amount\n");
          printf("# debit amount\n"); 
          printf("# balance\n");
	  printf("# finish\n");
	  printf("# exit \n");
	signal(SIGINT, Client_Exit);
	pthread_join(recThread, NULL);
	pthread_join(sendThread, NULL);
      }  
 
    close(sockID);     
    return 0;
}


