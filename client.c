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
  char* host_name;
  char* host_port;
  char* username;
  char* active_channel;
  int h_socket,err;
  struct sockaddr_un server;
  //takes three command line arguments
   //server_host_name server_listening_port_number username

   if (argc != 4)
     perror("wrong number of arguments");
   else if (sizeof(argv[3]) > USERNAME_MAX)
    perror("Username too long");
   else{
     host_name = argv[1];
     host_port = argv[2];
     username = argv[3];

     fprintf(stderr, "%s logging in on port %s", username,host_port);

     //create the socket
    h_socket = socket(PF_UNIX,SOCK_DGRAM,0);

    //create the server structure
    server.sun_family = AF_UNIX;
    strcpy(server.sun_path, host_name);

    //connect/bind to the server, something
    err = connect(h_socket, &server, sizeof server);

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
