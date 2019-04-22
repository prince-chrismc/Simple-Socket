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
#include <iterator>
#include <array>

using namespace std::string_view_literals;
using namespace std::chrono_literals;

static constexpr const char* GROUP_ADDR = "239.1.2.3";
static constexpr auto TEST_PACKET = "Test Packet"sv;
static constexpr auto PORT_NUMBER = 60000;

static constexpr unsigned int SIZEOF_TEST_PACKET = length( TEST_PACKET.data() );
static_assert( SIZEOF_TEST_PACKET == TEST_PACKET.length(), "Failed to compute SIZEOF_TEST_PACKET" );

int main()
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

      oSender.SetMulticast( true );
      // oSender.BindInterface( "192.168.0.195" );

      const auto sResult = ( oSender.JoinMulticast( GROUP_ADDR, PORT_NUMBER ) ) ? "Successfully" : "Failed to";
      {
         std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
         std::cout << "Tx // " << sResult << " join group '" << oSender.GetJoinedGroup() << "'." << std::endl;
      }

      constexpr auto SEND_DELAY = 250ms;
      while ( oExitEvent->wait_for( SEND_DELAY ) == std::future_status::timeout )
      {
         {
            std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
            std::cout << "Tx // Sending..." << std::endl;
         }
         oSender.Send( TEST_PACKET );
      }

      oSender.Shutdown( CSimpleSocket::Both );
      return true;
   } );

   // ---------------------------------------------------------------------------------------------
   // Listener Code
   // ---------------------------------------------------------------------------------------------
   CSimpleSocket oReceiver( CSimpleSocket::SocketTypeUdp );

   oReceiver.SetMulticast( true );
   // oReceiver.BindInterface( "192.168.0.195" );

   const auto sResult = ( oReceiver.JoinMulticast( GROUP_ADDR, PORT_NUMBER ) ) ? "Successfully" : "Failed to";
   {
      std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
      std::cout << "Rx // " << sResult << " join group '" << oReceiver.GetJoinedGroup() << "'." << std::endl;
   }

   auto oRxComplete = std::async( std::launch::async, [oExitEvent, &muConsoleOut, &oReceiver]() {
      constexpr auto READ_DELAY = 100ms;
      while ( oExitEvent->wait_for( READ_DELAY ) == std::future_status::timeout )
      {
         std::array<uint8_t, SIZEOF_TEST_PACKET + 1> buffer = { '\0' };
         oReceiver.Receive( SIZEOF_TEST_PACKET, buffer.data() );

         std::lock_guard<std::mutex> oPrintLock( muConsoleOut );
         std::cout << "Rx // Obtained: '";
         std::copy( buffer.cbegin(), buffer.cend(), std::ostream_iterator<char>( std::cout, " " ) );
         std::cout << "' from " << oReceiver.GetClientAddr() << std::endl;

         if ( oReceiver.GetBytesReceived() < 1 ) break;
      }
      return true;
   } );

   constexpr auto EXAMPLE_DURATION = 10s;
   std::this_thread::sleep_for( EXAMPLE_DURATION );

   oExitSignal.set_value();

   oReceiver.Shutdown( CSimpleSocket::Both );
   oReceiver.Close();

   oTxComplete.get();
   oRxComplete.get();

   return 0;
}
