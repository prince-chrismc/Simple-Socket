/*

MIT License

Copyright (c) 2019 Chris McArthur, prince.chrismc(at)gmail(dot)com

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

#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"
#include "PassiveSocket.h"

using namespace std::string_view_literals;


TEST_CASE( "socket send", "[.][Benchmark][TCP][UDP]" )
{
   static constexpr auto udp = CSimpleSocket::SocketTypeUdp;

   SECTION( "TCP" )
   {
      CActiveSocket socket;
      REQUIRE( socket.Open( "www.google.ca", 80 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      BENCHMARK( "TCP Send" ) { socket.Send( "GET / HTTP/1.0\r\n\r\n"sv ); };
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.Flush() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }

#ifndef _DARWIN
   SECTION( "UDP" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );
      REQUIRE( socket.Open( "8.8.8.8", 53 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      static constexpr uint8_t DNS_QUERY[] = { '\x12', '\x34', '\x01', '\x00', '\x00', '\x01', '\x00', '\x00',
                                               '\x00', '\x00', '\x00', '\x00', '\x07', '\x65', '\x78', '\x61',
                                               '\x6d', '\x70', '\x6c', '\x65', '\x03', '\x63', '\x6f', '\x6d',
                                               '\x00', '\x00', '\x01', '\x00', '\x01' };
      static constexpr auto DNS_QUERY_LENGTH = ( sizeof( DNS_QUERY ) / sizeof( DNS_QUERY[ 0 ] ) );

	  BENCHMARK( "UDP Send" ) { socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ); };
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE_FALSE( socket.Flush() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
   }
#endif
}
