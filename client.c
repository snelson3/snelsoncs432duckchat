#include <stdio.h>
#include "duckchat.h"
#include <sys/socket.h> // Core BSD socket functions and data structures
#include <netinet/in.h> //AF_INET and AF_INET6 address families
#include <sys/un.h> //AF_UNIX address family. Used for local communication between programs running on the same computer
#include <arpa/inet.h> //Functions for manipulating numeric IP addresses.
#include <netdb.h> //Functions for translating protocol names and host names into numeric addresses

const int DEBUG = 5;

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

int main(int argc, char *argv[]) {
  const char* host_name;
  char* host_port;
  char* username;
  int h_socket,err;
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
    server.sin_port = 1234;

    //connect to the server
    err = connect(h_socket, (struct sockaddr*) &server, sizeof server);
    if (err < 0)
      perror("ERROR running connect()");
    else
      debug("Connection successful",1);

    //now I create a login packet, and send it
    struct request_login p_login;
    p_login.req_type = 0;
    strcpy(p_login.req_username,username);

    send(h_socket, p_login, sizeof p_login, 0);

    //Wait for a response packet

    char buffer[64];
    recv(h_socket,buffer,sizeof buffer, 0);

    debug(buffer,2);


    //make active channel Common
   }

   //on startup
   //client automatically connects to chat server
   //joins channel "Common"

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
