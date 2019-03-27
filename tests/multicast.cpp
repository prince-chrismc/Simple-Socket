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

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );

      REQUIRE_FALSE( socket.GetMulticast() );
      REQUIRE( socket.SetMulticast( true ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.GetMulticast() );
   }

   SECTION( "Can toggle socket to Multicast", "[Multicast][UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );

      REQUIRE_FALSE( socket.GetMulticast() );
      REQUIRE( socket.SetMulticast( true ) );

      REQUIRE( socket.GetMulticast() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.SetMulticast( false ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE_FALSE( socket.GetMulticast() );
   }

   SECTION( "Can not use TCP for Multicast", "[Multicast][TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.GetMulticast() );
      REQUIRE_FALSE( socket.SetMulticast( true ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
      REQUIRE_FALSE( socket.GetMulticast() );
   }
}

TEST_CASE( "Sockets can Join group", "[Join]" )
{
   SECTION( "Can UDP Multicast socket join", "[Multicast][UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.IsSocketValid() );

      REQUIRE( socket.SetMulticast( true ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.GetMulticast() );

      REQUIRE( socket.JoinMulticast( "239.9.2.3", 12345 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.GetJoinedGroup() == "239.9.2.3" );
      REQUIRE( socket.GetClientPort() == 12345 );
   }

   SECTION( "Can UDP unicast socket join", "[UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.IsSocketValid() );
      REQUIRE_FALSE( socket.GetMulticast() );

      REQUIRE_FALSE( socket.JoinMulticast( "239.9.2.3", 12345 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );

      REQUIRE( socket.GetJoinedGroup() == "0.0.0.0" );
      REQUIRE( socket.GetClientPort() == 0 );
   }

   SECTION( "Can TCP socket join", "[TCP]")
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      REQUIRE_FALSE( socket.GetMulticast() );

      REQUIRE_FALSE( socket.JoinMulticast( "239.9.2.3", 12345 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );

      REQUIRE( socket.GetJoinedGroup() == "0.0.0.0" );
      REQUIRE( socket.GetClientPort() == 0 );
   }
}

TEST_CASE( "Sockets can Join from certain interface", "[Join][Bind]" )
{
   CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

   CHECK( socket.IsSocketValid() );

   REQUIRE( socket.SetMulticast( true ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.GetMulticast() );

   REQUIRE( socket.BindInterface( "0.0.0.0" ) );

   REQUIRE( socket.JoinMulticast( "239.9.2.3", 12345 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.GetJoinedGroup() == "239.9.2.3" );
   REQUIRE( socket.GetClientPort() == 12345 );
}