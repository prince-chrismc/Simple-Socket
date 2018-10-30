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
#include "catch2/catch.hpp"

#include <future>
#include "PassiveSocket.h"       // Include header for active socket object definition

#define MAX_PACKET 4096
#define TEST_PACKET "Test Packet"

TEST_CASE( "Sockets are created", "[Socket.Initialize]" )
 {
     CSimpleSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.GetSocketDescriptor() != INVALID_SOCKET );
}

TEST_CASE( "Sockets can send", "[Socket.Send]" )
 {
     CActiveSocket socket(CSimpleSocket::SocketTypeUdp);

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.Open("8.8.8.8", 53 ) );

    std::string dnsQuery = "\xAA\xAA\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\
        \x07\x65\x78\x61\x6d\x70\x6c\x65\x03\x63\x6f\x6d\x00\x00\x01\x00\x01";

    REQUIRE( socket.Send( reinterpret_cast<const uint8*>( dnsQuery.c_str() ), dnsQuery.length() ) );
}

TEST_CASE( "Sockets can receive", "[Socket.Receive]" )
 {
     CActiveSocket socket;

    REQUIRE( socket.Initialize() );
    REQUIRE( socket.Open("www.google.ca", 80 ) );

    std::string httpRequest = "GET / HTTP/1.0\r\n\r\n";

    REQUIRE( socket.Send( reinterpret_cast<const uint8*>( httpRequest.c_str() ), httpRequest.length() ) );

    REQUIRE ( socket.Receive( 17 ) );

    std::string httpResponse = socket.GetData();

    REQUIRE( httpResponse.length() > 0 );
    REQUIRE( httpResponse.compare("HTTP/1.0 200 OK\r\n") == 0 );
}
