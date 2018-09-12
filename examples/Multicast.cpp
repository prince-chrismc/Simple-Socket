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
#include "SimpleSocket.h"       // Include header for simple socket object definition

using namespace std::chrono_literals;

static constexpr const char* GROUP_ADDR = "239.1.2.3";
static constexpr const char* TEST_PACKET = "Test Packet";
static constexpr unsigned int SIZEOF_TEST_PACKET = sizeof( TEST_PACKET );

int main( int argc, char** argv )
{
   std::promise<void> oExitSignal;

   // ---------------------------------------------------------------------------------------------
   // Broadcaster Code
   // ---------------------------------------------------------------------------------------------
   auto oRetval = std::async( std::launch::async, [ oExitEvent = oExitSignal.get_future() ]() {
      CSimpleSocket oSender( CSimpleSocket::SocketTypeUdp );

      bool bRetval = oSender.Initialize();

      bRetval = oSender.SetMulticast( true );

      //bRetval = oSender.BindInterface( "172.31.15.134" );

      bRetval = oSender.JoinMulticast( GROUP_ADDR, 60000 );

      while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
      {
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

   bRetval = oReceiver.JoinMulticast( GROUP_ADDR, 60000 );

   uint8 buffer[ SIZEOF_TEST_PACKET ];
   oReceiver.Receive( SIZEOF_TEST_PACKET, buffer );

   std::this_thread::sleep_for( 5min );

   oExitSignal.set_value();
   oRetval.get();
   return 0;
}
