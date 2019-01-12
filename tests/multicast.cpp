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

#include "catch2/catch.hpp"
#include "SimpleSocket.h"

TEST_CASE( "Valid sockets are multicast", "[Initialization]" )
{
   SECTION( "Can set socket to Multicast", "[Multicast][UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.GetSocketDescriptor() != INVALID_SOCKET );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );

      REQUIRE_FALSE( socket.GetMulticast() );
      REQUIRE( socket.SetMulticast( true ) );
      REQUIRE( socket.GetMulticast() );
   }

   SECTION( "Can toggle socket to Multicast", "[Multicast][UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.GetSocketDescriptor() != INVALID_SOCKET );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );

      REQUIRE_FALSE( socket.GetMulticast() );
      REQUIRE( socket.SetMulticast( true ) );

      REQUIRE( socket.GetMulticast() );

      REQUIRE( socket.SetMulticast( false ) );
      REQUIRE_FALSE( socket.GetMulticast() );
   }

   SECTION( "Can not use TCP for Multicast", "[Multicast][TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.GetSocketDescriptor() != INVALID_SOCKET );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.GetMulticast() );
      REQUIRE_FALSE( socket.SetMulticast( true ) );
      REQUIRE_FALSE( socket.GetMulticast() );
   }
}
