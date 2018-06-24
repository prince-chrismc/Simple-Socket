/*

MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <string>
#include <future>
#include "PassiveSocket.h" // Include header for both passive and active socket object definition

using namespace std::chrono_literals;

auto TextToWire = []( const char* text ) constexpr { return (const uint8*)text; };
auto WireToText = []( const uint8* text ) constexpr { return (const char*)text; };

static constexpr const int32 NEXT_BYTE = 1;
static constexpr const char* TEST_PACKET = "Test Packet";
static constexpr const unsigned int TEST_PACKET_SIZE = sizeof( "Test Packet" );
static constexpr const char* LOCAL_HOST = "127.0.0.1";

uint16 LaunchEchoServer( std::promise<void>* pExitSignal );

int main( int argc, char** argv )
{
   std::promise<void> oExitSignal;

   const uint16 nPort = LaunchEchoServer( &oExitSignal );

   CActiveSocket oClient;

   oClient.Initialize();
   oClient.SetNonblocking();

   if( oClient.Open( LOCAL_HOST, nPort ) )
   {
      if( oClient.Send( TextToWire( ( std::to_string( TEST_PACKET_SIZE ) + "\n" + TEST_PACKET ).c_str() ), TEST_PACKET_SIZE ) )
      {
         int numBytes = -1;
         int bytesReceived = 0;

         oClient.Select();

         while( bytesReceived != TEST_PACKET_SIZE )
         {
            numBytes = oClient.Receive( NEXT_BYTE );

            if( numBytes > 0 )
            {
               std::string result;
               bytesReceived += numBytes;
               result.assign( WireToText( oClient.GetData() ), numBytes );
               printf( "received %d bytes: '%s'\n", numBytes, result.c_str() );
            }
            else
            {
               printf( "Received %d bytes\n", numBytes );
            }
         }
      }
   }
}




uint16 LaunchEchoServer( std::promise<void>* pExitSignal )
{
   std::promise<uint16> oPortEvent;
   auto oPortRetval = oPortEvent.get_future();

   auto oRetval = std::async( std::launch::async, [ oExitEvent = pExitSignal->get_future(), &oPortEvent ]() {
      CPassiveSocket oSocket;

      oSocket.Initialize(); // Initialize our socket object
      oSocket.SetNonblocking(); // Configure this socket to be non-blocking
      oSocket.Listen( LOCAL_HOST, 0 ); // Bind to local host on port any port

      oPortEvent.set_value( oSocket.GetServerPort() );

      while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
      {
         CActiveSocket* pClient = nullptr;
         if( ( pClient = oSocket.Accept() ) != nullptr ) // Wait for an incomming connection
         {
            pClient->Select(); // Wait for an event to occur on the socket

            uint32 iBytesLeftToReceive = -1;
            std::string sMessage;
            while( pClient->Receive( NEXT_BYTE ) >= 0 && iBytesLeftToReceive ) // Receive request from the client.
            {
               sMessage.append( WireToText( pClient->GetData() ), pClient->GetBytesReceived() ); // Gather Message in a buffer
               if( sMessage.back() == '\n' ) { iBytesLeftToReceive = std::stoul( sMessage ); sMessage = ""; }
               --iBytesLeftToReceive;
            }

            pClient->Send( TextToWire( sMessage.c_str() ), sMessage.size() ); // Send response to client and close connection to the client.
            pClient->Close(); // Close socket since we have completed transmission

            delete pClient; // Delete memory
         }
      }

      oSocket.Close(); // Release the bound socket. Must be done to exit blocking accept call
   }
   );

   return oPortRetval.get();
}
