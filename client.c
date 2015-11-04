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

char *active_channel = new char[CHANNEL_MAX];
char* username = new char[USERNAME_MAX];

void myError(const char *msg)
{
  perror(msg);
  exit(-1);
}

int reportError(text_error *e_packet)
{
  fprintf(stderr,"Error: %s\n",e_packet->txt_error);
  return 0;
}

int sendJoin(int socket, const char *channel)
{
  int err;
  struct request_join p_join;
  p_join.req_type = REQ_JOIN;
  strcpy(p_join.req_channel,channel);
  err = send(socket, &p_join, sizeof p_join,0);
  if (err < 0) { return err; }
  strcpy(active_channel,channel);
  return err;
}

int sendLogout(int socket)
{
  int err;
  //now I create a logout packet, and send it
  struct request_logout p_logout;
  p_logout.req_type = REQ_LOGOUT;

  err = send(socket, &p_logout, sizeof p_logout, 0);
  if (err < 0) myError("Error sending logout packet");
  return 0;
}

int sendLeave(int socket, const char *channel)
{
  int err;
  struct request_leave p_leave;
  p_leave.req_type = REQ_LEAVE;
  strcpy(p_leave.req_channel,channel);
  err = send(socket, &p_leave, sizeof p_leave,0);
  if (err < 0) { myError("Error sending leave packet"); }
  if (strcmp(channel,active_channel)==0)
    {strcpy(active_channel,"");}
  return 0;
}

int sendSay(int socket, const char *msg)
{
  if (strcmp(active_channel,"")==0){
    fprintf(stderr,"No active channel, can't send message\n");
    return 0;
  }
  int err;
  struct request_say p_say;
  p_say.req_type = REQ_SAY;
  strcpy(p_say.req_channel,active_channel);
  strcpy(p_say.req_text,msg);
  err = send(socket, &p_say, sizeof p_say, 0);
  if (err < 0) { myError("Error sending say message"); }
  return 0;
}

int sendList(int socket)
{
  int err;
  struct request_list p_list;
  p_list.req_type = REQ_LIST;

  err = send(socket, &p_list, sizeof p_list, 0);
  if (err < 0) myError("Error sending list packet");

  struct text_list l_packet[9999];
  recv(socket,l_packet,sizeof l_packet, 0);
  if (l_packet->txt_type == TXT_ERROR){
    reportError((text_error *)l_packet);
  }
  else{
    fprintf(stderr,"Existing channels:\n");

    for (int i = 0; i < l_packet->txt_nchannels;i++){
      fprintf(stderr,"%s\n",l_packet->txt_channels[i].ch_channel);
    }
  }
  return 0;
}

int sendWho(int socket, const char *channel)
{
  int err;
  struct request_who p_who;
  p_who.req_type = REQ_WHO;
  strcpy(p_who.req_channel,channel);
  err = send(socket, &p_who, sizeof p_who,0);
  if (err < 0) { myError("Error sending who packet"); }

  struct text_who w_packet[9999];
  recv(socket,w_packet,sizeof w_packet, 0);
  if (w_packet->txt_type == TXT_ERROR)
  {
    reportError((text_error *)w_packet);
  }
  else
  {
    fprintf(stderr,"Users on channel %s\n", channel);

    for (int i = 0; i < w_packet->txt_nusernames;i++){
      fprintf(stderr,"%s\n",w_packet->txt_users[i].us_username);
  }}
  return 0;
}

int switchActive(int socket, const char*channel)
{
  //get a list of the users in that channel, and see if it has this user.
  int err;
  struct request_who p_who;
  p_who.req_type = REQ_WHO;
  strcpy(p_who.req_channel,channel);
  err = send(socket, &p_who, sizeof p_who, 0);
  if (err < 0) { myError("Error sending who packet");}

  struct text_who w_packet[9999];
  recv(socket,w_packet,sizeof w_packet,0);
  if (w_packet->txt_type == TXT_ERROR){
    reportError((text_error *)w_packet);
  }
  else

  {
  bool user_in_channel = false;


    for (int i = 0; i < w_packet->txt_nusernames; i++) {
      if (strcmp(username, w_packet->txt_users[i].us_username)==0)
      {
        user_in_channel = true;
      }
    }

    if (user_in_channel)
    {
      strcpy(active_channel,channel);
    }
    else
    {
      fprintf(stderr, "Can't switch to channel not joined\n");
    }
}
  return 0;
}

int parseCommand(int socket, const char*command)
{
  char *pC;
  pC = strtok((char *)command," ");

  if (strcmp(pC,"/exit")==0)
  {
    //logout request
    sendLogout(socket);
    return 1;
  }
  else if (strcmp(pC, "/join")==0)
  {
    //join request
    pC = strtok(NULL, " ");
    if (pC != NULL)
    {
     sendJoin(socket,pC);
    }
    else
    {
      fprintf(stderr,"No channel specified!\n");
    }
  }
  else if (strcmp(pC, "/leave")==0)
  {
    //leave request
    pC = strtok(NULL, " ");
    if (pC != NULL)
    {
     sendLeave(socket,pC);
    }
    else
    {
      fprintf(stderr,"No channel specified!\n");
    }
  }
  else if (strcmp(pC, "/list")==0)
  {
    //list request
    sendList(socket);
  }
  else if (strcmp(pC,"/who")==0)
  {
    //who request
    pC = strtok(NULL, " ");
    if (pC != NULL)
    {
     sendWho(socket,pC);
    }
    else
    {
      fprintf(stderr,"No channel specified!\n");
    }
  }
  else if (strcmp(pC, "/switch")==0)
  {
    //switch active channels
    pC = strtok(NULL, " ");
    if (pC != NULL)
    {
     switchActive(socket,pC);
    }
    else
    {
      fprintf(stderr,"No channel specified!\n");
    }
  }
  else{
    fprintf(stderr,"Bad Command\n");
  }
  return 0;
}

int reportSay(text_say *s_packet)
{
  fprintf(stderr,"[%s][%s]: %s\n",s_packet->txt_channel,s_packet->txt_username,s_packet->txt_text);
  return 0;
}

int parseServerPacket(int socket)
{
  struct text u_packet[PACKET_MAX];
  recv(socket,u_packet,sizeof u_packet, 0);
  if (u_packet->txt_type == TXT_SAY)
  {
    reportSay((text_say *)u_packet);
  }
  else if (u_packet->txt_type == TXT_ERROR)
  {
    reportError((text_error *)u_packet);
  }
  else perror("Incorrect txt_type\n");
  return 0;
}

int main(int argc, char *argv[]) {
  const char* host_name;
  int host_port;
  int h_socket,err;
  bool connected = false;
  struct sockaddr_in server;
  //takes three command line arguments
   //server_host_name server_listening_port_number username
   if (argc != 4)
     perror("wrong number of arguments");
   else if (sizeof(argv[3]) > USERNAME_MAX)
    perror("Username too long");
   else{
     if (strcmp("localhost",argv[1]) == 0)
        host_name = LOCALHOST;
     else
        host_name = argv[1];
     host_port = strtol(argv[2], NULL, 0);
     username = argv[3];

     //create the socket
    h_socket = socket(PF_INET,SOCK_DGRAM,0);

    //create the server structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(host_name);
    server.sin_port = htons(host_port);

    //connect to the server
    err = connect(h_socket, (struct sockaddr*) &server, sizeof server);
    if (err < 0)
      myError("ERROR running connect()");
    else{
      fprintf(stderr,"Connection successful\n");
      connected = true;
    }

    //now I create a login packet, and send it
    struct request_login p_login;
    p_login.req_type = REQ_LOGIN;
    strcpy(p_login.req_username,username);

    err = send(h_socket, &p_login, sizeof p_login, 0);
    if (err < 0) myError("Error sending login packet");

    //join channel Common
    err = sendJoin(h_socket, "Common");
    if (err < 0) myError("Error joining channel Common");

    fd_set readfds;

    while (connected){
          FD_ZERO(&readfds);

          FD_SET(h_socket,&readfds);
          FD_SET(STDIN_FILENO,&readfds);
          int n = fmax(STDIN_FILENO,h_socket)+1;
          char user_in[SAY_MAX+1];
          char *p_user_in = new char[SAY_MAX];

          select(n, &readfds,NULL,NULL,0);
            if (FD_ISSET(STDIN_FILENO,&readfds)){
              fgets(user_in,SAY_MAX,stdin);

              //get rid of the trailing newline
              p_user_in = strtok(user_in,"\n");

              if (strncmp(user_in,"/",1)==0)
              {
                  int i = parseCommand(h_socket,user_in);
                  if (i == 1) { connected = 0;}
              }
              else
              //otherwise call say(user_in)
              {
                sendSay(h_socket,user_in);
              }
              FD_CLR(STDIN_FILENO,&readfds);
            }

            if (FD_ISSET(h_socket,&readfds))
            {
              //a packet was sent from the server, parse itf
              parseServerPacket(h_socket);
              FD_CLR(STDIN_FILENO,&readfds);
            }
    };
   }
}
