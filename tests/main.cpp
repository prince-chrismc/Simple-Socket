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

#ifdef __cpp_lib_uncaught_exceptions
#define CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS // Not supported by Clang 6.0
#endif

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch2/catch.hpp"

#include "PassiveSocket.h"       // Include header for active socket object definition

#if defined(_LINUX) || defined (_DARWIN)
#include <netdb.h>
#endif

static constexpr auto DNS_QUERY_LENGTH = 30;
static constexpr uint8 DNS_QUERY[ DNS_QUERY_LENGTH ] = { '\x12','\x34','\x01','\x00','\x00','\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x07','\x65','\x78','\x61','\x6d','\x70','\x6c','\x65','\x03','\x63','\x6f','\x6d','\x00','\x00','\x01','\x00','\x01' };
static constexpr std::string_view HTTP_GET_ROOT_REQUEST = "GET / HTTP/1.0\r\n\r\n";

TEST_CASE( "Sockets are created", "[Initialize][TCP]" )
{
   CSimpleSocket socket;

   REQUIRE( socket.GetSocketDescriptor() != INVALID_SOCKET );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can open", "[Open][UDP]" )
{
   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   REQUIRE( socket.Open( "8.8.8.8", 53 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can connect", "[Open][TCP]" )
{
   CActiveSocket socket;

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can send", "[Send][UDP]" )
{
   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   REQUIRE( socket.Open( "8.8.8.8", 53 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can transfer", "[Send][TCP]" )
{
   CActiveSocket socket;

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can read", "[Receive][UDP]" )
{
   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   REQUIRE( socket.Open( "8.8.8.8", 53 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Receive( 1024 ) == 45 );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   const std::string dnsResponse = socket.GetData();

   REQUIRE( dnsResponse.length() == 45 );
   REQUIRE( dnsResponse.compare( 0, 37, "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x07\x65\x78\x61" \
            "\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00" \
            "\x01\x00\x01\x00\x00", 37 ) == 0
   );
   // Dont compare the two bytes for the TTL since it changes...
   REQUIRE( dnsResponse.compare( 39, 6, "\x00\x04\x5d\xb8\xd8\x22", 6 ) == 0 );
}

TEST_CASE( "Sockets can receive", "[Receive][TCP]" )
{
   CActiveSocket socket;

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( socket.Receive( 17 ) == 17 );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = socket.GetData();

   REQUIRE( httpResponse.length() > 0 );
   CAPTURE( httpResponse );
   REQUIRE( httpResponse == "HTTP/1.0 200 OK\r\n" );
}

TEST_CASE( "Sockets have server information" )
{
   CActiveSocket socket;

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   sockaddr_in serverAddr;
   memset( &serverAddr, 0, sizeof( serverAddr ) );
   serverAddr.sin_family = AF_INET;

   SECTION( "Socket.GetServerAddr" )
   {
      addrinfo hints{};
      memset( &hints, 0, sizeof( hints ) );
      hints.ai_flags = AI_ALL;
      hints.ai_family = AF_INET;
      addrinfo* pResult = NULL;
      const int iErrorCode = getaddrinfo( "www.google.ca", NULL, &hints, &pResult );

      REQUIRE( iErrorCode == 0 );

      char buff[ 16 ];
      std::string googlesAddr = inet_ntop( AF_INET, &( (sockaddr_in*)pResult->ai_addr )->sin_addr, buff, 16 );

      CAPTURE( buff );
      CAPTURE( socket.GetServerAddr() );

      REQUIRE( googlesAddr == socket.GetServerAddr() );
   }

   SECTION( "Socket.GetServerPort" )
   {
      REQUIRE( socket.GetServerPort() == 80 );
   }
}

TEST_CASE( "Sockets can disconnect", "[Close][TCP]" )
{
   CActiveSocket socket;

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( socket.Receive( 17 ) == 17 );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = socket.GetData();

   REQUIRE( httpResponse.length() > 0 );
   REQUIRE( httpResponse == "HTTP/1.0 200 OK\r\n" );

   REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( socket.Close() );
   REQUIRE( socket.IsSocketValid() == false );
}

TEST_CASE( "Sockets are ctor copyable", "[Socket][TCP]" )
{
   CActiveSocket alpha;

   REQUIRE( alpha.Open( "www.google.ca", 80 ) );

   REQUIRE( alpha.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( alpha.Receive( 17 ) == 17 );
   REQUIRE( alpha.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = alpha.GetData();

   REQUIRE( httpResponse.length() > 0 );
   REQUIRE( httpResponse == "HTTP/1.0 200 OK\r\n" );

   CActiveSocket beta( alpha );

   REQUIRE( beta.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( beta.Receive( 6 ) == 6 );
   REQUIRE( beta.GetSocketError() == CSimpleSocket::SocketSuccess );

   httpResponse = beta.GetData();
   CAPTURE( httpResponse );
   REQUIRE( httpResponse == "Date: " );

   // Only clean up once since we duplicated the socket!
   REQUIRE( beta.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( beta.Close() );
   REQUIRE( beta.IsSocketValid() == false );
}

TEST_CASE( "Sockets are assign copyable", "[Socket=][TCP]" )
{
   CActiveSocket alpha;

   REQUIRE( alpha.Open( "www.google.ca", 80 ) );

   REQUIRE( alpha.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( alpha.Receive( 17 ) == 17 );
   REQUIRE( alpha.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = alpha.GetData();

   REQUIRE( httpResponse.length() > 0 );
   REQUIRE( httpResponse == "HTTP/1.0 200 OK\r\n" );

   CActiveSocket beta;
   beta = alpha;

   REQUIRE( beta.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( beta.Receive( 6 ) == 6 );
   REQUIRE( beta.GetSocketError() == CSimpleSocket::SocketSuccess );

   httpResponse = beta.GetData();
   REQUIRE( httpResponse == "Date: " );

   // Only clean up once since we duplicated the socket!
   REQUIRE( beta.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( beta.Close() );
   REQUIRE( beta.IsSocketValid() == false );
}
