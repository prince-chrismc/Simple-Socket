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

#include "SimpleSocket.h"       // Include header for simple socket object definition
#include <iostream>
#include <future>
#include <mutex>

using namespace std::chrono_literals;

static constexpr const char* GROUP_ADDR = "239.1.2.3";
static constexpr const char* TEST_PACKET = "Test Packet";

int constexpr length( const char* str ) { return *str ? 1 + length( str + 1 ) : 0; }
static constexpr unsigned int SIZEOF_TEST_PACKET = length( TEST_PACKET );
static_assert( SIZEOF_TEST_PACKET == 11, "Failed to compute SIZEOF_TEST_PACKET" );

int main( int argc, char** argv )
{
   std::cout << "Simple-Socket: Multi-cast Example" << std::endl << std::endl;

   std::promise<void> oExitSignal;
   std::mutex muConsoleOut;

   // ---------------------------------------------------------------------------------------------
   // Broadcaster Code
   // ---------------------------------------------------------------------------------------------
   auto oRetval = std::async( std::launch::async, [ oExitEvent = oExitSignal.get_future(), &muConsoleOut ]() {
      CSimpleSocket oSender( CSimpleSocket::SocketTypeUdp );

      bool bRetval = oSender.Initialize();

      bRetval = oSender.SetMulticast( true );

      //bRetval = oSender.BindInterface( "192.168.0.195" );

      bRetval = oSender.JoinMulticast( GROUP_ADDR, 60000 );

      {
         const auto sResult = ( bRetval ) ? "Successfully" : "Failed to";
         std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
         std::cout << "Tx // " << sResult << " join group '" << oSender.GetJoinedGroup().c_str() << "'." << std::endl;
      }

      while( oExitEvent.wait_for( 250ms ) == std::future_status::timeout )
      {
         {
            std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
            std::cout << "Tx // Sending..." << std::endl;
         }

         oSender.Send( reinterpret_cast<const uint8*>(TEST_PACKET), SIZEOF_TEST_PACKET );
      }

      return bRetval;
   }
   );

   // ---------------------------------------------------------------------------------------------
   // Listener Code
   // ---------------------------------------------------------------------------------------------
   CSimpleSocket oReceiver( CSimpleSocket::SocketTypeUdp );

   bool bRetval = oReceiver.Initialize();

   bRetval = oReceiver.SetMulticast( true );

   //bRetval = oReceiver.BindInterface( "192.168.0.195" );

   bRetval = oReceiver.JoinMulticast( GROUP_ADDR, 60000 );

   {
      const auto sResult = ( bRetval ) ? "Successfully" : "Failed to";
      std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
      std::cout << "Rx // " << sResult << " join group '" << oReceiver.GetJoinedGroup().c_str() << "'." << std::endl;
   }

   uint8 buffer[ SIZEOF_TEST_PACKET + 1 ];
   oReceiver.Receive( SIZEOF_TEST_PACKET, buffer );
   buffer[ SIZEOF_TEST_PACKET ] = '\0';

   {
      std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
      std::cout << "Rx // Obtained: '" << reinterpret_cast<char*>( buffer ) << "' from " << oReceiver.GetClientAddr() << std::endl;
   }

   std::this_thread::sleep_for( 5min );

   oExitSignal.set_value();
   oRetval.get();
   return 0;
}
