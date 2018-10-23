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

#include <future>
#include <chrono>
#include "PassiveSocket.h" // Include header for passive socket object definition

using namespace std::chrono_literals;

static constexpr const int32 MAX_PACKET = 4096;

int main( int argc, char** argv )
{
   CPassiveSocket oSocket;
   std::promise<void> oExitSignal;

   oSocket.Initialize(); // Initialize our socket object
   oSocket.Listen( "127.0.0.1", 6789 ); // Bind to local host on port 6789 for ability to wait for incomming connections

   auto oRetval = std::async( std::launch::deferred, [ &oSocket, oExitEvent = oExitSignal.get_future() ]() {
         while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
         {
            std::unique_ptr<CActiveSocket> pClient;
            if( ( pClient = oSocket.AcceptUniqueOwnership() ) != nullptr ) // Wait for an incomming connection
            {
               if( pClient->Receive( MAX_PACKET ) ) // Receive request from the client.
               {
                  pClient->Send( pClient->GetData(), pClient->GetBytesReceived() ); // Send response to client and close connection to the client.
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
