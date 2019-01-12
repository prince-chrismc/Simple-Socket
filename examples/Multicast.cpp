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

#include "SimpleSocket.h"   // Include header for simple socket object definition

#include <future>
#include <iostream>
#include <mutex>

using namespace std::chrono_literals;

static constexpr const char* GROUP_ADDR = "239.1.2.3";
static constexpr const char* TEST_PACKET = "Test Packet";
static constexpr auto PORT_NUMBER = 60000;

static constexpr unsigned int SIZEOF_TEST_PACKET = length( TEST_PACKET );
static_assert( SIZEOF_TEST_PACKET == 11, "Failed to compute SIZEOF_TEST_PACKET" );

int main( int argc, char** argv )
{
   std::cout << "Simple-Socket: Multi-cast Example" << std::endl << std::endl;

   std::mutex muConsoleOut;
   std::promise<void> oExitSignal;
   auto oExitEvent = std::make_shared<std::shared_future<void>>( oExitSignal.get_future() );

   // ---------------------------------------------------------------------------------------------
   // Broadcaster Code
   // ---------------------------------------------------------------------------------------------
   auto oTxComplete = std::async( std::launch::async, [oExitEvent, &muConsoleOut]() {
      CSimpleSocket oSender( CSimpleSocket::SocketTypeUdp );

      bool bRetval = oSender.SetMulticast( true );

      // bRetval = oSender.BindInterface( "192.168.0.195" );

      bRetval = oSender.JoinMulticast( GROUP_ADDR, PORT_NUMBER );

      {
         const auto sResult = ( bRetval ) ? "Successfully" : "Failed to";
         std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
         std::cout << "Tx // " << sResult << " join group '" << oSender.GetJoinedGroup().c_str() << "'." << std::endl;
      }

      while ( oExitEvent->wait_for( 250ms ) == std::future_status::timeout )
      {
         {
            std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
            std::cout << "Tx // Sending..." << std::endl;
         }

         oSender.Send( reinterpret_cast<const uint8_t*>( TEST_PACKET ), SIZEOF_TEST_PACKET );
      }

      oSender.Shutdown( CSimpleSocket::Both );

      return bRetval;
   } );

   // ---------------------------------------------------------------------------------------------
   // Listener Code
   // ---------------------------------------------------------------------------------------------
   CSimpleSocket oReceiver( CSimpleSocket::SocketTypeUdp );

   bool bRetval = oReceiver.SetMulticast( true );

   // bRetval = oReceiver.BindInterface( "192.168.0.195" );

   bRetval = oReceiver.JoinMulticast( GROUP_ADDR, PORT_NUMBER );

   {
      const auto sResult = ( bRetval ) ? "Successfully" : "Failed to";
      std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
      std::cout << "Rx // " << sResult << " join group '" << oReceiver.GetJoinedGroup().c_str() << "'." << std::endl;
   }

   auto oRxComplete = std::async( std::launch::async, [oExitEvent, &muConsoleOut, &oReceiver]() {
      while ( oExitEvent->wait_for( 100ms ) == std::future_status::timeout )
      {
         uint8_t buffer[ SIZEOF_TEST_PACKET + 1 ] = { '\0' };
         oReceiver.Receive( SIZEOF_TEST_PACKET, buffer );

         std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
         std::cout << "Rx // Obtained: '" << reinterpret_cast<char*>( buffer ) << "' from " << oReceiver.GetClientAddr()
                   << std::endl;

         if ( oReceiver.GetBytesReceived() < 1 ) break;
      }
   } );

   std::this_thread::sleep_for( 10s );

   oExitSignal.set_value();

   bRetval = oReceiver.Shutdown( CSimpleSocket::Both );
   bRetval = oReceiver.Close();

   oTxComplete.get();
   oRxComplete.get();

   return 0;
}
