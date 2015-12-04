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

/* Server struct, used to keep track of IPs/Host names for the servers*/

struct c_server {
  sockaddr_in server;
  int socket;
};

struct channelcontents {
  vector<struct c_server> servers;
  vector<string> usernames;
};

void myError(const char *msg)
{
  perror(msg);
  exit(-1);
}

void outputMSG(struct sockaddr_in host, struct sockaddr_in client,const char * packetmsg)
{
  cerr<<"127.0.0.1:"<<host.sin_port<<" 127.0.0.1:"<<client.sin_port<<" "<<packetmsg<<"\n";
}

void outputWithChannel(struct sockaddr_in host, struct sockaddr_in client, const char * packetmsg, const char * channelname)
{
  cerr<<"127.0.0.1:"<<host.sin_port<<" 127.0.0.1:"<<client.sin_port<<" "<<packetmsg<<" "<<channelname<<"\n";
}

void outputASay(struct sockaddr_in host, struct sockaddr_in client, const char * packetmsg, string username, const char * channelname, const char *msg)
{
  cerr<<"127.0.0.1:"<<host.sin_port<<" 127.0.0.1:"<<client.sin_port<<" "<<packetmsg<<" "<<username<<" "<<channelname<<" "<<msg<<"\n";
}

 void dListUsers(map <string, struct sockaddr_in> users)
 {
   fprintf(stderr,"Printing list of Users for DEBUG purposes\n");
   for ( map<string,struct sockaddr_in>::iterator it = users.begin(); it != users.end(); it++)
   {
     cerr<<it->first<<":";
     cerr<<"\tsin_port: "<<it->second.sin_port<<"\n";
   }
 }

void dListChannels(map<string,vector<channelcontents> > channels)
{
  cerr<<"Printing list of channels for DEBUG purposes\n";
  for ( map<string, vector<channelcontents> >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    cerr<<it->first<<":";
    for (int i=0; i < (int)it->second.usernames.size();i++)
    {
      cerr<<it->second.usernames[i]<<",";
    }
    cerr<<"\n";
  }
}

int getUserIndex(string user, vector<string> channel)
{
  for (int i = 0; i < (int)channel.size(); i++)
  {
    if (user==channel[i])
    {
      return i;
    }
  }
  return -1;
}

void leave(string user, string channel, map<string,vector<channelcontents> > *channels)
{
  //find the channel
  //remove the user from it
  //if theres nobody in the channel, remove it from the map
  for (map<string,vector<channelcontents> >::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    if (channel == it->first)
    {
      int n;
      n = getUserIndex(user,it->second.usernames);
      if (n>-1) it->second.usernames.erase(it->second.usernames.begin()+n);
      if (it->second.usernames.size() == 0)
      {
        channels->erase(it->first);
      }
      return;
    }
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

void logIn(struct request_login * l_packet, struct sockaddr_in client_addr, map<string, struct sockaddr_in> *users)
{
  users->insert(make_pair(l_packet->req_username,client_addr));
}

void logout(string user, map<string,struct sockaddr_in> *users, map<string,vector<channelcontents> > *channels)
{
  //first go through each channel and remove user from the channel's logged in list (make sure to run checkEmpty, or call the Leave function or something)
  for (map<string,vector<channelcontents> >::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    leave(user,it->first,channels);
  }
  users->erase(user);
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

sockaddr_in getUserByName(map<string,struct sockaddr_in> users, string name)
{
  return users[name];
}

bool inChannel(string user, vector<string> channel)
{
  for (int i=0; i < (int)channel.size(); i++)
  {
    if (user==channel[i]) return true;
  }
  return false;
}

void cpString(const string& input, char *dst, size_t dst_size)
{
  strncpy(dst, input.c_str(),dst_size - 1);
  dst[dst_size -1] = '\0';
}

void s2sJoin(int server_socket, const char * channel)
{
  int err;
  struct s2s_join p_join;
  p_join.req_type = 8;
  strcpy(p_join.s2s_channel,channel);
  err = send(server_socket, &p_join, sizeof p_join, 0);
  if (err < 0) { myError("Error sending S2S Join");}
}

void joinS2S(s2s_join * packet, map<string,vector<channelcontents> > * channels, vector<c_server> servers, sockaddr_in host)
{
  bool ex = false;
  for (map<string,vector<channelcontents> >::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    if (it->first == packet->s2s_channel)
    {
      //channel already exists, join chain ends
      return;
    }
  }
  //channel doesn't exist, create it and send more joins
  struct channelcontents cc;
  vector<string> joined_users;
  cc.usernames = joined_users;
  channels->insert(make_pair(packet->s2s_channel,cc));
  for (int i = 0; i < (int)servers.size(); i++)
  {
    outputWithChannel(host,servers[i].server,"send S2S Join", packet->s2s_channel);
    s2sJoin(servers[i].socket,packet->s2s_channel);
  }
}

void join(request_join * packet,string user,map<string,vector<channelcontents> > *channels, vector<c_server> servers,sockaddr_in host)
{
  //whenever a user joins a nonexistent channel, it's created
  bool ex = false;
  for (map<string,vector<channelcontents> >::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    if (it->first==packet->req_channel)
    {
      //channel already exists, have to check if user already in channel
      if (!inChannel(user,it->second.usernames))
      {
        //can add user to channel
        it->second.usernames.push_back(user);
      }
      ex = true;
    }
  }
  if (!ex)
  {
    struct channelcontents cc;
    vector<string> joined_users;
    joined_users.push_back(user);
    cc.usernames = joined_users;
    //channel doesn't exist, create channel and add user to channel, also S2S join
    channels->insert(make_pair(packet->req_channel,cc));
    for (int i = 0; i < (int)servers.size(); i++)
    {
      outputWithChannel(host,servers[i].server,"send S2S Join ", packet->req_channel);
      s2sJoin(servers[i].socket, packet->req_channel);
    }
  }
}

void sendError(sockaddr_in connection, int socket,const char * msg)
{
  struct text_error packet;
  packet.txt_type = TXT_ERROR;
  strcpy(packet.txt_error,msg);
  sendto(socket,&packet,sizeof packet, 0, (const sockaddr *)&connection, sizeof connection);
}

void say(request_say *packet,string user,map<string,struct sockaddr_in> users,map<string,vector<channelcontents> > channels,int socket)
{
  struct text_say send_packet;
  send_packet.txt_type = TXT_SAY;
  strcpy(send_packet.txt_channel,packet->req_channel);
  cpString(user,send_packet.txt_username,sizeof send_packet.txt_username);
  strcpy(send_packet.txt_text,packet->req_text);
  for (map<string,vector<channelcontents> >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    if (it->first==packet->req_channel)
    {
      //this is the right channel, iterate through the users to send the message multiple times
      for (int i = 0; i < (int)it->second.usernamessize(); i++)
      {
        string username = it->second.usernames[i];
        sockaddr_in connection= getUserByName(users,username);
        sendto(socket, &send_packet, sizeof send_packet, 0, (const sockaddr *)&connection,sizeof connection);
      }
    }
  }
}

void sendList(sockaddr_in connection, map<string,vector<channelcontents> > channels, int socket)
{
  struct text_list packet[channels.size()+8];
  packet->txt_type = TXT_LIST;
  packet->txt_nchannels = channels.size();
  int index = 0;
  for (map<string, vector<channelcontents> >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    cpString(it->first,packet->txt_channels[index].ch_channel,sizeof packet->txt_channels[index].ch_channel);
    index++;
  }
  sendto(socket,&packet,sizeof packet,0,(const sockaddr *)&connection,sizeof connection);
}

void sendWho(sockaddr_in connection, int socket, request_who *w_packet, map<string, vector<channelcontents> > channels)
{
  for (map<string, vector<channelcontents> >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    if (it->first==w_packet->req_channel)
    {
      //the right channel, construct the packet and send to person
      struct text_who packet[it->second.usernames.size()+40];
      packet->txt_type = TXT_WHO;
      cpString(it->first,packet->txt_channel,sizeof packet->txt_channel);
      packet->txt_nusernames = it->second.usernames.size();
      for (int i = 0; i < (int)it->second.usernames.size(); i++)
      {
        cpString(it->second.usernames[i],packet->txt_users[i].us_username,sizeof packet->txt_users[i].us_username);
      }
      sendto(socket,&packet,sizeof packet,0,(const sockaddr *)&connection, sizeof connection);
      return;
    }
  }
  sendError(connection,socket,"Requested channel does not exist");
}



int main(int argc, char *argv[]) {
  //Server must take variable number of arguments, in additional to the first two currently used, takes IP Addresses and Port numbers of additional servers
  //server must also keep track of which adjacent servers are subscribed to a channel
  map <string, struct sockaddr_in> users;
  map <string,vector <channelcontents> > channels;
  vector <c_server> servers;
  const char *host_name;
  int host_port;
  int my_socket;
  bool firstpacket = true;
  bool secondpacket = false;
  struct sockaddr_in my_server;
    //server takes two arguments repeatedly
        //host_address port_number
    if (argc<3)
      myError("Wrong number of arguments");
    if (argc % 2 == 0)
      myError("Wrong number of arguments");
    if (strcmp("localhost",argv[1]) == 0)
      host_name = LOCALHOST;
    else
      host_name = argv[1];
    host_port = strtol(argv[2],NULL,0);

    for (int i = 3; i < argc; i+=2)
    {
      int p;
      const char * hn;
      c_server new_server;
      new_server.server.sin_family = AF_INET;
      new_server.server.sin_addr.s_addr = htons(INADDR_ANY);
      p = strtol(argv[i+1],NULL,0);
      new_server.server.sin_port = htons(p);
      servers.push_back(new_server);
    }

    my_socket = socket(PF_INET,SOCK_DGRAM,0);

    //binding the socket
    my_server.sin_family = AF_INET;
    my_server.sin_addr.s_addr = htons(INADDR_ANY);

    my_server.sin_port = htons(host_port);
    bind(my_socket, (struct sockaddr *) &my_server, sizeof(my_server));

    // cerr<"Sleeping 5 seconds before connecting to other servers\n";
    // sleep(5);
    //Wait until all the other servers are connected, otherwise my hacky thing breaks
    cerr<<"This is server port " << my_server.sin_port<<"\n"<<"connected to \n";
      for (int i=0; i < (int)servers.size();i++)
      {
        int sock = socket(PF_INET,SOCK_DGRAM,0);
        int err = connect(sock, (struct sockaddr*)&servers[i], sizeof servers[i]);
        servers[i].socket = sock;
        cerr<<"port "<<servers[i].server.sin_port;
      }

//should be connected to all the servers, hopefully nothing hacky has to be done

      string hacky_name;

    while (1) {
      struct request u_packet[100];
      struct sockaddr_in client_addr;

      int addrlen;
      //server running, wait for a packet to arrive
      recvfrom(my_socket,u_packet,100,0,(struct sockaddr *) &client_addr,(socklen_t *)&addrlen);
      if (u_packet->req_type > 7)
      {
        //s2s packet
        //will need to fix hacky method so joins don't send a join back to the server that sent it
      }
      else{
        if (secondpacket) {secondpacket = false; users.clear(); users.insert(make_pair(hacky_name,client_addr));}
        if (firstpacket) { firstpacket = false; secondpacket = true;hacky_name = ((request_login *)u_packet)->req_username;}
      }
      //cerr<"I RECEIVED A PACKET\n";
      //fprintf(stderr,"I RECEIVED A PACKET");
      //output debugging whenever receiving message from client
        //[channel][user][message]

      //if server receives message from someone not logged in, ignore.

      //then I need to parse the packet
      if (u_packet->req_type == 0)
      {
        logIn((request_login *) u_packet, client_addr, &users);
      }
      else if ((u_packet->req_type == 1) && (loggedIn(client_addr,users)))
      {
        //deal with logout request
        outputMSG(my_server,client_addr,"recv Request Logout");
        logout(getUser(users,client_addr),&users,&channels);
        //whenever a channel has no users, it's deleted
      }
      else if ((u_packet->req_type == 2) && (loggedIn(client_addr,users)))
      {
        //deal with join request
        outputWithChannel(my_server,client_addr,"recv Request Join",((request_join *)u_packet)->req_channel);
        join((request_join *) u_packet, getUser(users,client_addr), &channels, servers,my_server );
      }
      else if (u_packet->req_type == 8)
      {
        //deal with s2s join request

        outputWithChannel(my_server,client_addr,"recv S2S Join ",((s2s_join *)u_packet)->s2s_channel);
        joinS2S((s2s_join *)u_packet,&channels,servers,my_server);
      }
      else if ((u_packet->req_type == 3) &&  (loggedIn(client_addr,users)))
      {
        outputWithChannel(my_server,client_addr,"recv Request Leave",((request_leave *)u_packet)->req_channel);
        leave(getUser(users,client_addr), ((request_leave *)u_packet)->req_channel ,&channels);
        //deal with leave request
        //whenever a channel has no users, it's deleted
      }
      else if ((u_packet->req_type == 4) &&  (loggedIn(client_addr,users)))
      {
        outputASay(my_server,client_addr,"recv Request Say", getUser(users,client_addr), ((request_say *)u_packet)->req_channel, ((request_say *)u_packet)->req_text);
        //deal with say request
        say((request_say *) u_packet, getUser(users,client_addr), users,channels,my_socket);
      }
      else if ((u_packet->req_type == 5) &&  (loggedIn(client_addr,users)))
      {
        outputMSG(my_server,client_addr,"recv Request List");
        sendList(client_addr, channels,my_socket);
      }
      else if ((u_packet->req_type == 6) &&  (loggedIn(client_addr,users)))
      {
        //deal with who request
        outputWithChannel(my_server,client_addr,"recv Request Who",((request_who *)u_packet)->req_channel);
        cerr<<getUser(users,client_addr)<<"gets a list of users in "<<((request_who *)u_packet)->req_channel<<"\n";
        sendWho(client_addr,my_socket,(request_who *)u_packet,channels);
        //sendWho(client_addr,   my_socket,    ((request_who *)u_packet)->req_channel    ,channels);
      }
      else if (loggedIn(client_addr,users))
      {
        //send an error
      }


    //  dListUsers(users);
      dListChannels(channels);
    }

    //server delivers messages from a user X to all users on X's active channel
         //must keep track of individual users and channels theyve subbed to
         //also must track each channel and subbed users on it


}
