#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include<ctype.h>
#include <sys/types.h>
#include <time.h> 

void error (const char *message) {
    perror (message);
    exit (1);
}

void serviceSocket(int fd){
  	printf ("Client found\n");
    char buffer [255];
    char commandBuffer [255];
    char *quit = "quit";
    
    if (fd < 0) {
        error ("Error on Accept");
    }
    
    while (1) {
        
        // Read Command
        read (fd, &commandBuffer, 255);
        
        int quitN = strncmp ("quit", commandBuffer, 4);
        int getFileN = strncmp ("get", commandBuffer, 3);
        int putFileN = strncmp ("put", commandBuffer, 3);
        
        if (quitN == 0) {
            printf ("Exiting from parent\n");
            break;
        }
        else if (getFileN == 0) {
            // Get File
            
            printf ("Get File\n");
            char *token;
            token = strtok(commandBuffer, " ");
            
            int index = 0;
            char *fileName;
            while (token != NULL) {
                if (index == 1) {
                    fileName = token;
                }
                token = strtok (NULL, " ");
                index = index + 1;
            }
            
            FILE *file;
            
            size_t length = strlen(fileName);
            for (int i=0 ; i< length ; i++) {
                if (fileName[i] == '\n' || fileName[i] == EOF) {
                    fileName[i] = '\0';
                }
            }
            
            if((file = fopen(fileName,"r"))!=NULL)
            {
                // file exists
                int words = 0;
                char buffer [255];
                
                char c;
                while ((c = getc (file)) != EOF) {
                    fscanf (file, "%s", buffer);
                    if (isspace(c) || c == '\t') {
                        words ++;
                    }
                }
                
                printf ("Total words : %d \n",words);
                write (fd, &words, sizeof (int));
                rewind (file);
                
                char ch;
                while (ch != EOF) {
                    fscanf (file, "%s", buffer);
                    
                    printf ("Written : %s \n", buffer);
                    write (fd , buffer, 255);
                    ch = fgetc (file);
                }

                printf ("File Read\n");
                fclose(file);
            }
            else
            {
                printf ("File not found\n");
                //File not found, no memory leak since 'file' == NULL
                //fclose(file) would cause an error
            }
        }
        
        else if (putFileN == 0) {
            // Put File
            
            int ch = 0;
            int words;
            
            read (fd, &words, sizeof (int));
            printf ("Received words : %d \n",words);
            
            printf ("Put File\n");
            char *token;
            token = strtok(commandBuffer, " ");
            
            int index = 0;
            char *fileName;
            while (token != NULL) {
                if (index == 1) {
                    fileName = token;
                }
                token = strtok (NULL, " ");
                index = index + 1;
            }
            
            size_t length = strlen(fileName);
            for (int i=0 ; i< length ; i++) {
                if (fileName[i] == '\n' || fileName[i] == EOF) {
                    fileName[i] = '\0';
                }
            }
            
            FILE *fp;
            fp = fopen (fileName, "a");
            
            printf ("Writting to : %s \n", fileName);
            
            while (ch != words) {
                read (fd, buffer, 255);
                fprintf (fp, "%s ", buffer);
                ch++;
            }
            fclose(fp);
            
            printf ("The file has been received successfully.\n");
        }
    }
    
    close (fd);
}

int main(int argc, char *argv[])
{
    int listenfd = 0, connfd = 0, portNumber;
    struct sockaddr_in serv_addr; 

    char sendBuff[1025];
    time_t ticks; 

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    portNumber = atoi (argv[1]);
    serv_addr.sin_port = htons(portNumber); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 5); 

    while(1)
    {
        connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		if(!fork()){
			close(listenfd);
			serviceSocket(connfd);
		}
       
     }
	 
  }