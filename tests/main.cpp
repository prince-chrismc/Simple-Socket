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

#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_MAIN   // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"
#include "PassiveSocket.h"

namespace
{
   void SocketHasDefaultValues( CSimpleSocket&& socket, CSimpleSocket::CSocketType type )
   {
      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == type );

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
      // This does occasionally come out as 0
      //REQUIRE( socket.GetTotalTimeUsec() > 0 );   // Timer tracked internal init from ctor

      REQUIRE( socket.GetServerAddr() == "0.0.0.0" );
      REQUIRE( socket.GetServerPort() == 0 );

      REQUIRE( socket.GetClientAddr() == "0.0.0.0" );
      REQUIRE( socket.GetClientPort() == 0 );

      REQUIRE( socket.GetJoinedGroup() == "0.0.0.0" );
   }

   void SocketHasInvalidValues( CSimpleSocket& socket )
   {
      REQUIRE_FALSE( socket.IsSocketValid() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
      REQUIRE( socket.GetSocketType() == CSimpleSocket::SocketTypeInvalid );

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
      // This does occasionally come out as 0
      // REQUIRE( socket.GetTotalTimeUsec() > 0 );   // Timer tracked internal init from ctor

      REQUIRE( socket.GetServerAddr() == CSimpleSocket::DescribeError( CSimpleSocket::SocketInvalidSocket ) );
      REQUIRE( socket.GetServerPort() == 0 );

      REQUIRE( socket.GetClientAddr() == CSimpleSocket::DescribeError( CSimpleSocket::SocketInvalidSocket ) );
      REQUIRE( socket.GetClientPort() == 0 );

      REQUIRE( socket.GetJoinedGroup() == CSimpleSocket::DescribeError( CSimpleSocket::SocketInvalidSocket ) );
   }

   /*template<class SOCKET>
   void SocketInitTest( CSimpleSocket::CSocketType type )
   {
      const SOCKET socket( type );
      REQUIRE( socket.IsSocketValid() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.GetSocketType() == type );
   }*/
}

TEST_CASE( "socket constructors", "[Initialization][TCP][UDP]" )
{
   auto tcp = CSimpleSocket::SocketTypeTcp;
   auto udp = CSimpleSocket::SocketTypeUdp;
   auto type = GENERATE( as<CSimpleSocket::CSocketType>{}, CSimpleSocket::SocketTypeTcp, CSimpleSocket::SocketTypeUdp );

   SECTION( "exceptions" )
   {
      SECTION( "Simple Socket" )
      {
         REQUIRE_NOTHROW( CSimpleSocket{} );
         REQUIRE_NOTHROW( CSimpleSocket{ tcp } );
         REQUIRE_NOTHROW( CSimpleSocket{ udp } );
         REQUIRE_THROWS_AS( CSimpleSocket{ CSimpleSocket::SocketTypeInvalid }, std::runtime_error );
      }

      SECTION( "Active Socket" )
      {
         REQUIRE_NOTHROW( CActiveSocket{} );
         REQUIRE_NOTHROW( CActiveSocket{ tcp } );
         REQUIRE_NOTHROW( CActiveSocket{ udp } );
         REQUIRE_THROWS_AS( CActiveSocket{ CSimpleSocket::SocketTypeInvalid }, std::runtime_error );
      }

      SECTION( "Passive Socket" )
      {
         REQUIRE_NOTHROW( CPassiveSocket{} );
         REQUIRE_NOTHROW( CPassiveSocket{ tcp } );
         REQUIRE_NOTHROW( CPassiveSocket{ udp } );
         REQUIRE_THROWS_AS( CPassiveSocket{ CSimpleSocket::SocketTypeInvalid }, std::runtime_error );
      }
   }

   SECTION( "valid handle" )
   {
      SECTION( "Simple Socket" )
      {
         const CSimpleSocket socket( type );
         REQUIRE( socket.IsSocketValid() );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
         REQUIRE( socket.GetSocketType() == type );
      }

      SECTION( "Active Socket" )
      {
         const CActiveSocket socket( type );
         REQUIRE( socket.IsSocketValid() );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
         REQUIRE( socket.GetSocketType() == type );
      }

      SECTION( "Passive Socket" )
      {
         const CPassiveSocket socket( type );
         REQUIRE( socket.IsSocketValid() );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
         REQUIRE( socket.GetSocketType() == type );
      }
   }

   SECTION( "default values" )
   {
      SECTION( "Simple Socket" )
      {
         CHECK_NOTHROW( SocketHasDefaultValues( CSimpleSocket{}, tcp ) );
         CHECK_NOTHROW( SocketHasDefaultValues( CSimpleSocket{ type }, type ) );
      }

      SECTION( "Active Socket" )
      {
         CHECK_NOTHROW( SocketHasDefaultValues( CActiveSocket{}, tcp ) );
         CHECK_NOTHROW( SocketHasDefaultValues( CActiveSocket{ type }, type ) );
      }

      SECTION( "Passive Socket" )
      {
         CHECK_NOTHROW( SocketHasDefaultValues( CPassiveSocket{}, tcp ) );
         CHECK_NOTHROW( SocketHasDefaultValues( CPassiveSocket{ type }, type ) );
      }
   }

   SECTION( "invalid after moved" )
   {
      SECTION( "Simple Socket" )
      {
         CSimpleSocket socket( type );
         CSimpleSocket secondary = std::move( socket );
         CHECK_NOTHROW( SocketHasInvalidValues( socket ) );
         CHECK_NOTHROW( SocketHasDefaultValues( std::move( secondary ), type ) );
      }

      SECTION( "Active Socket" )
      {
         CActiveSocket socket( type );
         CActiveSocket secondary = std::move( socket );
         CHECK_NOTHROW( SocketHasInvalidValues( socket ) );
         CHECK_NOTHROW( SocketHasDefaultValues( std::move( secondary ), type ) );
      }

      SECTION( "Passive Socket" )
      {
         CPassiveSocket socket( type );
         CPassiveSocket secondary = std::move( socket );
         CHECK_NOTHROW( SocketHasInvalidValues( socket ) );
         CHECK_NOTHROW( SocketHasDefaultValues( std::move( secondary ), type ) );
      }
   }
}

TEST_CASE( "Sockets dont leak" )
{
   auto type = GENERATE( as<CSimpleSocket::CSocketType>{}, CSimpleSocket::SocketTypeTcp, CSimpleSocket::SocketTypeUdp );

   SECTION( "Simple Socket" )
   {
      CSimpleSocket socket( type );
      REQUIRE( socket.Close() );
      REQUIRE_FALSE( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }

   SECTION( "Active Socket" )
   {
      CActiveSocket socket( type );
      REQUIRE( socket.Close() );
      REQUIRE_FALSE( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }

   SECTION( "Passive Socket" )
   {
      CPassiveSocket socket( type );
      REQUIRE( socket.Close() );
      REQUIRE_FALSE( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }
}