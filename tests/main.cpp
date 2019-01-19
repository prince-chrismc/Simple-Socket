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

#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"
#include "SimpleSocket.h"

TEST_CASE( "Valid sockets are created", "[Initialization]" )
{
   SECTION( "TCP socket instantiation", "[TCP]" )
   {
      const CSimpleSocket socket;

      REQUIRE( socket.IsSocketValid() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );
   }

   SECTION( "UDP socket instantiation", "[UDP]" )
   {
      const CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      REQUIRE( socket.IsSocketValid() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );
   }

   SECTION( "Invalid socket instantiation", "[TCP][UDP][INVALID]" )
   {
      REQUIRE_NOTHROW( CSimpleSocket{} );
      REQUIRE_NOTHROW( CSimpleSocket{ CSimpleSocket::SocketTypeTcp } );
      REQUIRE_NOTHROW( CSimpleSocket{ CSimpleSocket::SocketTypeUdp } );
      REQUIRE_THROWS_AS( CSimpleSocket{ CSimpleSocket::SocketTypeInvalid }, std::runtime_error );
   }

   SECTION( "Socket instantiated with default zero values", "[TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE_FALSE( socket.GetMulticast() );

      auto internalBuffer = socket.GetData();
      CAPTURE( internalBuffer );

      REQUIRE( internalBuffer.empty() );
      REQUIRE( internalBuffer.length() == 0 );

      REQUIRE( socket.GetBytesReceived() == -1 );
      REQUIRE( socket.GetBytesSent() == -1 );

      REQUIRE( socket.GetConnectTimeoutSec() == 0 );
      REQUIRE( socket.GetConnectTimeoutUSec() == 0 );
      REQUIRE( socket.GetReceiveTimeoutSec() == 0 );
      REQUIRE( socket.GetReceiveTimeoutUSec() == 0 );
      REQUIRE( socket.GetSendTimeoutSec() == 0 );
      REQUIRE( socket.GetSendTimeoutUSec() == 0 );

      REQUIRE( socket.GetTotalTimeMs() == 0 );
      REQUIRE( socket.GetTotalTimeUsec() > 0 ); // Timer tracked internal init from ctor

      REQUIRE( socket.GetServerAddr() == "0.0.0.0" );
      REQUIRE( socket.GetServerPort() == 0 );

      REQUIRE( socket.GetClientAddr() == "0.0.0.0" );
      REQUIRE( socket.GetClientPort() == 0 );

      REQUIRE( socket.GetJoinedGroup() == "0.0.0.0" );
   }
}
