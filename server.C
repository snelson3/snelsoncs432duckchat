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
#include <uuid/uuid.h>

using namespace std;

/* Server struct, used to keep track of IPs/Host names for the servers*/

void cpString(const string& input, char *dst, size_t dst_size)
{
  strncpy(dst, input.c_str(),dst_size - 1);
  dst[dst_size -1] = '\0';
}

struct c_server {
  sockaddr_in server;
  int socket;
};

struct channelcontents {
  vector<struct c_server> servers;
  vector<string> usernames;
  vector<string> ids;
};

void myError(const char *msg)
{
  perror(msg);
  exit(-1);
}

void randomID(char * id)
{
  // uuid id;
  // uuid id_ns;
  // char *str;
  //
  // id_ns.load("ns:URL");
  // id.make(UUID_MAKE_V3, &id_ns, "/dev/urandom");
  // str = id.string();
  // return str;
//  cerr<<"GENERATING A RAND VAL ";
  uuid_t random;
  uuid_generate(random);
//  cerr<<"random               "<<random<<"\n";
  //cerr<<"GENERATING A RAND VAL random            "<<random<<"\n";
  strcpy(id,(char *) random);
}

bool isRecentID(string id, vector<string> recent)
{
//  cerr<<"size is "<<recent.size()<<"COMPARINGCOMPARINGCOMPARING\n"<<id<<"\n"<<"with \n";
 for (int i = 0; i < (int)recent.size(); i++) cerr<<recent[i]<<"\n";
  for (int i = 0; i < (int)recent.size(); i++)
  {
    //cerr<<recent[i]<<"\n";
    //char test[SAY_ID_MAX];
    //cpString(recent[i],test,sizeof test);
    if (id ==recent[i]) return true;
  }
  return false;
}

void outputMSG(struct sockaddr_in host, struct sockaddr_in client,const char * packetmsg)
{
  cerr<<"127.0.0.1:"<<host.sin_port<<" 127.0.0.1:"<<client.sin_port<<" "<<packetmsg<<"\n";
}

void outputWithChannel(struct sockaddr_in host, struct sockaddr_in client, const char * packetmsg, const char * channelname)
{
  cerr<<"127.0.0.1:"<<host.sin_port<<" 127.0.0.1:"<<client.sin_port<<" "<<packetmsg<<" "<<channelname<<"\n";
}

void outputWithChannelS(struct sockaddr_in host, struct sockaddr_in client, const char * packetmsg, string channelname)
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

void dListChannels(map<string,struct channelcontents > channels)
{
  cerr<<"Printing list of channels for DEBUG purposes\n";
  for ( map<string, struct channelcontents >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    cerr<<it->first<<":";
    for (int i=0; i < (int)it->second.usernames.size();i++)
    {
      cerr<<it->second.usernames[i]<<",";
    }
    cerr<<"\n";
  }
}

int getClientIndex(sockaddr_in client, vector<c_server> servers)
{
  for (int i = 0; i < (int)servers.size(); i++)
  {
    if (client.sin_port==servers[i].server.sin_port)
    {
      return i;
    }
  }
  return -1;
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

void leave(string user, string channel, map<string,struct channelcontents > *channels)
{
  //find the channel
  //remove the user from it
  //if theres nobody in the channel, remove it from the map
  for (map<string,struct channelcontents >::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    if (channel == it->first)
    {
      int n;
      n = getUserIndex(user,it->second.usernames);
      if (n>-1) it->second.usernames.erase(it->second.usernames.begin()+n);
      if (it->second.usernames.size() == 0)
      {
        if (it->second.servers.size() == 0)
          {
            channels->erase(it->first);
          }
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

void logout(string user, map<string,struct sockaddr_in> *users, map<string,struct channelcontents > *channels)
{
  //first go through each channel and remove user from the channel's logged in list (make sure to run checkEmpty, or call the Leave function or something)
  for (map<string,struct channelcontents >::iterator it = channels->begin(); it!=channels->end(); it++)
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

int removeServerIndex(vector<c_server> servers, sockaddr_in client)
{
  for (int i = 0; i < (int)servers.size(); i++)
  {
    if (servers[i].server.sin_port == client.sin_port)
    {
      return i;
    }
  }
  return -1;
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


void s2sLeave(const char * channel, int socket, sockaddr_in connection)
{
  int err;
  struct s2s_leave p_leave;
  p_leave.req_type = 9;
  strcpy(p_leave.s2s_channel,channel);
  err = sendto(socket,&p_leave, sizeof p_leave, 0, (const sockaddr *) &connection, sizeof connection);
}

void s2sLeaveS(string channel, int socket, sockaddr_in connection)
{
  int err;
  struct s2s_leave p_leave;
  p_leave.req_type = 9;
  char ch[CHANNEL_MAX];
  cpString(channel,p_leave.s2s_channel,sizeof p_leave.s2s_channel);
  // strcpy(p_leave.s2s_channel,channel);
  err = sendto(socket,&p_leave, sizeof p_leave, 0, (const sockaddr *) &connection, sizeof connection);
}

void leaveS2S(map<string,struct channelcontents > * channels, const char * channel, sockaddr_in client)
{
  //remove from channels server list
  for (map<string,struct channelcontents>::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    if (it->first == channel)
    {
      int n;
      n = getClientIndex(client,it->second.servers);
      it->second.servers.erase(it->second.servers.begin()+n);
        //if size of channel server list is 0
      if (it->second.servers.size() == 0)
      {
          //and 0 Clients
        if (it->second.usernames.size() == 0)
        {
            //delete channel
          channels->erase(it->first);
        }
      }
    }
  }
}

void s2sJoin(int socket, const char * channel,sockaddr_in connection)
{
  int err;
  struct s2s_join p_join;
  p_join.req_type = 8;
  strcpy(p_join.s2s_channel,channel);
  //err = send(server_socket, &p_join, sizeof p_join, 0);
  err = sendto(socket,&p_join, sizeof p_join, 0, (const sockaddr *) &connection, sizeof connection);
  if (err < 0) { cerr<<"Error sending S2S Join\n";}
}

void joinS2S(s2s_join * packet, map<string,struct channelcontents > * channels, vector<c_server> servers, sockaddr_in host, sockaddr_in client, int socket)
{
  bool ex = false;
  for (map<string,struct channelcontents >::iterator it = channels->begin(); it!=channels->end(); it++)
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
  //cerr<<"THIS IS THE CLIENT SERVERS SIN_PORT "<<client.sin_port<<"\n";
  for (int i = 0; i < (int)servers.size(); i++)
  {
    cc.servers.push_back(servers[i]);
    if (client.sin_port != servers[i].server.sin_port )
    {
      outputWithChannel(host,cc.servers[i].server,"send S2S Join", packet->s2s_channel);
      s2sJoin(socket,packet->s2s_channel,cc.servers[i].server);
    }
  }
  //cc.servers.erase(removeServerIndex(cc.servers, client));
  channels->insert(make_pair(packet->s2s_channel,cc));
  // for (int i = 0; i < (int)cc.servers.size(); i++)
  // {
  //   outputWithChannel(host,cc.servers[i].server,"send S2S Join", packet->s2s_channel);
  //   s2sJoin(socket,packet->s2s_channel,cc.servers[i].server);
  // }
}

void join(request_join * packet,string user,map<string,struct channelcontents > *channels, vector<c_server> servers,sockaddr_in host,int socket)
{
  //whenever a user joins a nonexistent channel, it's created
  bool ex = false;
  for (map<string,struct channelcontents >::iterator it = channels->begin(); it!=channels->end(); it++)
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
    cc.servers = servers;
    joined_users.push_back(user);
    cc.usernames = joined_users;
    //channel doesn't exist, create channel and add user to channel, also S2S join
    channels->insert(make_pair(packet->req_channel,cc));
    for (int i = 0; i < (int)cc.servers.size(); i++)
    {
      outputWithChannel(host,cc.servers[i].server,"send S2S Join ", packet->req_channel);
      s2sJoin(socket, packet->req_channel,cc.servers[i].server);
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

void sayS2S(s2s_say *packet, map<string,struct sockaddr_in> users, map<string, struct channelcontents > *channels, int socket,sockaddr_in host, sockaddr_in client)
{

  //for all users in channel, parse packet and send to them
  struct text_say send_packet;
  send_packet.txt_type = TXT_SAY;
  strcpy(send_packet.txt_channel,packet->s2s_channel);
  strcpy(send_packet.txt_username, packet->s2s_username);
  strcpy(send_packet.txt_text, packet->s2s_text);
  for (map<string,struct channelcontents>::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    if (it->first == packet->s2s_channel)
    {
      //this is the right channel, iterate through users to send the message multiple times
      cerr<<"IDS\n\n\n";
      // for (int i = 0; i < (int)it->second.ids.size();i++) cerr<<it->second.ids[i]<<"\n";
      // if (isRecentID(it->second.ids[0],it->second.ids)) cerr<<"\nLooks like it's working right\n";
      //check if ID matches recent list
      if (isRecentID((string)packet->id,it->second.ids))
      {
         //if so return/leave
         outputWithChannelS(host,client,"send S2S Leave",it->first);
         s2sLeaveS(it->first,socket,client);
         return;
      }

      if ((int)it->second.usernames.size() == 0)
      {
        if ((int)it->second.servers.size() == 1)
        {
          outputWithChannelS(host,client,"send S2S Leave",it->first);
          s2sLeaveS(it->first,socket,client);
          channels->erase(it->first);
          return;
        }
      }
      for (int i = 0; i < (int)it->second.usernames.size(); i++)
      {
        string username = it->second.usernames[i];
        sockaddr_in connection = getUserByName(users,username);
        sendto(socket, &send_packet, sizeof send_packet, 0, (const sockaddr *)&connection, sizeof connection);

      }
      char id[SAY_ID_MAX];
      strcpy(id,packet->id);
      //add the unique id to the list of recently used ID's
      it->second.ids.push_back((string) id);

      //forward packet to other servers
      for (int i = 0; i < (int) it->second.servers.size(); i++)
      {
        if (client.sin_port != it->second.servers[i].server.sin_port)
        {
          int err;
          outputASay(host,it->second.servers[i].server,"send S2S Say", packet->s2s_username, packet->s2s_channel, packet->s2s_text);
          err = sendto(socket,packet,sizeof *packet, 0, (const sockaddr *) &it->second.servers[i].server, sizeof it->second.servers[i].server);
        }
      }
    }
  }


}

void say(request_say *packet,string user,map<string,struct sockaddr_in> users,map<string,struct channelcontents > *channels,int socket,sockaddr_in host)
{

  struct text_say send_packet;
  send_packet.txt_type = TXT_SAY;
  strcpy(send_packet.txt_channel,packet->req_channel);
  cpString(user,send_packet.txt_username,sizeof send_packet.txt_username);
  strcpy(send_packet.txt_text,packet->req_text);
  for (map<string,struct channelcontents >::iterator it = channels->begin(); it!=channels->end(); it++)
  {
    if (it->first==packet->req_channel)
    {
      //this is the right channel, iterate through the users to send the message multiple times
      for (int i = 0; i < (int)it->second.usernames.size(); i++)
      {
        string username = it->second.usernames[i];
        sockaddr_in connection= getUserByName(users,username);
        sendto(socket, &send_packet, sizeof send_packet, 0, (const sockaddr *)&connection,sizeof connection);
      }
      //construct the s2s_say packet
      struct s2s_say spacket;
      spacket.req_type = 10;
      char id[SAY_ID_MAX];
      randomID(id);
      strcpy(spacket.id,id);
      cpString(user,spacket.s2s_username,sizeof spacket.s2s_username);
      strcpy(spacket.s2s_channel,packet->req_channel);
      strcpy(spacket.s2s_text, packet->req_text);

      //add the unique id to the list of recently used id's
      it->second.ids.push_back((string) id);
      //send the packet to the adjacent servers
      for (int i = 0; i < (int)it->second.servers.size(); i++)
      {
        int err;
        cerr<<"UUID "<<sizeof spacket.id<<"\n";

        outputASay(host,it->second.servers[i].server,"send S2S Say", spacket.s2s_username, spacket.s2s_channel, spacket.s2s_text);
        err = sendto(socket,&spacket,sizeof spacket,0, (const sockaddr *) &it->second.servers[i].server, sizeof it->second.servers[i].server);
        cerr<<"ERR NO "<<err<<"\n";
      }
    }
  }
  cerr<<"DONE WITH THE SAY\n";
}

void sendList(sockaddr_in connection, map<string,struct channelcontents > channels, int socket)
{
  struct text_list packet[channels.size()+8];
  packet->txt_type = TXT_LIST;
  packet->txt_nchannels = channels.size();
  int index = 0;
  for (map<string, struct channelcontents >::iterator it = channels.begin(); it!=channels.end(); it++)
  {
    cpString(it->first,packet->txt_channels[index].ch_channel,sizeof packet->txt_channels[index].ch_channel);
    index++;
  }
  sendto(socket,&packet,sizeof packet,0,(const sockaddr *)&connection,sizeof connection);
}

void sendWho(sockaddr_in connection, int socket, request_who *w_packet, map<string, struct channelcontents > channels)
{
  for (map<string, struct channelcontents >::iterator it = channels.begin(); it!=channels.end(); it++)
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
  map <string,struct channelcontents > channels;
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
    // cerr<<"This is server port " << my_server.sin_port<<"\n"<<"connected to \n";
    //   for (int i=0; i < (int)servers.size();i++)
    //   {
    //     int sock = socket(PF_INET,SOCK_DGRAM,0);
    //     int err = connect(sock, (struct sockaddr*)&servers[i].server, sizeof servers[i].server);
    //     servers[i].socket = sock;
    //     cerr<<"port "<<servers[i].server.sin_port<<"\n";
    //   }

//should be connected to all the servers, hopefully nothing hacky has to be done

      string hacky_name;

    while (1) {
      cerr<<"WHILE LOOP INTERATION\n";
      struct request u_packet[100];
      struct sockaddr_in client_addr;

      socklen_t addrlen;
      addrlen = sizeof(client_addr);
      //server running, wait for a packet to arrive
      recvfrom(my_socket,u_packet,100,0,(struct sockaddr *) &client_addr,&addrlen);
      //cerr<<"Received Clients port "<<client_addr.sin_port<<"\n";
      // if (u_packet->req_type > 7)
      // {
      //   if (secondpacket) {secondpacket = false;}
      //   if (firstpacket) { firstpacket = false; secondpacket = true;}
      //   //s2s packet, has to be a s2s_join
      //   //will need to fix hacky method so joins don't send a join back to the server that sent it
      // }
      // else{
      //   if (secondpacket) {secondpacket = false; users.clear(); users.insert(make_pair(hacky_name,client_addr));}
      //   if (firstpacket) { firstpacket = false; secondpacket = true;hacky_name = ((request_login *)u_packet)->req_username;}
      // }
      cerr<"I RECEIVED A PACKET\n";
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
        join((request_join *) u_packet, getUser(users,client_addr), &channels, servers,my_server,my_socket );
      }
      else if (u_packet->req_type == 8)
      {
        //deal with s2s join request
        outputWithChannel(my_server,client_addr,"recv S2S Join",((s2s_join *)u_packet)->s2s_channel);
        joinS2S((s2s_join *)u_packet,&channels,servers,my_server, client_addr,my_socket);
      }
      else if ((u_packet->req_type == 3) &&  (loggedIn(client_addr,users)))
      {
        outputWithChannel(my_server,client_addr,"recv Request Leave",((request_leave *)u_packet)->req_channel);
        leave(getUser(users,client_addr), ((request_leave *)u_packet)->req_channel ,&channels);
        //deal with leave request
        //whenever a channel has no users, it's deleted
      }
      else if (u_packet->req_type == 9)
      {
        //deal with s2s leave request
        outputWithChannel(my_server,client_addr,"recv S2S Leave",((s2s_leave *)u_packet)->s2s_channel);
        leaveS2S(&channels,((s2s_leave *)u_packet)->s2s_channel,client_addr);
      }
      else if ((u_packet->req_type == 4) &&  (loggedIn(client_addr,users)))
      {
        outputASay(my_server,client_addr,"recv Request Say", getUser(users,client_addr), ((request_say *)u_packet)->req_channel, ((request_say *)u_packet)->req_text);
        //deal with say request
        say((request_say *) u_packet, getUser(users,client_addr), users,&channels,my_socket,my_server);
      }
      else if (u_packet->req_type == 10)
      {
        //deal with s2s say request
        outputASay(my_server,client_addr,"recv S2S Say", (string)((s2s_say *)u_packet)->s2s_username,((s2s_say *)u_packet)->s2s_channel, ((s2s_say *)u_packet)->s2s_text);
        sayS2S(((s2s_say *)u_packet), users,&channels,my_socket,my_server,client_addr );
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
