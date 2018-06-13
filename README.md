# Simple-Socket [![Build Status](https://travis-ci.org/prince-chrismc/clsocket.svg?branch=master)](https://travis-ci.org/prince-chrismc/clsocket)
This ~fork~ repository aims to have the original library compile and work reliably using modern c++ compilers ( MSVC 15.7 / GCC 7.3 ) with a focus on the async and multicast functionality.

### Table of Contents
1. [History](#History)
2. [Building and Installing](Building-and-Installing)
3. [Class Overview](#Class-Overview)
4. [Examples](#Examples)

## History
Written by Mark Carrier to provide a mechanism for writing cross platform socket code. This library was originally written to only support blocking TCP sockets. Over the years it has been extended to support UDP and RAW sockets as well with many contribution from the Dwarf Fortress community. This library supports:
* Cross platform abstraction
   * Windows
   * Linux
   * Mac OSX
* Multiple IO modes
   * sychronious
   * asychronious
* Supports TCP Streams, UDP Datagrams, Raw Sockets
* Thread Safe and Signal Safe

The library's original release notes can be found [here](https://github.com/DFHack/clsocket/blob/master/ReleaseNotes) for more details.

## Building and Installing	
This is a very small library, it iss very easy to build and configure and requires no third-party support. To build and install, use CMake to generate the files required for your platform and execute the appropriate build command or procedure.

## Class Overview
Network communications via sockets can be abstracted into two categories of functionality; the active socket and the passive socket. The active socket object initiates a connection with a known host, whereas the passive socket object waits (or listens) for inbound requests for communication. The functionality of both objects is identical as far as sending and receiving data. This library makes distinction between the two objects because the operations for constructing and destructing the two are different.

This library is different from other socket libraries which define TCP sockets, UDP sockets, HTTP sockets, etc. The reason is the operations required for TCP, UDP, and RAW network communication is identical from a logical stand point. Thus a program could initially be written employing TCP streams, and then at some future point it could be discovered that UDP datagrams would satisify the solution. Changing between the two transport protocols would only require changing how the object is instantiated. The remaining code would in theory require minimal to no changes.

This library avoids abstractions like HTTP socket, or SMTP socket, soley because this type of object mixes the application and the transport layer. These types of abstractions can be created using this library as a base class.

The simple socket library is comprised of two class which can be used to represent all socket communications.
* Active Socket Class
* Passive Socket Class 

## Examples
When operating on a socket object most methods will return true or false make validation very clean

There are two application specific examples:
- an [HTTP client](https://github.com/prince-chrismc/clsocket/tree/master/examples/HttpRequest)
- a [DNS client](https://github.com/prince-chrismc/clsocket/tree/master/examples/Dns)
which may help with specific details for each.

##### Simple Active Socket
As mentioned previously the active socket (CActiveSocket) is used to initiate a connections with a server on some known port. 

> So you want to connect to an existing server...
>
> How do you do it?

There are many ways using the existing Berkley Socket API, but the goal of this class is to remove the many calls and man page lookups and replace them with clear, concise set of methods which allow a developer to focus on the logic of network programming.

The following code will connect to a DAYTIME server on port 13, query for the current time, and close the socket.

```cpp
#include <string>
#include "ActiveSocket.h"       // Include header for active socket object definition

int main(int argc, char** argv)
{
    CActiveSocket socket;       // Instantiate active socket object (defaults to TCP).
    std::string   time;

    // Initialize our socket object 
    socket.Initialize();

    if (socket.Open("time-C.timefreq.bldrdoc.gov", 13))
    {
        // Send a requtest the server requesting the current time.
        if (socket.Send((const uint8 *)"\n", 1))
        {
            // Receive response from the server.
            socket.Receive(49);
            time.assign((const char*)socket.GetData());
            printf("%s\n", time);

            // Close the connection.
            socket.Close();
        }
    }

    return 1;
}
```

You can see that the amount of code required to an object for network communciation is very small and simple.

##### Simple Passive Socket
As mentioned previously the passive socket (CPassiveSocket) is used to listen for incomming connections on some defines port. 

> Now you want to build a server...
>
> How do you do it?

For a practical test lets build an echo server. The server will listen on port 6789 an repsond back with what ever has been sent to the server.

```cpp
#include "PassiveSocket.h"       // Include header for active socket object definition

#define MAX_PACKET 4096 

int main(int argc, char **argv)
{
    CPassiveSocket socket;
    CActiveSocket *pClient = NULL;

    // Initialize our socket object 
    socket.Initialize();
    socket.Listen("127.0.0.1", 6789);
    while (true)
    {
        if ((pClient = socket.Accept()) != NULL)
        {
            // Receive request from the client.
            if (pClient->Receive(MAX_PACKET))
            {
                // Send response to client and close connection to the client.
                pClient->Send( pClient->GetData(), pClient->GetBytesReceived() );
                pClient->Close();
            }

            delete pClient;
        }
    }
    socket.Close();

    return 1;
}
```
