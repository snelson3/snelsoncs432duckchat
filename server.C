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

 void dListUsers(map <string, struct sockaddr_in> users)
 {
   cerr<<users.size();
   fprintf(stderr,"Printing list of Users for DEBUG purposes\n");
   for ( map<string,struct sockaddr_in>::iterator it = users.begin(); it != users.end(); it++)
   {
     cerr<<it->first<<":";
     cerr<<"\tsin_port: "<<it->second.sin_port<<"\n";
   }
 }

void dListChannels(map<string,vector<string> > channels)
{
  cerr<<"Printing list of channels for DEBUG purposes\n";
  for ( map<string, vector<string> >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    cerr<<it->first<<":";
    for (int i=0; i < (int)it->second.size();i++)
    {
      cerr<<it->second[i]<<",";
    }
    cerr<<"\n";
  }
}

 int loggedIn(struct sockaddr_in connection, map <string, struct sockaddr_in> users)
 {
   for (map<string,struct sockaddr_in>::iterator it = users.begin(); it!= users.end(); it++)
   {
     if (connection.sin_port == it->second.sin_port)
     {
       return 1;
     }
   }
   cerr<<"Rejected packet from non logged in user\n";
   return 0;
 }

void logIn(struct request_login * l_packet, struct sockaddr_in client_addr, map<string, struct sockaddr_in> users)
{
  users.insert(make_pair(l_packet->req_username,client_addr));
  cerr<<l_packet->req_username<<" Logged in\n";
}

string getUser(map<string, struct sockaddr_in> users, struct sockaddr_in ip)
{
  for (map<string,struct sockaddr_in>::iterator it = users.begin(); it!=users.end(); it++)
  {
    if (it->second.sin_port == ip.sin_port) return it->first;
  }
  myError("Trying to get invalid user");
  return "NULL";
}

bool inChannel(string user, vector<string> channel)
{
  for (int i=0; i < (int)channel.size(); i++)
  {
    if (user==channel[i]) return true;
  }
  return false;
}

void join(request_join * packet,string user,map<string,vector<string> >channels)
{
  //whenever a user joins a nonexistent channel, it's created
  cerr<<user<<" joining "<<packet->req_channel<<"\n";
  bool ex = false;
  for (map<string,vector<string> >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    if (it->first==packet->req_channel)
    {
      //channel already exists, have to check if user already in channel
      if (!inChannel(user,it->second))
      {
        //can add user to channel
        it->second.push_back(user);
      }
      ex = true;
    }
  }
  if (!ex)
  {
    vector<string> joined_users;
    joined_users.push_back(user);
    //channel doesn't exist, create channel and add user to channel
    channels.insert(make_pair(packet->req_channel,joined_users));
  }
}

int main(int argc, char *argv[]) {
  map <string, struct sockaddr_in> users;
  map <string,vector <string> > channels;
  const char *host_name;
  int host_port;
  int my_socket;
  bool firstpacket = true;
  bool secondpacket = false;
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
      struct request u_packet[100];
      struct sockaddr_in client_addr;
      string hacky_name;
      int addrlen;

      //server running, wait for a packet to arrive
      recvfrom(my_socket,u_packet,100,0,(struct sockaddr *) &client_addr,(socklen_t *)&addrlen);
      if (secondpacket) {secondpacket = false; users.insert(make_pair(hacky_name,client_addr));}
      if (firstpacket) { firstpacket = false; secondpacket = true; hacky_name = ((request_login *)u_packet)->req_username;}
      //cerr<"I RECEIVED A PACKET\n";
      //fprintf(stderr,"I RECEIVED A PACKET");
      //output debugging whenever receiving message from client
        //[channel][user][message]

      //if server receives message from someone not logged in, ignore.

      //then I need to parse the packet
      if (u_packet->req_type == 0)
      {
        logIn((request_login *) u_packet, client_addr, users);
      }
      else if ((u_packet->req_type == 1) && (loggedIn(client_addr,users)))
      {
        //deal with logout request

        //whenever a channel has no users, it's deleted
      }
      else if ((u_packet->req_type == 2) && (loggedIn(client_addr,users)))
      {
        //deal with join request
        join((request_join *) u_packet, getUser(users,client_addr), channels );
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


      dListUsers(users);
      dListChannels(channels);
    }

    //server delivers messages from a user X to all users on X's active channel
         //must keep track of individual users and channels theyve subbed to
         //also must track each channel and subbed users on it


}
