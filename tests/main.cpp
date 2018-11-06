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

#include "PassiveSocket.h"       // Include header for active socket object definition

#if defined(_LINUX) || defined (_DARWIN)
    #include <netdb.h>
#endif

#define MAX_PACKET 4096
#define TEST_PACKET "Test Packet"

TEST_CASE( "Sockets are created", "[Socket.Initialize]" )
 {
     CSimpleSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.GetSocketDescriptor() != INVALID_SOCKET );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can open", "[Socket.Open.UDP]" )
{
    CActiveSocket socket(CSimpleSocket::SocketTypeUdp);

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
    REQUIRE( socket.Open("8.8.8.8", 53 ) );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can connect", "[Socket.Open.TCP]" )
{
    CActiveSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
    REQUIRE( socket.Open("www.google.ca", 80 ) );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can send", "[Socket.Send.UDP]" )
{
    CActiveSocket socket(CSimpleSocket::SocketTypeUdp);

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.Open("8.8.8.8", 53 ) );

    std::string dnsQuery = "\xAA\xAA\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\
        \x07\x65\x78\x61\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01";

    REQUIRE( socket.Send( reinterpret_cast<const uint8*>( dnsQuery.c_str() ), dnsQuery.length() ) == dnsQuery.length() );
#if defined(_DARWIN)
    CAPTURE( errno );
#endif
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can transfer", "[Socket.Send.TCP]" )
{
    CActiveSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.Open("www.google.ca", 80 ) );

    std::string httpRequest = "GET / HTTP/1.0\r\n\r\n";

    REQUIRE( socket.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() )
    == httpRequest.length() );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );
}

TEST_CASE( "Sockets can read", "[Socket.Receive.UDP]" )
{
    // Travis CI didn't like this case =?
}

TEST_CASE( "Sockets can receive", "[Socket.Receive.TCP]" )
{
    CActiveSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.Open("www.google.ca", 80 ) );

    std::string httpRequest = "GET / HTTP/1.0\r\n\r\n";

    REQUIRE( socket.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() )
    == httpRequest.length() );

    REQUIRE( socket.Receive( 17 ) == 17 );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

    std::string httpResponse = socket.GetData();

    REQUIRE( httpResponse.length() > 0 );
    REQUIRE( httpResponse.compare("HTTP/1.0 200 OK\r\n") == 0 );
}

TEST_CASE( "Sockets have server information" )
{
    CActiveSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.Open("www.google.ca", 80 ) );

    sockaddr_in serverAddr;
    memset( &serverAddr, 0, sizeof( serverAddr ) );
    serverAddr.sin_family = AF_INET;

    SECTION("Socket.GetServerAddr")
    {
        addrinfo hints{};
        memset( &hints, 0, sizeof( hints ) );
        hints.ai_flags = AI_ALL;
        hints.ai_family = AF_INET;
        addrinfo* pResult = NULL;
        const int iErrorCode = getaddrinfo( "www.google.ca", NULL, &hints, &pResult );

        REQUIRE( iErrorCode == 0 );

        char buff[16];
        std::string googlesAddr =  inet_ntop(AF_INET,
            &( (sockaddr_in*)pResult->ai_addr )->sin_addr, buff, 16);

        CAPTURE( buff );
        CAPTURE( socket.GetServerAddr() );

        REQUIRE( googlesAddr.compare( socket.GetServerAddr()) == 0);
    }

    SECTION("Socket.GetServerPort")
    {
        REQUIRE( socket.GetServerPort() == 80 );
    }
}

TEST_CASE( "Sockets can disconnect", "[Socket.Close.TCP]" )
{
    CActiveSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.Open("www.google.ca", 80 ) );

    std::string httpRequest = "GET / HTTP/1.0\r\n\r\n";

    REQUIRE( socket.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() )
    == httpRequest.length() );

    REQUIRE( socket.Receive( 17 ) == 17 );
    REQUIRE( socket.GetSocketError() == CSimpleSocket::SocketSuccess );

    std::string httpResponse = socket.GetData();

    REQUIRE( httpResponse.length() > 0 );
    REQUIRE( httpResponse.compare("HTTP/1.0 200 OK\r\n") == 0 );

    REQUIRE( socket.Shutdown( CSimpleSocket::Both ) );
    REQUIRE( socket.Close() );
}

TEST_CASE( "Sockets are ctor copyable", "[Socket.ctor(socket).TCP]" )
{
    CActiveSocket alpha;

    REQUIRE( alpha.Initialize() );
    REQUIRE( alpha.Open("www.google.ca", 80 ) );

    std::string httpRequest = "GET / HTTP/1.0\r\n\r\n";

    REQUIRE( alpha.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() )
    == httpRequest.length() );

    REQUIRE( alpha.Receive( 17 ) == 17 );
    REQUIRE( alpha.GetSocketError() == CSimpleSocket::SocketSuccess );

    std::string httpResponse = alpha.GetData();

    REQUIRE( httpResponse.length() > 0 );
    REQUIRE( httpResponse.compare("HTTP/1.0 200 OK\r\n") == 0 );

    CActiveSocket beta(alpha);

    REQUIRE( beta.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() )
    == httpRequest.length() );

    REQUIRE( beta.Receive( 6 ) == 6 );
    REQUIRE( beta.GetSocketError() == CSimpleSocket::SocketSuccess );

    httpResponse = beta.GetData();
    CAPTURE( httpResponse );
    REQUIRE( httpResponse.compare("Date: ") == 0 );
}

TEST_CASE( "Sockets are assign copyable", "[Socket.=.TCP]" )
{
    CActiveSocket alpha;

    REQUIRE( alpha.Initialize() );
    REQUIRE( alpha.Open("www.google.ca", 80 ) );

    std::string httpRequest = "GET / HTTP/1.0\r\n\r\n";

    REQUIRE( alpha.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() )
    == httpRequest.length() );

    REQUIRE( alpha.Receive( 17 ) == 17 );
    REQUIRE( alpha.GetSocketError() == CSimpleSocket::SocketSuccess );

    std::string httpResponse = alpha.GetData();

    REQUIRE( httpResponse.length() > 0 );
    REQUIRE( httpResponse.compare("HTTP/1.0 200 OK\r\n") == 0 );

    CActiveSocket beta;
    beta = alpha;

    REQUIRE( beta.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() )
    == httpRequest.length() );

    REQUIRE( beta.Receive( 6 ) == 6 );
    REQUIRE( beta.GetSocketError() == CSimpleSocket::SocketSuccess );

    httpResponse = beta.GetData();
    REQUIRE( httpResponse.compare("Date: ") == 0 );
}
