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
      CSimpleSocket socket;

      REQUIRE( socket.GetSocketDescriptor() != INVALID_SOCKET );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );
   }

   SECTION( "UDP socket instantiation", "[UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      REQUIRE( socket.GetSocketDescriptor() != INVALID_SOCKET );
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
}