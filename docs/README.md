# Simple-Socket [![Build Status](https://travis-ci.org/prince-chrismc/Simple-Socket.svg?branch=master)](https://travis-ci.org/prince-chrismc/Simple-Socket)[![Build status](https://ci.appveyor.com/api/projects/status/sqbvy884ybs8rtv7?svg=true)](https://ci.appveyor.com/project/prince-chrismc/simple-socket)[![MIT License](http://img.shields.io/badge/license-MIT-blue.svg?style=flat)](LICENSE)
This ~~fork~~ repository aims to have the original library compile and work reliably using modern c++ compilers ( MSVC 15.7 / GCC 7.3 / Clang 6.0 ) with a focus on
the async and multicast functionality.

### Table of Contents
1. [History](#history)
2. [Building and Installing](building-and-installing)
3. [Class Overview](#class-overview)
4. [Examples](#examples)

## History
Written by Mark Carrier to provide a mechanism for writing cross platform socket code. This library was originally written to only support blocking
TCP sockets. Over the years it has been extended to support UDP and RAW sockets as well with many contribution from the Dwarf Fortress community.
This library supports:
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
This is a very small library, it is very easy to build and configure and requires no third-party support.
To build and install, use CMake to generate the files required for your platform and execute the appropriate build command or procedure.

- Unix Systems: The command is `make` which produce a release and debug version depending on the `CMAKE_BUILD_TYPE` specified.
- Windows Systems: The usual MSVC files can be build through the IDE or via command line interface.

Installation can be achieved by adding the runing `make install` from your build folder.

## Class Overview
Network communications via sockets can be abstracted into two categories of functionality; the active socket and the passive socket.
The active socket object initiates a connection with a known host, whereas the passive socket object waits (or listens) for inbound requests for
communication. The functionality of both objects is identical as far as sending and receiving data. This library makes distinction between the two
objects because the operations for constructing and destructing the two are different.

This library is different from other socket libraries which define TCP sockets, UDP sockets, HTTP sockets, etc.
The reason is the operations required for TCP, UDP, and RAW network communication is identical from a logical stand point.
Thus a program could initially be written employing TCP streams, and then at some future point it could be discovered that UDP datagrams would
satisify the solution. Changing between the two transport protocols would only require changing how the object is instantiated. The remaining code
would in theory require minimal to no changes.

This library avoids abstractions like HTTP socket, or SMTP socket, soley because this type of object mixes the application and the transport layer.
These types of abstractions can be created using this library as a base class.

The simple socket library is comprised of two class which can be used to represent all socket communications.
* Active Socket Class
* Passive Socket Class

## Examples
When operating on a socket object most methods will return true or false make validation very clean, see [below](#Simple-Active-Socket) of details.

There are two application specific examples provided by this repository:
- an [HTTP client](https://github.com/prince-chrismc/clsocket/tree/master/examples/http)
- a [DNS client](https://github.com/prince-chrismc/clsocket/tree/master/examples/dns)

There are also general purpose examples for:
- a [Multicast Sender/Receiver](https://github.com/prince-chrismc/Simple-Socket/blob/master/examples/Multicast.cpp)
- an [Asynchronous Client/Server](https://github.com/prince-chrismc/Simple-Socket/blob/master/examples/RecvAsync.cpp)

##### Simple Active Socket
As mentioned previously the active socket ( `class CActiveSocket` ) is used to initiate a connections with a server on some known address and port.

> So you want to connect to an existing server...
>
> How do you do it?

There are many ways using the existing Berkley Socket API, but the goal of this class is to remove the many calls and man page lookups and replace
them with clear, concise set of methods which allow a developer to focus on the logic of network programming.

The following code will connect to a DAYTIME server on port 13, query for the current time, and close the socket.

```cpp
#include <string>
#include "ActiveSocket.h" // Include header for active socket object definition

int main( int argc, char** argv )
{
   CActiveSocket oSocket; // Instantiate active socket object (defaults to TCP).

   if( oSocket.Open( "time-C.timefreq.bldrdoc.gov", 13 ) ) // Attempt connection to known remote server
   {
      if( oSocket.Send( "\n" ) ) // Send a request the server for the current time.
      {
         if( oSocket.Receive( 48 ) ) // Receive response from the server.
            std::cout << oSocket.GetData() << std::endl;
         else
            std::cout << "Unable to obtain time!" << std::endl;
      }
   }

   return 1;
}

```
_Note:_ This example requires C++1z to compile for an example which is C++03 compilant see [here](https://github.com/prince-chrismc/Simple-Socket/blob/d7b1c5d3a8436cdbc60793701ffed4f1f504c754/examples/QueryDayTime.cpp)

You can see that the amount of code required to have an object perform network communciation is very small and simple.

##### Simple Passive Socket
As mentioned previously the passive socket ( `class CPassiveSocket` ) is used to listen for incomming connections on some defines port.

> Now you want to build a server...
>
> How do you do it?

For a practical test lets build an echo server. The server will listen on port 6789 an repsond back with what ever has been sent to the server.

```cpp

#include <future>
#include <chrono>
#include "PassiveSocket.h" // Include header for passive socket object definition

static constexpr const int32 MAX_PACKET = 4096;
using namespace std::chrono_literals;

int main( int argc, char** argv )
{
   CPassiveSocket oSocket;
   std::promise<void> oExitSignal;

   oSocket.Listen( "127.0.0.1", 6789 ); // Bind to local host on port 6789 to wait for incomming connections

   auto oRetval = std::async( std::launch::deferred, [ &oSocket, oExitEvent = oExitSignal.get_future() ]() {
         while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
         {
            std::unique_ptr<CActiveSocket> pClient;
            if( ( pClient = oSocket.Accept() ) != nullptr ) // Wait for an incomming connection
            {
               if( pClient->Receive( MAX_PACKET ) ) // Receive request from the client.
               {
                  // Send response to client and close connection to the client.
                  pClient->Send( reinterpret_cast<const uint8*>( pClient->GetData().c_str() ),
                                 pClient->GetBytesReceived() );
               }
            }
         }
      }
   );

   std::this_thread::sleep_for( 1h );
   oSocket.Close(); // Release the bound socket. Must be done to exit blocking accept call
   oExitSignal.set_value();
   oRetval.get();

   return 1;
}
```
_Note:_ This example requires C++1z to compile for an example which is C++03 compilant see [here](https://github.com/prince-chrismc/Simple-Socket/blob/d7b1c5d3a8436cdbc60793701ffed4f1f504c754/examples/EchoServer.cpp)

In this example the code is more involved because the `Accept()` on a blocking socket only returns with a new connection, in order to correctly close
the socket `Close()` must be called from a seperate thread. This examples makes use of a deffered launch to obtain this effect.
