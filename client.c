#include <stdio.h>
#include "duckchat.h"
#include <sys/socket.h> // Core BSD socket functions and data structures
#include <netinet/in.h> //AF_INET and AF_INET6 address families
#include <sys/un.h> //AF_UNIX address family. Used for local communication between programs running on the same computer
#include <arpa/inet.h> //Functions for manipulating numeric IP addresses.
#include <netdb.h> //Functions for translating protocol names and host names into numeric addresses
#include <stdlib.h>
#include "raw.h"
#include <iostream>
#include <string>
//#include "raw.c"

const int DEBUG = 4;
char* active_channel;
char* username;

void myError(const char *msg)
{
  perror(msg);
  exit(-1);
}

void debug (const char *msg, int priority)
{
  int i;
  if (priority <= DEBUG) {
    for (i = 0; i < priority; i++) fprintf(stderr," ");
    fprintf(stderr, "%s\n",msg);
  }
}

void debugn(const char *msg, int n, int priority)
{
  int i;
  if (priority <= DEBUG){
    for (i = 0; i < priority; i++) fprintf(stderr, " ");
    fprintf(stderr, "%s%d\n",msg,n);
  }
}

int sendJoin(int socket, const char *channel)
{
  int err;
  struct request_join p_join;
  p_join.req_type = REQ_JOIN;
  strcpy(p_join.req_channel,channel);
  err = send(socket, &p_join, sizeof p_join,0);
  if (err < 0) { return err; }
  active_channel = (char *) channel;
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
    {active_channel = NULL;}
  return 0;
}

int sendSay(int socket, const char *channel, const char *msg)
{
  debug("EVENTUALLY WILL SEND SAY REQUEST",1);
  //check if active channel
  return 0;
}

int sendList(int socket)
{
  debug("EVENTUALLY WILL SEND LIST REQUEST",1);
  return 0;
}

int sendWho(int socket, const char *channel)
{
  debug("EVENTUALLY WILL SEND A WHO REQUEST",1);
  return 0;
}

int switchActive(int socket, const char*channel)
{
  debug("EVENTUALLY WILL SWITCH ACTIVE CHANNEL",1);
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


int main(int argc, char *argv[]) {
  const char* host_name;
  char* host_port;
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
     host_port = argv[2];
     username = argv[3];

     fprintf(stderr, "%s logging in on port %s\n", username,host_port);

     //create the socket
    h_socket = socket(PF_INET,SOCK_DGRAM,0);

    //create the server structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(host_name);
    server.sin_port = htons(8546);

    //connect to the server
    err = connect(h_socket, (struct sockaddr*) &server, sizeof server);
    if (err < 0)
      myError("ERROR running connect()");
    else{
      debug("Connection successful",1);
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

    //
    // //Wait for a response packet
    // char buffer[128];
    // recv(h_socket,buffer,sizeof buffer, 0);
    //
    // //Now that I have a response packet, create a txt structure to look at the type
    // struct text login_r;
    // login_r.txt_type = buffer[0];
    //
    //

    //I don't think this is strictly neccessary, less important than other things
    //raw_mode();
    //atexit(cooked_mode);

    while (connected){
          char user_in[SAY_MAX+1];
          char *p_user_in;
          //then it loops through providing the user a promp

          fgets(user_in,SAY_MAX,stdin);

          //get rid of the trailing newline
          p_user_in = strtok(user_in,"\n");

          if (strncmp(user_in,"/",1)==0)
          {
              debug("It's a command!",8);
              int i = parseCommand(h_socket,user_in);
              if (i == 1) { connected = 0;}
          }
          else
          //otherwise call say(user_in)
          {
            debug("Saying a thing",8);
            sendSay(h_socket,active_channel,user_in);
          }
    };

  //  close(h_socket) do I have to do this?
   }

   //then it loops through providing the user a prompt
    //when user hits enter
      //if it's text, text is sent to server (using "say request" message)
         //sent to the "active channel" for the user
         // switch and join change the active channel (kept track of by client)
      //if first character is a / interpreted as command
       // /exit: logout and exit the client
       // /join [channel]: Join (subscribe in) named channel, create if nonexistant
       // /leave [channel]: Leave the named channel
         //if leaving active channel, discard typed text until switch to active channel
       // /list: list the names of all channels
       // /who [channel]: List the users who are on the named channel
       // /switch [channel]: Switch to an existing channel joined by user
         //give error if user switches to a channel not joined

  //when client receives text from server display in following format
  // [channel][username]: text
  //print backspace characters (\b) to erase prompt/anything local user has typed
  //after text is printed, the client should redisplay the prompt and user input
  // have to keep track of user's input as it is being typed

}
