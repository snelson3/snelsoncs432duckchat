#include <stdio.h>
#include "duckchat.h"
#include <sys/socket.h> // Core BSD socket functions and data structures
#include <netinet/in.h> //AF_INET and AF_INET6 address families
#include <sys/un.h> //AF_UNIX address family. Used for local communication between programs running on the same computer
#include <arpa/inet.h> //Functions for manipulating numeric IP addresses.
#include <netdb.h> //Functions for translating protocol names and host names into numeric addresses


int main(int argc, char *argv[]) {
    //server takes two arguments
        //host_address port_number

    //binds to an ip address, if localhost 127.0.0.1

    //output debugging whenever receiving message from client
      //[channel][user][message]

    //server delivers messages from a user X to all users on X's active channel
         //must keep track of individual users and channels theyve subbed to
         //also must track each channel and subbed users on it

    //whenever a channel has no users, it's deleted
    //whenever a user joins a nonexistent channel, it's created

    //if server receives message from someone not logged in, ignore.
}
