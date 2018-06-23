
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
   oSocket.Listen( "127.0.0.1", 6789 ); // Bind to local host on port 6789 for ability to wait for incomming connections

   auto oRetval = std::async( std::launch::deferred, [ &oSocket, oExitEvent = oExitSignal.get_future() ]() {
         while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
         {
            CActiveSocket* pClient = nullptr;
            if( ( pClient = oSocket.Accept() ) != nullptr ) // Wait for an incomming connection
            {
               if( pClient->Receive( MAX_PACKET ) ) // Receive request from the client.
               {
                  pClient->Send( pClient->GetData(), pClient->GetBytesReceived() ); // Send response to client and close connection to the client.
                  pClient->Close(); // Close socket since we have completed transmission
               }

               delete pClient; // Delete memory
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
