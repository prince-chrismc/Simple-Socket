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

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_NO_CPP17_UNCAUGHT_EXCEPTIONS // Not supported by Clang 6.0
#include "catch2/catch.hpp"

#include "PassiveSocket.h"

#include <string_view>

#ifdef _WIN32
#include <Ws2tcpip.h>
#elif defined(_LINUX) || defined (_DARWIN)
#include <netdb.h>
#endif

using namespace std::string_view_literals;

static constexpr auto HTTP_GET_ROOT_REQUEST = "GET / HTTP/1.0\r\n\r\n"sv;
static constexpr uint8 DNS_QUERY[] = { '\x12','\x34','\x01','\x00','\x00','\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x07','\x65','\x78','\x61','\x6d','\x70','\x6c','\x65','\x03','\x63','\x6f','\x6d','\x00','\x00','\x01','\x00','\x01' };
static constexpr auto DNS_QUERY_LENGTH = ( sizeof( DNS_QUERY ) / sizeof( DNS_QUERY[ 0 ] ) );
static constexpr uint8 TEXT_PACKET[] = { 'T', 'e', 's', 't', ' ', 'P', 'a', 'c', 'k', 'e', 't' };
static constexpr auto TEXT_PACKET_LENGTH = ( sizeof( TEXT_PACKET ) / sizeof( TEXT_PACKET[ 0 ] ) );

TEST_CASE( "Valid sockets are created", "[Initialization]" )
{
   SECTION( "TCP socket instantiation", "[TCP]" )
   {
      CSimpleSocket socket;

      REQUIRE( socket.GetSocketDescriptor() != INVALID_SOCKET );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );
   }

   SECTION( "UDP  socket instantiation", "[UDP]" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

      REQUIRE( socket.GetSocketDescriptor() != INVALID_SOCKET );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );
   }
}

TEST_CASE( "Open socket for communication", "[Open][UDP]" )
{
   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

   REQUIRE( socket.Open( "8.8.8.8", 53 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetServerAddr() == "8.8.8.8" );
   CHECK( socket.GetServerPort() == 53 );
}

TEST_CASE( "Establish connection to remote host", "[Open][TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

   SECTION( "Bad Port" )
   {
      CHECK_FALSE( socket.Open( "www.google.ca", 0 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidPort );
   }

   SECTION( "No Address" )
   {
      CHECK_FALSE( socket.Open( nullptr, 35345 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );
   }

   SECTION( "No Handle" )
   {
      CHECK( socket.Close() );
      CHECK_FALSE( socket.IsSocketValid() );
      CHECK_FALSE( socket.Open( "www.google.ca", 80 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
   }

   SECTION( "To Google" )
   {
      REQUIRE( socket.Open( "www.google.ca", 80 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }
}

#ifndef _DARWIN
TEST_CASE( "Sockets can send", "[Send][UDP]" )
{
   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   CHECK( socket.Open( "8.8.8.8", 53 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
   REQUIRE( socket.GetBytesSent() == DNS_QUERY_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}
#endif

TEST_CASE( "Sockets can transfer", "[Send][TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.Open( "www.google.ca", 80 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );
   REQUIRE( socket.GetBytesSent() == HTTP_GET_ROOT_REQUEST.length() );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

#ifndef _DARWIN
TEST_CASE( "Sockets can read", "[Receive][UDP]" )
{
   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   CHECK( socket.Open( "8.8.8.8", 53 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Receive( 1024 ) == 45 );
   REQUIRE( socket.GetBytesReceived() == 45 );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   const std::string dnsResponse = socket.GetData();

   REQUIRE( dnsResponse.length() == 45 );
   REQUIRE_THAT( dnsResponse, Catch::StartsWith( "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x07\x65\x78\x61" \
                 "\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00" \
                 "\x01\x00\x01\x00\x00" )
   );
   REQUIRE_THAT( dnsResponse, Catch::EndsWith( "\x00\x04\x5d\xb8\xd8\x22" ) );  // NOLINT(misc-string-literal-with-embedded-nul)
}
#endif

TEST_CASE( "Sockets can receive", "[Receive][TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.Open( "www.google.ca", 80 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
          == HTTP_GET_ROOT_REQUEST.length() );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetBytesReceived() == CSimpleSocket::SocketError );

   SECTION( "No Handle" )
   {
      CHECK( socket.Close() );
      CHECK_FALSE( socket.IsSocketValid() );
      CHECK( socket.Receive( 2048 ) == CSimpleSocket::SocketError );
      CHECK( socket.GetBytesReceived() == CSimpleSocket::SocketError );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
   }

   SECTION( "Nothing ???" )
   {
      CHECK( socket.Receive( 0 ) == 0 );
      CHECK( socket.GetBytesReceived() == 0 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }

   SECTION( "Using interal buffer" )
   {
      REQUIRE( socket.Receive( 2048 ) == 2048 );
      REQUIRE( socket.GetBytesReceived() == 2048 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      const std::string httpResponse = socket.GetData();

      REQUIRE( httpResponse.length() > 0 );
      REQUIRE_THAT( httpResponse, Catch::StartsWith( "HTTP/1.0 200 OK\r\n" ) && Catch::Contains( "\r\n\r\n<!doctype html>" ) && Catch::Contains( "<title>Google</title>" ) );
   }

   SECTION( "Using external buffer" )
   {
      uint8 buffer[ 2048 ];
      REQUIRE( socket.Receive( 2048, buffer ) == 2048 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      const std::string httpResponse( reinterpret_cast<const char*>( buffer ), socket.GetBytesReceived() );

      REQUIRE( httpResponse.length() > 0 );
      REQUIRE_THAT( httpResponse, Catch::StartsWith( "HTTP/1.0 200 OK\r\n" ) && Catch::Contains( "\r\n\r\n<!doctype html>" ) && Catch::Contains( "<title>Google</title>" ) );
   }

   SECTION( "external buffer is too small", "[.]" )
   {
      // sadly this test is not valid https://github.com/catchorg/Catch2/issues/553
      //uint8 buffer[ 512 ];
      //CHECK( socket.Receive( 1024, buffer ) == CSimpleSocket::SocketError );
      //CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidPointer );
   }
}

TEST_CASE( "Sockets have remotes information", "[TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

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
      addrinfo* pResult = nullptr;
      const int iErrorCode = getaddrinfo( "www.google.ca", NULL, &hints, &pResult );

      REQUIRE( iErrorCode == 0 );

      char buff[ 16 ];
      std::string googlesAddr = inet_ntop( AF_INET, &reinterpret_cast<sockaddr_in*>( pResult->ai_addr )->sin_addr, buff, 16 );

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

   CHECK( socket.Open( "www.google.ca", 80 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
          == HTTP_GET_ROOT_REQUEST.length() );

   CHECK( socket.Receive( 17 ) == 17 );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = socket.GetData();

   CHECK( httpResponse.length() > 0 );
   CHECK( httpResponse == "HTTP/1.0 200 OK\r\n" );

   REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( socket.Close() );
   REQUIRE_FALSE( socket.IsSocketValid() );

   REQUIRE( socket.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == CSimpleSocket::SocketError );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
}

#ifndef _DARWIN
TEST_CASE( "Sockets can close", "[Close][UDP]" )
{
   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   CHECK( socket.Open( "8.8.8.8", 53 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.Receive( 1024 ) == 45 );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   const std::string dnsResponse = socket.GetData();

   CHECK( dnsResponse.length() == 45 );
   CHECK_THAT( dnsResponse, Catch::StartsWith( "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x07\x65\x78\x61" \
               "\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00" \
               "\x01\x00\x01\x00\x00" )
   );
   CHECK_THAT( dnsResponse, Catch::EndsWith( "\x00\x04\x5d\xb8\xd8\x22" ) );  // NOLINT(misc-string-literal-with-embedded-nul)

   REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( socket.Close() );
   REQUIRE_FALSE( socket.IsSocketValid() );

   REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == CSimpleSocket::SocketError );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
}
#endif

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
   REQUIRE_FALSE( beta.IsSocketValid() );

   REQUIRE( alpha.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == CSimpleSocket::SocketError );
   REQUIRE( alpha.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
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
   REQUIRE( beta.Close() );
   REQUIRE_FALSE( beta.IsSocketValid() );

   beta = alpha;
   REQUIRE( beta.IsSocketValid() );

   REQUIRE( beta.Send( reinterpret_cast<const uint8*>( HTTP_GET_ROOT_REQUEST.data() ), HTTP_GET_ROOT_REQUEST.length() )
            == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( beta.Receive( 6 ) == 6 );
   REQUIRE( beta.GetSocketError() == CSimpleSocket::SocketSuccess );

   httpResponse = beta.GetData();
   REQUIRE( httpResponse == "Date: " );

   // Only clean up once since we duplicated the socket!
   REQUIRE( beta.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( beta.Close() );
   REQUIRE_FALSE( beta.IsSocketValid() );
}

#ifndef _DARWIN
#include <future>

TEST_CASE( "Sockets can echo", "[Echo][UDP]" )
{
   CPassiveSocket server( CSimpleSocket::SocketTypeUdp );

   CHECK( server.GetServerAddr() == "0.0.0.0" );
   CHECK( server.GetServerPort() == 0 );

   CHECK( server.GetClientAddr() == "0.0.0.0" );
   CHECK( server.GetClientPort() == 0 );

   REQUIRE( server.Listen( "127.0.0.1", 35346 ) );
   REQUIRE( server.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( server.GetServerAddr() == "127.0.0.1" );
   CHECK( server.GetServerPort() == 35346 );

   CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

   REQUIRE( socket.Open( "127.0.0.1", 35346 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetServerAddr() == "127.0.0.1" );
   CHECK( socket.GetServerPort() == 35346 );

   CAPTURE( socket.GetClientAddr() );
   CAPTURE( socket.GetClientPort() );

   auto serverRespone = std::async( std::launch::async, [ & ]
                                    {
                                       uint8 buffer[ TEXT_PACKET_LENGTH + 1 ];
                                       REQUIRE( server.Receive( 1024, buffer ) == TEXT_PACKET_LENGTH );

                                       CHECK( server.GetClientAddr() == socket.GetClientAddr() );
                                       CHECK( server.GetClientPort() == socket.GetClientPort() );

                                       REQUIRE( server.Send( buffer, server.GetBytesReceived() ) == TEXT_PACKET_LENGTH );
                                    }
   );

   REQUIRE( socket.Send( TEXT_PACKET, TEXT_PACKET_LENGTH ) == TEXT_PACKET_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Receive( 1024 ) == TEXT_PACKET_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetServerAddr() == "127.0.0.1" );
   CHECK( socket.GetServerPort() == 35346 );

   const std::string actualResponse = socket.GetData();
   const std::string expectedResponse( reinterpret_cast<const char*>( TEXT_PACKET ), TEXT_PACKET_LENGTH );

   CAPTURE( actualResponse );

   REQUIRE( actualResponse.length() == TEXT_PACKET_LENGTH );
   REQUIRE_THAT( actualResponse, Catch::StartsWith( expectedResponse ) );

   REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( socket.Close() );
   REQUIRE_FALSE( socket.IsSocketValid() );
}
#endif
