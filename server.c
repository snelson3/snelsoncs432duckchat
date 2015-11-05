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
#include <map>
#include <vector>

struct logged_in_user {
  char username[USERNAME_MAX];
} packed;

void myError(const char *msg)
{
  perror(msg);
  exit(-1);
}
//
// void listUsers(std::map<struct sockaddr_in *, char *> users)
// {
//   fprintf(stderr,"Printing list of Users for DEBUG purposes\n");
//   for ( std::map<struct sockaddr_in *,char *>::iterator it = users.begin(); it != users.end(); it++)
//   {
//     fprintf(stderr, "%s\n", it->second);
//   }
// }

int main(int argc, char *argv[]) {
  std::map <struct sockaddr_in , struct logged_in_user> users;
  std::map <char,std::vector <char> > channels;
  const char *host_name;
  int host_port, addrlen;
  int my_socket;
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

    while (1) {
      struct request_login u_packet[100];
      //server running, wait for a packet to arrive
      recvfrom(my_socket,u_packet,100,0,(struct sockaddr *) &client_addr,(socklen_t *)&addrlen);
      //output debugging whenever receiving message from client
        //[channel][user][message]

    //if server receives message from someone not logged in, ignore.

      //then I need to parse the packet
      if (u_packet->req_type == 0)
      {
        struct logged_in_user new_user;
        strcpy(new_user.username, u_packet->req_username);
        //deal with login request
        users.insert(std::make_pair(client_addr,new_user));
        fprintf(stderr,"after call, users size is %ld\n", users.size());
      }
      else if (u_packet->req_type == 1)
      {
        //deal with logout request

        //whenever a channel has no users, it's deleted
      }
      else if (u_packet->req_type == 2)
      {
        //deal with join request

        //whenever a user joins a nonexistent channel, it's created
      }
      else if (u_packet->req_type == 3)
      {
        //deal with leave request
        //whenever a channel has no users, it's deleted
      }
      else if (u_packet->req_type == 4)
      {
        //deal with say request
      }
      else if (u_packet->req_type == 5)
      {
        //deal with list request
      }
      else if (u_packet->req_type == 6)
      {
        //deal with who request
      }
      else
      {
        //send an error
      }

  //  listUsers(users);

    }


    //server delivers messages from a user X to all users on X's active channel
         //must keep track of individual users and channels theyve subbed to
         //also must track each channel and subbed users on it





}
