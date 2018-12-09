---
id: Getting-Started
title: Getting Started
sidebar_label: Getting Started
---
### Table of Contents
- [Client App](#client-app)
- [Server App](#server-app)

When operating on a socket object most methods will return true or false make validation very clean, see [below](#client-app).

## Client App
As mentioned previously the active socket ( `class CActiveSocket` ) is used to initiate a connections with a server on some known address and port.

> So you want to connect to an existing server...
>
> How do you do it?

There are many ways using the existing Berkley Socket API, but the goal of this class is to remove the many calls and man page lookups and replace
them with clear, concise set of methods which allow a developer to focus on the logic of network programming.

The following code will connect to a DAYTIME server on port 13, query for the current time, and close the socket.

```cpp
#include "ActiveSocket.h" // Include header for active socket object definition

constexpr const uint8* operator"" _byte( const char* text, std::size_t ) { return (const uint8 *)text; }

int main( int argc, char** argv )
{
   CActiveSocket oSocket; // Instantiate active socket object (defaults to TCP).
   oSocket.Initialize(); // Initialize our socket object

   if( oSocket.Open( "time-C.timefreq.bldrdoc.gov", 13 ) ) // Attempt connection to known remote server
   {
      if( oSocket.Send( "\n"_byte, 1 ) ) // Send a request the server for the current time.
      {
         if( oSocket.Receive( 48 ) ) // Receive response from the server.
         {
            std::cout << oSocket.GetData() << std::endl;
         }
         else
         {
            std::cout << "Unable to obtain time!" << std::endl;
         }
      }
   }

   oSocket.Close(); // Close the connection.

   return 1;
}

```

_Note:_ This example requires C++1z to compile for an example which is C++03 compilant see [here](https://github.com/prince-chrismc/Simple-Socket/blob/d7b1c5d3a8436cdbc60793701ffed4f1f504c754/examples/QueryDayTime.cpp)

You can see that the amount of code required to have an object perform network communciation is very small and simple.

## Server App
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

   oSocket.Initialize(); // Initialize our socket object
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
                  pClient->Close(); // Close socket since we have completed transmission
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
