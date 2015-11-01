#include <stdio.h>

const int DEBUG = 5;

void debug (char *msg, int priority)
{
  int i;
  if (priority <= DEBUG) {
    for (i = 0; i < priority; i++) fprintf(stderr," ");
    fprintf(stderr, "%s\n",msg);
  }
}

void debugn(char *msg, int n, int priority)
{
  int i;
  if (priority <= DEBUG){
    for (i = 0; i < priority; i++) fprintf(stderr, " ");
    fprintf(stderr, "%s%d\n",msg,n);
  }
}

int main(int argc, char *argv[]) {
  //takes three command line arguments
   //server_host_name server_listening_port_number username

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
