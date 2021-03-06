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

#include "catch2/catch.hpp"
#include "ActiveSocket.h"

using namespace std::string_view_literals;
static constexpr auto HTTP_GET_ROOT_REQUEST = "GET / HTTP/1.0\r\n\r\n"sv;

TEST_CASE( "Sockets can be set to non-blocking", "[Initialization]" )
{
   SECTION( "Non-blocking TCP sockets", "[Async][TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );
   }

   SECTION( "Non-blocking UDP sockets", "[Async][UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );
   }

   SECTION( "Toggle async mode on TCP sockets", "[Async][TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );

      REQUIRE( socket.SetBlocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE_FALSE( socket.IsNonblocking() );

      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );
   }

   SECTION( "Toggle async mode on UDP sockets", "[Async][UDP]" )
   {
      CSimpleSocket socket( CSimpleSocket::SocketTypeUdp );

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeUdp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );

      REQUIRE( socket.SetBlocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE_FALSE( socket.IsNonblocking() );

      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );
   }

   SECTION( "Invalid socket can not be set to non-blocking", "[Async][TCP]" )
   {
      CSimpleSocket socket;
      CSimpleSocket secondary = std::move( socket );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE_FALSE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
      REQUIRE_FALSE( socket.IsNonblocking() );
   }

   SECTION( "Invalid socket can not be set to blocking", "[Async][TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );

      CSimpleSocket secondary = std::move( socket );

      REQUIRE_FALSE( socket.SetBlocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketInvalidSocket );
      REQUIRE_FALSE( socket.IsNonblocking() );
   }

   SECTION( "Toggle async mode on moved socket", "[Async][TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );

      CSimpleSocket secondary = std::move( socket );
      REQUIRE( secondary.IsNonblocking() );
      REQUIRE( secondary.GetSocketError() == CSimpleSocket::SocketSuccess );

      REQUIRE( secondary.SetBlocking() );
      REQUIRE( secondary.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE_FALSE( secondary.IsNonblocking() );

      REQUIRE( secondary.SetNonblocking() );
      REQUIRE( secondary.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( secondary.IsNonblocking() );
   }

   SECTION( "Double set Non-blocking TCP sockets", "[Async][TCP]" )
   {
      CSimpleSocket socket;

      CHECK( socket.IsSocketValid() );
      CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

      REQUIRE_FALSE( socket.IsNonblocking() );
      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );

      REQUIRE( socket.SetNonblocking() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
      REQUIRE( socket.IsNonblocking() );
   }
}

TEST_CASE( "Non-blocking Sockets can connect", "[TCP][Async][Open]" )
{
   CActiveSocket socket;

   CHECK( socket.IsSocketValid() );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

   REQUIRE_FALSE( socket.IsNonblocking() );
   REQUIRE( socket.SetNonblocking() );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   REQUIRE( socket.IsNonblocking() );

   SECTION( "Connection Timeout" )
   {
      REQUIRE_FALSE( socket.Open( "www.google.ca", 34867 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketTimedout );
   }

   SECTION( "Connection Refused" )
   {
      REQUIRE_FALSE( socket.Open( "127.0.0.1", 34867 ) );

#ifdef _WIN32
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketTimedout );
#elif _DARWIN
      // For xcode 10.0
      // REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketTimedout );

      // For xcode 9.4
      // REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketConnectionRefused );

      REQUIRE_FALSE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
#else
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketConnectionRefused );
#endif
   }

   SECTION( "To Google Timeout" )
   {
      REQUIRE( socket.GetConnectTimeoutSec() == 0 );
      REQUIRE( socket.GetConnectTimeoutUSec() == 0 );

      REQUIRE_FALSE( socket.Open( "www.google.ca", 80 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketTimedout );
   }

   SECTION( "To Google" )
   {
      socket.SetConnectTimeout( 5, 500 );   // Allow enough time to establish connections

      REQUIRE( socket.GetConnectTimeoutSec() == 5 );
      REQUIRE( socket.GetConnectTimeoutUSec() == 500 );

      REQUIRE( socket.Open( "www.google.ca", 80 ) );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }
}

TEST_CASE( "Non-blocking Sockets can send", "[TCP][Async][Open][Send]" )
{
   CActiveSocket socket;

   CHECK( socket.IsSocketValid() );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

   REQUIRE_FALSE( socket.IsNonblocking() );
   REQUIRE( socket.SetNonblocking() );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   REQUIRE( socket.IsNonblocking() );

   socket.SetConnectTimeout( 5, 500 );   // Allow enough time to establish connections

   CHECK( socket.GetConnectTimeoutSec() == 5 );
   CHECK( socket.GetConnectTimeoutUSec() == 500 );

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   SECTION( "Send instant" )
   {
      REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

      CAPTURE( "Send (ms)", socket.GetTotalTimeMs(), "Send (us)", socket.GetTotalTimeUsec() );
      CHECK( socket.GetTotalTimeMs() == 0 );

      REQUIRE( socket.GetBytesSent() == HTTP_GET_ROOT_REQUEST.length() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }

   SECTION( "Send 2.5s timeout" )
   {
      REQUIRE( socket.SetSendTimeout( 2, 500 ) );

      REQUIRE( socket.GetSendTimeoutSec() == 2 );
      REQUIRE( socket.GetSendTimeoutUSec() == 500 );

      REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

      CAPTURE( "Send (ms)", socket.GetTotalTimeMs(), "Send (us)", socket.GetTotalTimeUsec() );
      CHECK( socket.GetTotalTimeMs() == 0 );

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

      REQUIRE( socket.GetBytesSent() == HTTP_GET_ROOT_REQUEST.length() );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   }
}

TEST_CASE( "Non-blocking Sockets can receive", "[TCP][Async][Open][Send][Receive]" )
{
   CActiveSocket socket;

   CHECK( socket.IsSocketValid() );
   CHECK( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   CHECK( socket.GetSocketType() == CSimpleSocket::SocketTypeTcp );

   REQUIRE_FALSE( socket.IsNonblocking() );
   REQUIRE( socket.SetNonblocking() );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
   REQUIRE( socket.IsNonblocking() );

   socket.SetConnectTimeout( 5, 500 );   // Allow enough time to establish connections

   CHECK( socket.GetConnectTimeoutSec() == 5 );
   CHECK( socket.GetConnectTimeoutUSec() == 500 );

   REQUIRE( socket.Open( "www.google.ca", 80 ) );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   REQUIRE( socket.SetSendTimeout( 1, 0 ) );

   REQUIRE( socket.GetSendTimeoutSec() == 1 );
   REQUIRE( socket.GetSendTimeoutUSec() == 0 );

   REQUIRE( socket.Send( HTTP_GET_ROOT_REQUEST ) == HTTP_GET_ROOT_REQUEST.length() );

   REQUIRE( socket.GetBytesSent() == HTTP_GET_ROOT_REQUEST.length() );
   REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

   SECTION( "Rx instant" )
   {
      REQUIRE( socket.Receive( 1368 ) == -1 );
      REQUIRE( socket.GetBytesReceived() == -1 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketEwouldblock );

      const std::string httpResponse = socket.GetData();
      CAPTURE( httpResponse );

      REQUIRE( httpResponse.length() == 0 );
      REQUIRE( httpResponse == "" );
   }

   SECTION( "Rx 0.5ms" )
   {
      REQUIRE( socket.SetReceiveTimeout(0, 500));

      CHECK( socket.GetReceiveTimeoutSec() == 0);
      CHECK( socket.GetReceiveTimeoutUSec() == 500);

      CAPTURE( "Receive (ms)", socket.GetTotalTimeMs(), "Receive (us)", socket.GetTotalTimeUsec() );
      CHECK( socket.GetTotalTimeMs() == 0 );

      REQUIRE( socket.Receive( 1368 ) == -1 );
      REQUIRE( socket.GetBytesReceived() == -1 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketEwouldblock );

      const std::string httpResponse = socket.GetData();
      CAPTURE( httpResponse );

      REQUIRE( httpResponse.length() == 0 );
      REQUIRE( httpResponse == "" );
   }

   SECTION( "Rx 2.5ms" )
   {
      REQUIRE( socket.SetReceiveTimeout(2, 500));

      CHECK( socket.GetReceiveTimeoutSec() == 2);
      CHECK( socket.GetReceiveTimeoutUSec() == 500);

      CAPTURE( "Receive (ms)", socket.GetTotalTimeMs(), "Receive (us)", socket.GetTotalTimeUsec() );
      CHECK( socket.GetTotalTimeMs() == 0 );

      REQUIRE( socket.Receive( 1368 ) == -1 );
      REQUIRE( socket.GetBytesReceived() == -1 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketEwouldblock );

      const std::string httpResponse = socket.GetData();
      CAPTURE( httpResponse );

      REQUIRE( httpResponse.length() == 0 );
      REQUIRE( httpResponse == "" );
   }

   SECTION( "Rx with select" )
   {
      REQUIRE( socket.Receive( 1368 ) == -1 );
      REQUIRE( socket.GetBytesReceived() == -1 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketEwouldblock );

      REQUIRE( socket.Select() );

      REQUIRE( socket.Receive( 1368 ) == -1 );
      REQUIRE( socket.GetBytesReceived() == -1 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketEwouldblock );

      const std::string httpResponse = socket.GetData();
      CAPTURE( httpResponse );

      REQUIRE( httpResponse.length() == 0 );
      REQUIRE( httpResponse == "" );
      
      /*REQUIRE( socket.Receive( 1368 ) == 1368 );
      REQUIRE( socket.GetBytesReceived() == 1368 );
      REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

      const std::string httpResponse = socket.GetData();

      REQUIRE( httpResponse.length() > 0 );
      REQUIRE_THAT( httpResponse,
                    Catch::StartsWith( "HTTP/1.0 200 OK\r\n" ) && Catch::Contains( "\r\n\r\n<!doctype html>" ) );*/
   }
}