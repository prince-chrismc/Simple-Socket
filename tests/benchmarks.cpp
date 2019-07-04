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

#include <future>

TEST_CASE( "socket send", "[.][Benchmark][TCP][UDP]" )
{
   static constexpr uint8_t MSG[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd' };
   static constexpr auto MSG_LENGTH = ( sizeof( MSG ) / sizeof( MSG[ 0 ] ) );

   SECTION( "TCP" )
   {
      CPassiveSocket server;
      REQUIRE( server.Listen( nullptr, 35346 ) );
      CActiveSocket socket;
      REQUIRE( socket.Open( "127.0.0.1", 35346 ) );
      auto remote = std::async( std::launch::async, [&] {
         return server.Accept();
      } );

      BENCHMARK( "TCP Send" ) { socket.Send( MSG, MSG_LENGTH ); };

      CHECK( socket.Close() );
   }

#ifndef _DARWIN
   SECTION( "UDP" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );
      REQUIRE( socket.Open( "127.0.0.1", 12345 ) );

      BENCHMARK( "UDP Send" ) { socket.Send( MSG, MSG_LENGTH ); };

      CHECK( socket.Close() );
   }
#endif
}
