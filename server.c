#include <stdio.h>
#include "duckchat.h"
#include <sys/socket.h> // Core BSD socket functions and data structures
#include <netinet/in.h> //AF_INET and AF_INET6 address families
#include <sys/un.h> //AF_UNIX address family. Used for local communication between programs running on the same computer
#include <arpa/inet.h> //Functions for manipulating numeric IP addresses.
#include <netdb.h> //Functions for translating protocol names and host names into numeric addresses
#include <stdlib.h>
#include <iostream>
#include <string>
#include <math.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>

void myError(const char *msg)
{
  perror(msg);
  exit(-1);
}


int main(int argc, char *argv[]) {
  const char *host_name;
  int host_port, addrlen;
  int my_socket;
  char msg[50];
  struct sockaddr_in my_server, client_addr;
    //server takes two arguments
        //host_address port_number
    if (argc!=3)
      myError("Wrong number of arguments");
    if (strcmp("localhost",argv[1]) == 0)
      host_name = LOCALHOST;
    else
      host_name = argv[1];
    host_port = strtol(argv[2],NULL,0);

    my_socket = socket(PF_INET,SOCK_DGRAM,0);

    //binding the socket
    my_server.sin_family = AF_INET;
    my_server.sin_addr.s_addr = htons(INADDR_ANY);
    my_server.sin_port = htons(host_port);

    bind(my_socket, (struct sockaddr *) &my_server, sizeof(my_server));

    recvfrom(my_socket,msg,11,0,(struct sockaddr *) &client_addr,(socklen_t *)&addrlen);

    fprintf(stderr,"printed\n");

    //binds to an ip address, if localhost 127.0.0.1

    //output debugging whenever receiving message from client
      //[channel][user][message]

    //server delivers messages from a user X to all users on X's active channel
         //must keep track of individual users and channels theyve subbed to
         //also must track each channel and subbed users on it

    //whenever a channel has no users, it's deleted
    //whenever a user joins a nonexistent channel, it's created

    //if server receives message from someone not logged in, ignore.
}
