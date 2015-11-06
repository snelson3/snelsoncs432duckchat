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

using namespace std;

void myError(const char *msg)
{
  perror(msg);
  exit(-1);
}

 void dlistUsers(map <string, struct sockaddr_in> users)
 {
   fprintf(stderr,"Printing list of Users for DEBUG purposes\n");
   for ( map<string,struct sockaddr_in>::iterator it = users.begin(); it != users.end(); it++)
   {
     cerr<<it->first<<":\n";
     //cerr<<"\ts_addr: "<<it->second.sin_addr.s_addr<<"\n";
     cerr<<"\tsin_port: "<<it->second.sin_port<<"\n";
     fprintf(stderr, "%d\n", it->second.sin_addr.s_addr);
   }
 }

 int loggedIn(struct sockaddr_in connection, map <string, struct sockaddr_in> users)
 {
   cerr<<"Looking to see if user with IP "<<connection.sin_addr.s_addr<<" and port "<<connection.sin_port<<" is logged in\n";
   for (map<string,struct sockaddr_in>::iterator it = users.begin(); it!= users.end(); it++)
   {
//     cerr<<"\t is it "<<it->first<<"?\n";
  //      cerr<<"\t connection's port vs "<<it->first<<"'s port:\n";
          cerr<<"\n\t\t"<<connection.sin_port <<" vs "<<it->second.sin_port<<"\n";
    //    cerr<<"\t connection's ip vs " <<it->first<<"'s port:\n";
      //    cerr<<"\t\t"<<connection.sin_addr.s_addr<<" vs "<<it->second.sin_addr.s_addr<<"\n";
     if (connection.sin_port == it->second.sin_port)
     {
       cerr<<"Logged in!\n";
       return 1;
     }
   }
   cerr<<"Sorry\n";
   return 0;
 }

int main(int argc, char *argv[]) {
  map <string, struct sockaddr_in> users;
  map <string,vector <string> > channels;
  const char *host_name;
  int host_port;
  int my_socket;
  struct sockaddr_in my_server;
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
      struct sockaddr_in client_addr;
      int addrlen;
      //server running, wait for a packet to arrive
      recvfrom(my_socket,u_packet,100,0,(struct sockaddr *) &client_addr,(socklen_t *)&addrlen);
      //cerr<"I RECEIVED A PACKET\n";
      //fprintf(stderr,"I RECEIVED A PACKET");
      //output debugging whenever receiving message from client
        //[channel][user][message]

      //if server receives message from someone not logged in, ignore.

      //then I need to parse the packet
      if (u_packet->req_type == 0)
      {
        users.insert(make_pair(u_packet->req_username,client_addr));
        cerr<<u_packet->req_username<<" Logged in with port " << client_addr.sin_port<<", for "<<users.size()<<" total users\n";
      }
      else if ((u_packet->req_type == 1) && (loggedIn(client_addr,users)))
      {
        //deal with logout request

        //whenever a channel has no users, it's deleted
      }
      else if ((u_packet->req_type == 2))// && (loggedIn(client_addr,users)))
      {
        //deal with join request
        cerr<<"User sent a join request with port "<<client_addr.sin_port<<"\n";
        //whenever a user joins a nonexistent channel, it's created
      }
      else if ((u_packet->req_type == 3) &&  (loggedIn(client_addr,users)))
      {
        //deal with leave request
        //whenever a channel has no users, it's deleted
      }
      else if ((u_packet->req_type == 4) &&  (loggedIn(client_addr,users)))
      {
        //deal with say request
      }
      else if ((u_packet->req_type == 5) &&  (loggedIn(client_addr,users)))
      {
        //deal with list request
      }
      else if ((u_packet->req_type == 6) &&  (loggedIn(client_addr,users)))
      {
        //deal with who request
      }
      else if (loggedIn(client_addr,users))
      {
        //send an error
      }

   dlistUsers(users);

    }


    //server delivers messages from a user X to all users on X's active channel
         //must keep track of individual users and channels theyve subbed to
         //also must track each channel and subbed users on it





}
