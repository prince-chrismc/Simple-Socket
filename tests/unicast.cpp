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
#include "PassiveSocket.h"

#include <future>
#include <string_view>

#ifdef _WIN32
#include <Ws2tcpip.h>
#elif defined( _LINUX ) || defined( _DARWIN )
#include <netdb.h>
#endif

using namespace std::chrono_literals;
using namespace std::string_literals;
using namespace std::string_view_literals;

static constexpr auto HTTP_GET_ROOT_REQUEST = "GET / HTTP/1.0\r\n\r\n"sv;
static constexpr uint8_t DNS_QUERY[] = { '\x12', '\x34', '\x01', '\x00', '\x00', '\x01', '\x00', '\x00', '\x00', '\x00',
                                         '\x00', '\x00', '\x07', '\x65', '\x78', '\x61', '\x6d', '\x70', '\x6c', '\x65',
                                         '\x03', '\x63', '\x6f', '\x6d', '\x00', '\x00', '\x01', '\x00', '\x01' };
static constexpr auto DNS_QUERY_LENGTH = ( sizeof( DNS_QUERY ) / sizeof( DNS_QUERY[ 0 ] ) );
static constexpr auto TEXT_PACKET = "Test Packet"sv;
static constexpr auto TEXT_PACKET_LENGTH = TEXT_PACKET.length();

TEST_CASE( "Sockets can be bound certain interface", "[Bind]" )
{
   SECTION( "UDP" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      REQUIRE( socket.IsSocketValid() );
      REQUIRE( socket.GetClientAddr() == "0.0.0.0" );
      REQUIRE( socket.GetClientPort() == 0 );

      SECTION( "Bind nullptr" )
      {
         CHECK( socket.BindInterface( nullptr ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "0.0.0.0" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }

      SECTION( "Bind ANY_IP" )
      {
         CHECK( socket.BindInterface( "0.0.0.0" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "0.0.0.0" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }

      SECTION( "Bind localhost" )
      {
         CHECK( socket.BindInterface( "127.0.0.1" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "127.0.0.1" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }

      SECTION( "Cannot bind twice" )
      {
         CHECK( socket.BindInterface( "127.0.0.1" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "127.0.0.1" );
         CHECK_FALSE( socket.GetClientPort() == 0 );

         CHECK_FALSE( socket.BindInterface( "0.0.0.0" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );

         CHECK( socket.GetClientAddr() == "127.0.0.1" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }
   }

   SECTION( "TCP" )
   {
      CSimpleSocket socket;

      REQUIRE( socket.IsSocketValid() );
      REQUIRE( socket.GetClientAddr() == "0.0.0.0" );
      REQUIRE( socket.GetClientPort() == 0 );

      SECTION( "Bind nullptr" )
      {
         CHECK( socket.BindInterface( nullptr ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "0.0.0.0" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }

      SECTION( "Bind ANY_IP" )
      {
         CHECK( socket.BindInterface( "0.0.0.0" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "0.0.0.0" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }

      SECTION( "Bind localhost" )
      {
         CHECK( socket.BindInterface( "127.0.0.1" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "127.0.0.1" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }

      SECTION( "Cannot bind twice" )
      {
         CHECK( socket.BindInterface( "127.0.0.1" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetClientAddr() == "127.0.0.1" );
         CHECK_FALSE( socket.GetClientPort() == 0 );

         CHECK_FALSE( socket.BindInterface( "0.0.0.0" ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );

         CHECK( socket.GetClientAddr() == "127.0.0.1" );
         CHECK_FALSE( socket.GetClientPort() == 0 );
      }
   }
}

TEST_CASE( "Establish connection to remote host", "[Open][TCP][UDP]" )
{
   SECTION( "TCP" )
   {
      CActiveSocket socket;

      REQUIRE( socket.IsSocketValid() );
      CHECK( socket.GetServerAddr() == "0.0.0.0" );
      CHECK( socket.GetServerPort() == 0 );

      SECTION( "Bad Port" )
      {
         CHECK_FALSE( socket.Open( "www.google.ca", 0 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidPort );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "No Address" )
      {
         CHECK_FALSE( socket.Open( nullptr, 35345 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "Bad Address" )
      {
         CHECK_FALSE( socket.Open( "132.354.134.546", 35345 ) );
         auto errnoCapture = int{ errno };
         CAPTURE( errnoCapture );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "Unknow Host name" )
      {
         CHECK_FALSE( socket.Open( "xyz.allphebties.cool", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "No Handle" )
      {
         CHECK( socket.Close() );
         CHECK_FALSE( socket.IsSocketValid() );
         CHECK_FALSE( socket.Open( "www.google.ca", 80 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "Connection Timeout" )
      {
         CHECK_FALSE( socket.Open( "www.google.ca", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketTimedout );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "Connection Refused" )
      {
         CHECK_FALSE( socket.Open( "127.0.0.1", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketConnectionRefused );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "No effect with invalid" )
      {
         CActiveSocket secondary = std::move( socket );
         REQUIRE_FALSE( socket.IsSocketValid() );   // NOLINT
         CHECK_FALSE( socket.Open( "127.0.0.1", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
         CHECK( socket.GetServerAddr() == CSimpleSocket::DescribeError( CSimpleSocket::SocketInvalidSocket ) );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "To Google" )
      {
         REQUIRE( socket.Open( "www.google.ca", 80 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
         CHECK_FALSE( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 80 );
      }

      SECTION( "Double Connect" )
      {
         REQUIRE( socket.Open( "www.google.ca", 80 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
         std::string googlesAddres = socket.GetServerAddr();
         CHECK_FALSE( googlesAddres == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 80 );

         REQUIRE_FALSE( socket.Open( "github.com", 80 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketAlreadyConnected );
         CHECK( socket.GetServerAddr() == googlesAddres );
         CHECK( socket.GetServerPort() == 80 );
      }

      CHECK( socket.Shutdown( CSimpleSocket::Both ) );
      CHECK( socket.Close() );
      CHECK_FALSE( socket.IsSocketValid() );
   }

   SECTION( "UDP" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

      REQUIRE( socket.IsSocketValid() );
      CHECK( socket.GetServerAddr() == "0.0.0.0" );
      CHECK( socket.GetServerPort() == 0 );

      SECTION( "Bad Port" )
      {
         CHECK_FALSE( socket.Open( "8.8.8.8", 0 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidPort );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "No Address" )
      {
         CHECK_FALSE( socket.Open( nullptr, 53 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "Bad Address" )
      {
         CHECK_FALSE( socket.Open( "132.354.134.546", 53 ) );
         auto errnoCapture = int{ errno };
         CAPTURE( errnoCapture );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "Unknow Host name" )
      {
         CHECK_FALSE( socket.Open( "xyz.allphebties.cool", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "No Handle" )
      {
         CHECK( socket.Close() );
         CHECK_FALSE( socket.IsSocketValid() );
         CHECK_FALSE( socket.Open( "www.google.ca", 80 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
         CHECK( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "Connection Random A" )
      {
         // in UDP there's no check if someone is on the other end
         // so it should always work.
         CHECK( socket.Open( "www.google.ca", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
         CHECK_FALSE( socket.GetServerAddr() == "0.0.0.0" );
         CHECK( socket.GetServerPort() == 34867 );
      }

      SECTION( "Connection Random B" )
      {
         // in UDP there's no check if someone is on the other end
         // so it should always work.
         CHECK( socket.Open( "127.0.0.1", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
         CHECK( socket.GetServerAddr() == "127.0.0.1" );
         CHECK( socket.GetServerPort() == 34867 );
      }

      SECTION( "No effect with invalid" )
      {
         CActiveSocket secondary = std::move( socket );
         REQUIRE_FALSE( socket.IsSocketValid() );
         CHECK_FALSE( socket.Open( "127.0.0.1", 34867 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
         CHECK( socket.GetServerAddr() == CSimpleSocket::DescribeError( CSimpleSocket::SocketInvalidSocket ) );
         CHECK( socket.GetServerPort() == 0 );
      }

      SECTION( "To Google" )
      {
         REQUIRE( socket.Open( "8.8.8.8", 53 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.GetServerAddr() == "8.8.8.8" );
         CHECK( socket.GetServerPort() == 53 );
      }

      SECTION( "Double Open" )
      {
         REQUIRE( socket.Open( "8.8.8.8", 53 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.GetServerAddr() == "8.8.8.8" );
         REQUIRE( socket.GetServerPort() == 53 );

         REQUIRE( socket.Open( "1.1.1.1", 53 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.GetServerAddr() == "1.1.1.1" );
         CHECK( socket.GetServerPort() == 53 );
      }

      CHECK( socket.Shutdown( CSimpleSocket::Both ) );
      CHECK( socket.Close() );
      CHECK_FALSE( socket.IsSocketValid() );
   }
}

TEST_CASE( "Sockets can send", "[Send][TCP][UDP]" )
{
   SECTION( "TCP" )
   {
      CActiveSocket socket;

      CHECK( socket.Open( "www.google.ca", 80 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      SECTION( "Plain send" )
      {
         REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );
         REQUIRE( socket.GetBytesSent() == HTTP_GET_ROOT_REQUEST.length() );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Send from invalid socket" )
      {
         CActiveSocket secondary = std::move( socket );

         REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == CSimpleSocket::SocketError );   // NOLINT
         REQUIRE( socket.GetBytesSent() == CSimpleSocket::SocketError );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
      }

      SECTION( "Send 2.5s timeout" )
      {
         REQUIRE( socket.SetSendTimeout( 2, 500 ) );

         REQUIRE( socket.GetSendTimeoutSec() == 2 );
         REQUIRE( socket.GetSendTimeoutUSec() == 500 );

         REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

         CAPTURE( "Send (ms)", socket.GetTotalTimeMs(), "Send (us)", socket.GetTotalTimeUsec() );
         CHECK( socket.GetTotalTimeMs() == 0 );
         CHECK_FALSE( socket.GetTotalTimeUsec() == 0 );

         REQUIRE( socket.GetBytesSent() == HTTP_GET_ROOT_REQUEST.length() );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Send 0.1s timeout" )
      {
         REQUIRE( socket.SetSendTimeout( 0, 100 ) );

         REQUIRE( socket.GetSendTimeoutSec() == 0 );
         REQUIRE( socket.GetSendTimeoutUSec() == 100 );

         REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

         CAPTURE( "Send (ms)", socket.GetTotalTimeMs(), "Send (us)", socket.GetTotalTimeUsec() );
         CHECK( socket.GetTotalTimeMs() == 0 );
         CHECK_FALSE( socket.GetTotalTimeUsec() == 0 );

         REQUIRE( socket.GetBytesSent() == HTTP_GET_ROOT_REQUEST.length() );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Multiple servers Connect" )
      {
         REQUIRE( socket.Open( "www.google.ca", 80 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

         CHECK( socket.Receive( 17 ) == 17 );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         std::string httpResponse = socket.GetData();

         CAPTURE( httpResponse );

         CHECK( httpResponse.length() > 0 );
         CHECK( httpResponse == "HTTP/1.0 200 OK\r\n" );

         REQUIRE_FALSE( socket.Open( "github.com", 80 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketAlreadyConnected );
      }
   }

#ifndef _DARWIN
   SECTION( "UDP" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.Open( "8.8.8.8", 53 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      SECTION( "plain Send" )
      {
         REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
         REQUIRE( socket.GetBytesSent() == DNS_QUERY_LENGTH );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Send from invalid socket" )
      {
         CActiveSocket secondary = std::move( socket );

         REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == CSimpleSocket::SocketError );   // NOLINT
         REQUIRE( socket.GetBytesSent() == CSimpleSocket::SocketError );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
      }

      SECTION( "Send 2.5s timeout" )
      {
         REQUIRE( socket.SetSendTimeout( 2, 500 ) );

         REQUIRE( socket.GetSendTimeoutSec() == 2 );
         REQUIRE( socket.GetSendTimeoutUSec() == 500 );

         REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );

         CAPTURE( "Send (ms)", socket.GetTotalTimeMs(), "Send (us)", socket.GetTotalTimeUsec() );
         CHECK( socket.GetTotalTimeMs() == 0 );
         CHECK_FALSE( socket.GetTotalTimeUsec() == 0 );

         REQUIRE( socket.GetBytesSent() == DNS_QUERY_LENGTH );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Send 0.1s timeout" )
      {
         REQUIRE( socket.SetSendTimeout( 0, 100 ) );

         REQUIRE( socket.GetSendTimeoutSec() == 0 );
         REQUIRE( socket.GetSendTimeoutUSec() == 100 );

         REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );

         CAPTURE( "Send (ms)", socket.GetTotalTimeMs(), "Send (us)", socket.GetTotalTimeUsec() );
         CHECK( socket.GetTotalTimeMs() == 0 );
         CHECK_FALSE( socket.GetTotalTimeUsec() == 0 );

         REQUIRE( socket.GetBytesSent() == DNS_QUERY_LENGTH );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Double Connect" )
      {
         REQUIRE( socket.Open( "8.8.8.8", 53 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.GetServerAddr() == "8.8.8.8" );
         REQUIRE( socket.GetServerPort() == 53 );

         CHECK( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.Receive( 1024 ) == 45 );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         const std::string googleDnsResponse = socket.GetData();
         CAPTURE( googleDnsResponse );

         CHECK( googleDnsResponse.length() == 45 );
         CHECK_THAT( googleDnsResponse,
                     Catch::StartsWith( "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x07\x65\x78\x61"s +
                                        "\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00"s +
                                        "\x01\x00\x01\x00\x00"s ) );
         CHECK_THAT( googleDnsResponse, Catch::EndsWith( "\x00\x04\x5d\xb8\xd8\x22"s ) );

         REQUIRE( socket.Open( "1.1.1.1", 53 ) );
         REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.GetServerAddr() == "1.1.1.1" );
         CHECK( socket.GetServerPort() == 53 );

         CHECK( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         CHECK( socket.Receive( 1024 ) == 45 );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         const std::string cloudfareDnsResponse = socket.GetData();
         CAPTURE( cloudfareDnsResponse );

         CHECK( cloudfareDnsResponse.length() == 45 );
         REQUIRE_THAT( cloudfareDnsResponse,
                       Catch::StartsWith( "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x07\x65\x78\x61"s +
                                          "\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00"s +
                                          "\x01\x00\x01\x00\x00"s ) );
         REQUIRE_THAT( cloudfareDnsResponse, Catch::EndsWith( "\x00\x04\x5d\xb8\xd8\x22"s ) );

         REQUIRE_FALSE( googleDnsResponse == cloudfareDnsResponse );
      }
   }
#endif
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
   REQUIRE_THAT( dnsResponse,
                 Catch::StartsWith( "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x07\x65\x78\x61"s +
                                    "\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00"s +
                                    "\x01\x00\x01\x00\x00"s ) );
   REQUIRE_THAT( dnsResponse, Catch::EndsWith( "\x00\x04\x5d\xb8\xd8\x22"s ) );
}
#endif

TEST_CASE( "Sockets can receive", "[Receive][TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.Open( "www.google.ca", 80 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );
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
      REQUIRE( socket.Receive( 1368 ) == 1368 );
      REQUIRE( socket.GetBytesReceived() == 1368 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      const std::string httpResponse = socket.GetData();

      REQUIRE( httpResponse.length() > 0 );
      REQUIRE_THAT( httpResponse,
                    Catch::StartsWith( "HTTP/1.0 200 OK\r\n" ) && Catch::Contains( "\r\n\r\n<!doctype html>" ) );
   }

   SECTION( "Using external buffer" )
   {
      uint8_t buffer[ 1368 ];
      REQUIRE( socket.Receive( 1368, buffer ) == 1368 );
      REQUIRE( socket.GetBytesReceived() == 1368 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      const std::string httpResponse( reinterpret_cast<const char*>( buffer ), socket.GetBytesReceived() );

      REQUIRE( httpResponse.length() > 0 );
      REQUIRE_THAT( httpResponse,
                    Catch::StartsWith( "HTTP/1.0 200 OK\r\n" ) && Catch::Contains( "\r\n\r\n<!doctype html>" ) );
   }

   SECTION( "external buffer is too small" )
   {
      // sadly this test is not valid https://github.com/catchorg/Catch2/issues/553
      // uint8_t buffer[ 512 ];
      // CHECK( socket.Receive( 1024, buffer ) == CSimpleSocket::SocketError );
      // CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidPointer );
   }
}

TEST_CASE( "Receive a huge message", "[!mayfail][TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.Open( "www.google.ca", 80 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetBytesReceived() == CSimpleSocket::SocketError );

   // Testing with values above 1368 is risky because the
   // internet might fragment packets and fail the test
   REQUIRE( socket.Receive( 8420 ) > 1024 );

   // This list of values are ones I've seen and make sence based on the OS
   // And network enviroment... DISCLAIMER: It's very subjective!
   auto accpetedValues = {
      8420,         // MAX
      1418, 2836,   // UBUNTU
      1368, 2736,   // MAC
      7040, 7090    // TRAVIS_CI
   };
   CHECK_THAT( accpetedValues, Catch::VectorContains( socket.GetBytesReceived() ) );

   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   const std::string httpResponse = socket.GetData();

   REQUIRE( httpResponse.length() > 0 );
   REQUIRE_THAT( httpResponse,
                 Catch::StartsWith( "HTTP/1.0 200 OK\r\n" ) && Catch::Contains( "\r\n\r\n<!doctype html>" ) );
}

TEST_CASE( "Sockets have remotes information", "[!mayfail][TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   SECTION( "Socket.GetServerAddr" )
   {
      addrinfo hints{};
      memset( &hints, 0, sizeof( hints ) );
      hints.ai_flags = AI_ALL;
      hints.ai_family = AF_INET;
      addrinfo* pResult = nullptr;
      const int iErrorCode = getaddrinfo( "www.google.ca", nullptr, &hints, &pResult );

      REQUIRE( iErrorCode == 0 );

      char buff[ 16 ];
      std::string googlesAddr =
          inet_ntop( AF_INET, &reinterpret_cast<sockaddr_in*>( pResult->ai_addr )->sin_addr, buff, 16 );

      CAPTURE( buff );
      CAPTURE( socket.GetServerAddr() );

      // This may fail because the DNS lookup intentionally does bandwidth throttling
      // and returns different adress
      REQUIRE( googlesAddr == socket.GetServerAddr() );
   }

   SECTION( "Socket.GetServerPort" ) { REQUIRE( socket.GetServerPort() == 80 ); }
}

TEST_CASE( "Sockets can disconnect", "[Close][TCP]" )
{
   CActiveSocket socket;

   CHECK( socket.Open( "www.google.ca", 80 ) );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

   CHECK( socket.Receive( 17 ) == 17 );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = socket.GetData();

   CAPTURE( httpResponse );

   CHECK( httpResponse.length() > 0 );
   CHECK( httpResponse == "HTTP/1.0 200 OK\r\n" );

   REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( socket.Close() );
   REQUIRE_FALSE( socket.IsSocketValid() );

   REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == CSimpleSocket::SocketError );
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
   CAPTURE( dnsResponse );

   CHECK( dnsResponse.length() == 45 );
   CHECK_THAT( dnsResponse,
               Catch::StartsWith( "\x12\x34\x81\x80\x00\x01\x00\x01\x00\x00\x00\x00\x07\x65\x78\x61"s +
                                  "\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01\xc0\x0c\x00"s +
                                  "\x01\x00\x01\x00\x00"s ) );
   CHECK_THAT( dnsResponse, Catch::EndsWith( "\x00\x04\x5d\xb8\xd8\x22"s ) );

   REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( socket.Close() );
   REQUIRE_FALSE( socket.IsSocketValid() );

   REQUIRE( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == CSimpleSocket::SocketError );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
}
#endif

TEST_CASE( "Sockets are ctor moveable", "[Socket][TCP]" )
{
   CActiveSocket alpha;

   CHECK( alpha.Open( "www.google.ca", 80 ) );
   CHECK( alpha.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );
   CHECK( alpha.Receive( 17 ) == 17 );
   CHECK( alpha.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = alpha.GetData();

   CHECK( httpResponse.length() > 0 );
   CHECK( httpResponse == "HTTP/1.0 200 OK\r\n" );

   CActiveSocket beta( std::move( alpha ) );
   REQUIRE_FALSE( alpha.IsSocketValid() );   // NOLINT
   REQUIRE( beta.IsSocketValid() );

   REQUIRE( beta.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( beta.Receive( 6 ) == 6 );
   REQUIRE( beta.GetSocketError() == CSimpleSocket::SocketSuccess );

   httpResponse = beta.GetData();
   CAPTURE( httpResponse );
   REQUIRE( httpResponse == "Date: " );

   // Only clean up once since the other is uninitialized!
   REQUIRE( beta.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( beta.Close() );
   REQUIRE_FALSE( beta.IsSocketValid() );

   REQUIRE( alpha.Send( HTTP_GET_ROOT_REQUEST ) == CSimpleSocket::SocketError );   // NOLINT(hicpp-invalid-access-moved)
   REQUIRE( alpha.GetSocketError() == CSimpleSocket::SocketInvalidSocket );        // NOLINT(hicpp-invalid-access-moved)
}

TEST_CASE( "Sockets are assign moveable", "[Socket=][TCP]" )
{
   CActiveSocket alpha;

   CHECK( alpha.Open( "www.google.ca", 80 ) );
   CHECK( alpha.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );
   CHECK( alpha.Receive( 17 ) == 17 );
   CHECK( alpha.GetSocketError() == CSimpleSocket::SocketSuccess );

   std::string httpResponse = alpha.GetData();

   CHECK( httpResponse.length() > 0 );
   CHECK( httpResponse == "HTTP/1.0 200 OK\r\n" );

   CActiveSocket beta;
   REQUIRE( beta.Close() );
   REQUIRE_FALSE( beta.IsSocketValid() );

   beta = std::move( alpha );

   REQUIRE_FALSE( alpha.IsSocketValid() );   // NOLINT
   REQUIRE( beta.IsSocketValid() );

   REQUIRE( beta.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( beta.Receive( 6 ) == 6 );
   REQUIRE( beta.GetSocketError() == CSimpleSocket::SocketSuccess );

   httpResponse = beta.GetData();
   CAPTURE( httpResponse );
   REQUIRE( httpResponse == "Date: " );

   // Only clean up once since the other is uninitialized!
   REQUIRE( beta.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( beta.Close() );
   REQUIRE_FALSE( beta.IsSocketValid() );

   REQUIRE( alpha.Send( HTTP_GET_ROOT_REQUEST ) == CSimpleSocket::SocketError );   // NOLINT(hicpp-invalid-access-moved)
   REQUIRE( alpha.GetSocketError() == CSimpleSocket::SocketInvalidSocket );        // NOLINT(hicpp-invalid-access-moved)
}

TEST_CASE( "Sockets can listen", "[Listen][Bind][TCP]" )
{
   CPassiveSocket socket;

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

   SECTION( "Bad Address" )
   {
      CHECK_FALSE( socket.Listen( "132.354.134.546", 35345 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );

      CHECK( socket.GetServerAddr() == "0.0.0.0" );
      CHECK( socket.GetServerPort() == 0 );
   }

   SECTION( "Unknow Host name" )
   {
      CHECK_FALSE( socket.Listen( "xyz.allphebties.cool", 34867 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidAddress );

      CHECK( socket.GetServerAddr() == "0.0.0.0" );
      CHECK( socket.GetServerPort() == 0 );
   }

   SECTION( "No Handle" )
   {
      CHECK( socket.Close() );
      CHECK_FALSE( socket.IsSocketValid() );
      CHECK_FALSE( socket.Listen( "127.0.0.1", 80 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );

      CHECK( socket.GetServerAddr() == "0.0.0.0" );
      CHECK( socket.GetServerPort() == 0 );
   }

   SECTION( "To Localhost" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 35346 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 35346 );
   }

   SECTION( "To Any Port" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 0 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.GetServerAddr() == "127.0.0.1" );
      REQUIRE( socket.GetServerPort() > 0 );
   }

   SECTION( "To any Address" )
   {
      REQUIRE( socket.Listen( nullptr, 35345 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.GetServerAddr() == "0.0.0.0" );
      REQUIRE( socket.GetServerPort() == 35345 );
   }

   SECTION( "Double Listen from same socket" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 54683 );

      CHECK_FALSE( socket.Listen( nullptr, 45673 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );

      CHECK( socket.GetServerAddr() == "0.0.0.0" );
      CHECK( socket.GetServerPort() == 0 );
   }

   SECTION( "Second Listen one the same ip and port after close" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 54683 );

      REQUIRE( socket.Close() );

      CPassiveSocket secondSocket;

      REQUIRE( secondSocket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( secondSocket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( secondSocket.GetServerAddr() == "127.0.0.1" );
      CHECK( secondSocket.GetServerPort() == 54683 );
   }

   SECTION( "Double Listen one the same ip and port" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 54683 );

      CPassiveSocket duplicateSocket;

      CHECK_FALSE( duplicateSocket.Listen( "127.0.0.1", 54683 ) );
      CHECK( duplicateSocket.GetSocketError() == CSimpleSocket::SocketAddressInUse );

      CHECK( duplicateSocket.GetServerAddr() == "0.0.0.0" );
      CHECK( duplicateSocket.GetServerPort() == 0 );
   }

   SECTION( "BindInterface then Listen on same ip" )
   {
      REQUIRE( socket.BindInterface( "127.0.0.1" ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE_FALSE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );
   }

   SECTION( "BindInterface then Listen on different ip" )
   {
      REQUIRE( socket.BindInterface( "127.0.0.1" ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE_FALSE( socket.Listen( nullptr, 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );
   }

   SECTION( "Listen then BindInterface" )
   {
      REQUIRE( socket.Listen( nullptr, 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE_FALSE( socket.BindInterface( "127.0.0.1" ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );
   }
}

TEST_CASE( "Sockets can hear", "[Listen][UDP]" )
{
   CPassiveSocket socket( CSimpleSocket::SocketTypeUdp );

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

   SECTION( "Listen on port" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 54683 );
   }

   SECTION( "Double Listen from same socket" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 54683 );

      CHECK_FALSE( socket.Listen( nullptr, 45673 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );

      CHECK( socket.GetServerAddr() == "0.0.0.0" );
      CHECK( socket.GetServerPort() == 0 );
   }

   SECTION( "Second Listen one the same ip and port after close" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 54683 );

      REQUIRE( socket.Close() );

      CPassiveSocket secondSocket;

      CHECK( secondSocket.Listen( "127.0.0.1", 54683 ) );
      CHECK( secondSocket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( secondSocket.GetServerAddr() == "127.0.0.1" );
      CHECK( secondSocket.GetServerPort() == 54683 );
   }

   SECTION( "Double Listen one the same ip and port" )
   {
      REQUIRE( socket.Listen( "127.0.0.1", 54683 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.GetServerAddr() == "127.0.0.1" );
      CHECK( socket.GetServerPort() == 54683 );

      CPassiveSocket duplicateSocket;

      CHECK( duplicateSocket.Listen( "127.0.0.1", 54683 ) );
      CHECK( duplicateSocket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( duplicateSocket.GetServerAddr() == "127.0.0.1" );
      CHECK( duplicateSocket.GetServerPort() == 54683 );
   }

   SECTION( "Can not accept cause it's UDP" )
   {
      REQUIRE( socket.Accept() == nullptr );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
   }
}

#ifndef _DARWIN
TEST_CASE( "Sockets can repeate", "[Listen][Open][UDP]" )
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

   auto serverRespone = std::async( std::launch::async, [&] {
      uint8_t buffer[ TEXT_PACKET_LENGTH + 1 ];
      REQUIRE( server.Receive( 1024, buffer ) == TEXT_PACKET_LENGTH );

      CHECK( server.GetClientAddr() == socket.GetClientAddr() );
      CHECK( server.GetClientPort() == socket.GetClientPort() );

      CHECK( server.GetServerAddr() == socket.GetServerAddr() );
      CHECK( server.GetServerPort() == socket.GetServerPort() );

      REQUIRE( server.Send( buffer, server.GetBytesReceived() ) == TEXT_PACKET_LENGTH );
   } );

   REQUIRE( socket.Send( TEXT_PACKET ) == TEXT_PACKET_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Receive( 1024 ) == TEXT_PACKET_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetServerAddr() == "127.0.0.1" );
   CHECK( socket.GetServerPort() == 35346 );

   const std::string actualResponse = socket.GetData();
   const std::string expectedResponse( TEXT_PACKET.data(), TEXT_PACKET_LENGTH );

   CAPTURE( actualResponse );

   REQUIRE( actualResponse.length() == TEXT_PACKET_LENGTH );
   REQUIRE_THAT( actualResponse, Catch::StartsWith( expectedResponse ) );

   REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
   REQUIRE( socket.Close() );
   REQUIRE_FALSE( socket.IsSocketValid() );
}
#endif

TEST_CASE( "Sockets can echo", "[Listen][Open][Accept][TCP]" )
{
   CPassiveSocket server;

   CHECK( server.GetServerAddr() == "0.0.0.0" );
   CHECK( server.GetServerPort() == 0 );

   CHECK( server.GetClientAddr() == "0.0.0.0" );
   CHECK( server.GetClientPort() == 0 );

   REQUIRE( server.Listen( "127.0.0.1", 35346 ) );
   REQUIRE( server.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( server.GetServerAddr() == "127.0.0.1" );
   CHECK( server.GetServerPort() == 35346 );

   CActiveSocket socket;

   CHECK( socket.GetServerAddr() == "0.0.0.0" );
   CHECK( socket.GetServerPort() == 0 );

   REQUIRE( socket.Open( "127.0.0.1", 35346 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetServerAddr() == "127.0.0.1" );
   CHECK( socket.GetServerPort() == 35346 );

   CAPTURE( socket.GetClientAddr() );
   CAPTURE( socket.GetClientPort() );

   auto serverRespone = std::async( std::launch::async, [&] {
      std::unique_ptr<CActiveSocket> connection = server.Accept();
      REQUIRE( connection != nullptr );

      REQUIRE( connection->Receive( 1024 ) == TEXT_PACKET_LENGTH );

      CHECK( connection->GetClientAddr() == socket.GetClientAddr() );
      CHECK( connection->GetClientPort() == socket.GetClientPort() );

      CHECK( connection->GetServerAddr() == server.GetServerAddr() );
      CHECK( connection->GetServerPort() == server.GetServerPort() );

      CHECK( server.GetClientAddr() == socket.GetClientAddr() );
      CHECK( server.GetClientPort() == socket.GetClientPort() );

      CHECK( server.GetServerAddr() == socket.GetServerAddr() );
      CHECK( server.GetServerPort() == socket.GetServerPort() );

      REQUIRE( connection->Send( (uint8_t*)connection->GetData().c_str(), connection->GetBytesReceived() ) ==
               TEXT_PACKET_LENGTH );
   } );

   REQUIRE( socket.Send( TEXT_PACKET ) == TEXT_PACKET_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Receive( 1024 ) == TEXT_PACKET_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetServerAddr() == "127.0.0.1" );
   CHECK( socket.GetServerPort() == 35346 );

   const std::string actualResponse = socket.GetData();
   const std::string expectedResponse( TEXT_PACKET.data(), TEXT_PACKET_LENGTH );

   CAPTURE( actualResponse );

   REQUIRE( actualResponse.length() == TEXT_PACKET_LENGTH );
   REQUIRE_THAT( actualResponse, Catch::StartsWith( expectedResponse ) );

   CHECK( socket.Shutdown( CSimpleSocket::Both ) );
   CHECK( socket.Close() );
   CHECK_FALSE( socket.IsSocketValid() );

   CHECK( server.Shutdown( CSimpleSocket::Both ) );
   CHECK( server.Close() );
   CHECK_FALSE( server.IsSocketValid() );
}

TEST_CASE( "Sockets can be NIC specific", "[Bind][TCP]" )
{
   CActiveSocket socket;

   REQUIRE( socket.BindInterface( "127.0.0.1" ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   CHECK( socket.GetClientAddr() == "127.0.0.1" );

   SECTION( "Sockets communication while bound" )
   {
      CPassiveSocket server;

      REQUIRE( server.Listen( "127.0.0.1", 26148 ) );
      REQUIRE( server.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.Open( "127.0.0.1", 26148 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      auto serverRespone = std::async( std::launch::async, [&] {
         auto connection = server.Accept();
         REQUIRE( connection != nullptr );
         REQUIRE( connection->GetClientAddr() == "127.0.0.1" );

         uint8_t buffer[ TEXT_PACKET_LENGTH + 1 ];
         REQUIRE( connection->Receive( 1024, buffer ) == TEXT_PACKET_LENGTH );

         CAPTURE( buffer );

         REQUIRE( connection->Send( buffer, connection->GetBytesReceived() ) == TEXT_PACKET_LENGTH );
      } );

      REQUIRE( socket.Send( TEXT_PACKET ) == TEXT_PACKET_LENGTH );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.Receive( 1024 ) == TEXT_PACKET_LENGTH );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.Shutdown( CSimpleSocket::Both ) );
      CHECK( socket.Close() );
      CHECK_FALSE( socket.IsSocketValid() );

      CHECK( server.Shutdown( CSimpleSocket::Both ) );
      CHECK( server.Close() );
      CHECK_FALSE( server.IsSocketValid() );
   }

   SECTION( "Cannot connet to outside network" )
   {
      REQUIRE_FALSE( socket.Open( "www.google.ca", 80 ) );

      int socketError = errno;
      CAPTURE( socketError );

#ifdef _DARWIN
      REQUIRE_FALSE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
#else
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidOperation );
#endif
   }
}

TEST_CASE( "Waiting for connections can be closed", "[TCP][Listen][Accept][Close]" )
{
   CPassiveSocket server;

   REQUIRE( server.Listen( "127.0.0.1", 36748 ) );
   CHECK( server.GetSocketError() == CSimpleSocket::SocketSuccess );

   auto serverRespone = std::async( std::launch::async, [&] {
      REQUIRE( server.Accept() == nullptr );
      return server.GetSocketError();
   } );

   REQUIRE( serverRespone.wait_for( std::chrono::seconds( 10 ) ) == std::future_status::timeout );
   REQUIRE( server.Shutdown( CSimpleSocket::Both ) );

   //
   // This is a behavoiral difference between win_sock and posix sockets =?
   // not sure how to handle this at the API level
   //
#ifdef _WIN32
   REQUIRE( serverRespone.wait_for( std::chrono::seconds( 10 ) ) == std::future_status::timeout );
#elif _DARWIN
   REQUIRE( serverRespone.wait_for( std::chrono::seconds( 10 ) ) == std::future_status::timeout );
#else
   REQUIRE( serverRespone.wait_for( std::chrono::seconds( 10 ) ) == std::future_status::ready );
#endif

   REQUIRE( server.Close() );
   CHECK_FALSE( server.IsSocketValid() );

   REQUIRE( serverRespone.wait_for( std::chrono::seconds( 10 ) ) == std::future_status::ready );

   int socketError = errno;
   CAPTURE( socketError );

#ifdef _WIN32
   // Windows this scenario means a blocking call was canceled...
   REQUIRE( serverRespone.get() == CSimpleSocket::SocketInterrupted );
#elif _DARWIN
   // Different versions of xcode produce different error codes ( sometimes undefined )
   // REQUIRE( serverRespone.get() == CSimpleSocket::SocketConnectionAborted );
   REQUIRE_FALSE( serverRespone.get() == CSimpleSocket::SocketSuccess );
#else
   REQUIRE( serverRespone.get() == CSimpleSocket::SocketInvalidOperation );
#endif
}

TEST_CASE( "Sockets clear buffer on Rx fail", "[Listen][Open][Accept][TCP]" )
{
   CPassiveSocket server;

   REQUIRE( server.Listen( "127.0.0.1", 54981 ) );
   REQUIRE( server.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( server.GetServerAddr() == "127.0.0.1" );
   CHECK( server.GetServerPort() == 54981 );

   CActiveSocket socket;

   REQUIRE( socket.Open( "127.0.0.1", 54981 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   CHECK( socket.GetServerAddr() == "127.0.0.1" );
   CHECK( socket.GetServerPort() == 54981 );

   auto serverRespone = std::async( std::launch::async, [&] {
      std::unique_ptr<CActiveSocket> connection = server.Accept();
      REQUIRE( connection != nullptr );

      REQUIRE( connection->Receive( 1024 ) == TEXT_PACKET_LENGTH );

      std::this_thread::sleep_for( 125ms );

      REQUIRE( connection->Close() );
   } );

   REQUIRE( socket.Send( TEXT_PACKET ) == TEXT_PACKET_LENGTH );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.Receive( 1024 ) == 0 );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   const std::string actualResponse = socket.GetData();

   REQUIRE( actualResponse.length() == 0 );
   REQUIRE( actualResponse.empty() );

   CHECK( socket.Shutdown( CSimpleSocket::Both ) );
   CHECK( socket.Close() );
   CHECK_FALSE( socket.IsSocketValid() );

   CHECK( server.Shutdown( CSimpleSocket::Both ) );
   CHECK( server.Close() );
   CHECK_FALSE( server.IsSocketValid() );
}

TEST_CASE( "Sockets can linger", "[Linger]" )
{
   auto time = GENERATE( range<uint16_t>( 0, 90, 15 ) );
   SECTION( "TCP" )
   {
      CActiveSocket socket;
      REQUIRE( socket.Open( "www.google.ca", 80 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      SECTION( "Enable" )
      {
         REQUIRE( socket.SetOptionLinger( true, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Disable" )
      {
         REQUIRE( socket.SetOptionLinger( false, 0 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Toggle" )
      {
         REQUIRE( socket.SetOptionLinger( false, 0 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.SetOptionLinger( true, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.SetOptionLinger( false, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.SetOptionLinger( true, 50 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      CHECK( socket.Shutdown( CSimpleSocket::Both ) );
      CHECK( socket.Close() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK_FALSE( socket.IsSocketValid() );
   }

   SECTION( "UDP" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );
      REQUIRE( socket.Open( "8.8.8.8", 53 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

#ifdef _LINUX
      SECTION( "Enable" )
      {
         REQUIRE( socket.SetOptionLinger( true, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Disable" )
      {
         REQUIRE( socket.SetOptionLinger( false, 0 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Toggle" )
      {
         REQUIRE( socket.SetOptionLinger( false, 0 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.SetOptionLinger( true, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.SetOptionLinger( false, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

         REQUIRE( socket.SetOptionLinger( true, 50 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }
#elif _WIN32
      SECTION( "Enable" )
      {
         REQUIRE_FALSE( socket.SetOptionLinger( true, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
      }

      SECTION( "Disable" )
      {
         REQUIRE_FALSE( socket.SetOptionLinger( false, 0 ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
      }

      SECTION( "Disable with timeout" )
      {
         REQUIRE_FALSE( socket.SetOptionLinger( false, time ) );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
      }
#endif

      CHECK( socket.Shutdown( CSimpleSocket::Both ) );
      CHECK( socket.Close() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK_FALSE( socket.IsSocketValid() );
   }
}

TEST_CASE( "Sockets can be flushed", "[Flush]" )
{
   SECTION( "TCP" )
   {
      CActiveSocket socket;
      REQUIRE( socket.Open( "www.google.ca", 80 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      CHECK( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.Flush() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }

   SECTION( "UDP" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );
      REQUIRE( socket.Open( "8.8.8.8", 53 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

#ifndef _DARWIN
      CHECK( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
#endif

      REQUIRE_FALSE( socket.Flush() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
   }
}

TEST_CASE( "Sockets can set nagle on/off", "[Nagle]" )
{
   SECTION( "TCP" )
   {
      CActiveSocket socket;
      REQUIRE( socket.Open( "www.google.ca", 80 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      SECTION( "Disable" )
      {
         REQUIRE( socket.DisableNagleAlgoritm() );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      SECTION( "Disable" )
      {
         REQUIRE( socket.EnableNagleAlgoritm() );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      }

      CHECK( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( socket.Flush() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }

   SECTION( "UDP" )
   {
      CActiveSocket socket( CSimpleSocket::SocketTypeUdp );
      REQUIRE( socket.Open( "8.8.8.8", 53 ) );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      SECTION( "Disable" )
      {
         REQUIRE_FALSE( socket.DisableNagleAlgoritm() );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
      }

      SECTION( "Disable" )
      {
         REQUIRE_FALSE( socket.EnableNagleAlgoritm() );
         CHECK( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
      }

#ifndef _DARWIN
      CHECK( socket.Send( DNS_QUERY, DNS_QUERY_LENGTH ) == DNS_QUERY_LENGTH );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
#endif

      REQUIRE_FALSE( socket.Flush() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketProtocolError );
   }
}
