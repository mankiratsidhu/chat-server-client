#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
void error (const char *message) {
    perror (message);
    exit (1);
}

int main (int argc, char *argv[]) {
    
    if (argc != 3) {
        fprintf (stderr,"Please provide ip address and port number in format, example:\n ./client localhost 101010\n");
        exit (1);
    }
    
    int fd1, portNumber, n;
    char buffer [512];
    char buffer2 [512];
    struct sockaddr_in client_addr, server_addr; 
    socklen_t len;
    
    //SOCK_STREAM to use for tcp secure transfer
    fd1 = socket (AF_INET, SOCK_STREAM, 0);
    
    if (fd1 < 0) {
        error ("Error opening Socekt");
    }
    bzero ((char *) &server_addr, sizeof (server_addr)); // Clear data in what ever it is reference to
    portNumber = atoi (argv[2]);
    server_addr.sin_family = AF_INET;

    sscanf(argv[2], "%d", &portNumber);

    server_addr.sin_port = htons((uint16_t)portNumber);

    //convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, argv[1],&server_addr.sin_addr) < 0){
    fprintf(stderr, " inet_pton() has failed\n");
    exit(2);
    }
    
    
    if(connect(fd1, (struct sockaddr *) &server_addr,sizeof(server_addr))<0){
        fprintf(stderr, "connect() failed, exiting\n");
        exit(3);
    }
    
    
    printf ("[+]Connected to the server\n");
    
     while (1) {
        bzero (buffer, 255);
        fgets (buffer, 255, stdin);
        
        int quitN = strncmp ("quit", buffer, 4);
        int getFileN = strncmp ("get", buffer, 3);
        int putFileN = strncmp ("put", buffer, 3);
        
        if (quitN == 0) {
            printf ("Exiting from Client\n");
            n = write (fd1 , buffer, 255);
            if (n < 0) {
                error ("Error on writting command");
            }
            break;
        }
        else if (getFileN == 0) {
            printf ("Get File\n");
            n = write (fd1 , buffer, 255);
            if (n < 0) {
                error ("Error on writting command");
            }
            
            int ch = 0;
            int words;
            
            read (fd1, &words, sizeof (int));
            printf ("Received words : %d \n",words);
            
            char *token;
            token = strtok(buffer, " ");
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
                read (fd1, buffer, 255);
                fprintf (fp, "%s ", buffer);
                ch++;
            }
            fclose(fp);
            
            printf ("The file has been received successfully.\n");
            
        }
        else if (putFileN == 0) {
            printf ("Put File\n");
            n = write (fd1 , buffer, 255);
            if (n < 0) {
                error ("Error on writting command");
            }
            
            char *token;
            token = strtok(buffer, " ");
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
            
            FILE *file;
            
            if((file = fopen(fileName,"r"))!=NULL)
            {
                // file exists
                int words = 0;
                char buffer [255];
                
                char c;
                while ((c = getc (file)) != EOF) {
                    fscanf (file, "%s", buffer);
                    if (isspace (c) || c == '\t') {
                        words ++;
                    }
                }
                
                printf ("Total words : %d \n",words);
                write (fd1, &words, sizeof (int));
                rewind (file);
                
                char ch;
                while (ch != EOF) {
                    fscanf (file, "%s", buffer);
                    write (fd1 , buffer, 255);
                    ch = fgetc (file);
                }
                
                printf ("File Read\n");
                fclose(file);
            }
            else
            {
                printf ("File not found\n");
            }
        }
        else {
            printf ("Invalid command\n");
        }
    }
    
    
    close (fd1);
    return 0;
}
