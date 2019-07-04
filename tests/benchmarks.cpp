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

#define CATCH_CONFIG_ENABLE_CHRONO_STRINGMAKER
#define CATCH_CONFIG_ENABLE_BENCHMARKING
#include "catch2/catch.hpp"
#include "PassiveSocket.h"

#include <future>

class benchmark_socket : CActiveSocket
{
public:
   explicit benchmark_socket( CSocketType type = SocketTypeTcp ) : CActiveSocket( type ) {}

   using CActiveSocket::Open;
   using CSimpleSocket::Close;

   // Benchmark Results:
   // TCP: ~2us
   // UDP: ~11us
   int32_t sendOriginal( const uint8_t* pBuf, size_t bytesToSend )
   {
      if ( !IsSocketValid() || bytesToSend == 0 || pBuf == nullptr )
      {
         SetSocketError( IsSocketValid() ? SocketInvalidPointer : SocketInvalidSocket );
         m_nBytesSent = SocketError;
         return m_nBytesSent;
      }

      SetSocketError( SocketSuccess );
      m_nBytesSent = 0;

      std::function<int32_t()> sendMessage = [] { return -1; };
      if ( m_nSocketType == SocketTypeTcp )
         sendMessage = [&] { return SEND( m_socket, pBuf, bytesToSend, 0 ); };
      else if ( m_nSocketType == SocketTypeUdp )
         sendMessage = [&] {
            const auto addrToSentTo = reinterpret_cast<const sockaddr*>( GetUdpTxAddrBuffer() );
            return SENDTO( m_socket, pBuf, bytesToSend, 0, addrToSentTo, SOCKET_ADDR_IN_SIZE );
         };

      m_timer.SetStartTime();

      // Check error condition and attempt to resend if call was interrupted by a signal.
      do
      {
         m_nBytesSent += sendMessage();
         TranslateSocketError();
      } while ( GetSocketError() == CSimpleSocket::SocketInterrupted );

      m_timer.SetEndTime();

      return m_nBytesSent;
   }

   // Benchmark Results:
   // TCP: ~2us
   // UDP: ~11us
   int32_t noLoop( const uint8_t* pBuf, size_t bytesToSend )
   {
      if ( !IsSocketValid() || bytesToSend == 0 || pBuf == nullptr )
      {
         SetSocketError( IsSocketValid() ? SocketInvalidPointer : SocketInvalidSocket );
         m_nBytesSent = SocketError;
         return m_nBytesSent;
      }

      SetSocketError( SocketSuccess );
      m_nBytesSent = 0;

      std::function<int32_t()> sendMessage = [] { return -1; };
      if ( m_nSocketType == SocketTypeTcp )
         sendMessage = [&] { return SEND( m_socket, pBuf, bytesToSend, 0 ); };
      else if ( m_nSocketType == SocketTypeUdp )
         sendMessage = [&] {
            const auto addrToSentTo = reinterpret_cast<const sockaddr*>( GetUdpTxAddrBuffer() );
            return SENDTO( m_socket, pBuf, bytesToSend, 0, addrToSentTo, SOCKET_ADDR_IN_SIZE );
         };

      m_timer.SetStartTime();

      m_nBytesSent = sendMessage();
      TranslateSocketError();

      m_timer.SetEndTime();

      return m_nBytesSent;
   }
   // Benchmark Results:
   // TCP: ~2us
   // UDP: ~11us
   int32_t noTimer( const uint8_t* pBuf, size_t bytesToSend )
   {
      if ( !IsSocketValid() || bytesToSend == 0 || pBuf == nullptr )
      {
         SetSocketError( IsSocketValid() ? SocketInvalidPointer : SocketInvalidSocket );
         m_nBytesSent = SocketError;
         return m_nBytesSent;
      }

      SetSocketError( SocketSuccess );
      m_nBytesSent = 0;

      std::function<int32_t()> sendMessage = [] { return -1; };
      if ( m_nSocketType == SocketTypeTcp )
         sendMessage = [&] { return SEND( m_socket, pBuf, bytesToSend, 0 ); };
      else if ( m_nSocketType == SocketTypeUdp )
         sendMessage = [&] {
            const auto addrToSentTo = reinterpret_cast<const sockaddr*>( GetUdpTxAddrBuffer() );
            return SENDTO( m_socket, pBuf, bytesToSend, 0, addrToSentTo, SOCKET_ADDR_IN_SIZE );
         };

      // Check error condition and attempt to resend if call was interrupted by a signal.
      do
      {
         m_nBytesSent += sendMessage();
         TranslateSocketError();
      } while ( GetSocketError() == CSimpleSocket::SocketInterrupted );

      return m_nBytesSent;
   }
   // Benchmark Results:
   // TCP: n/a
   // UDP: ~9.3us
   int32_t pureUdp( const uint8_t* pBuf, size_t bytesToSend )
   {
      const auto addrToSentTo = reinterpret_cast<const sockaddr*>( GetUdpTxAddrBuffer() );
      m_nBytesSent = SENDTO( m_socket, pBuf, bytesToSend, 0, addrToSentTo, SOCKET_ADDR_IN_SIZE );
      TranslateSocketError();

      return m_nBytesSent;
   }
};

TEST_CASE( "socket send", "[.][Benchmark][TCP][UDP]" )
{
   static constexpr uint8_t MSG[] = { 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd' };
   static constexpr auto MSG_LENGTH = ( sizeof( MSG ) / sizeof( MSG[ 0 ] ) );

   SECTION( "TCP" )
   {
      CPassiveSocket server;
      REQUIRE( server.Listen( nullptr, 35346 ) );
      benchmark_socket socket;
      auto remote = std::async( std::launch::async, [&] { return server.Accept(); } );
      REQUIRE( socket.Open( "127.0.0.1", 35346 ) );
      auto connection = remote.get();

      BENCHMARK( "original" ) { socket.sendOriginal( MSG, MSG_LENGTH ); };
      BENCHMARK( "no loop" ) { socket.noLoop( MSG, MSG_LENGTH ); };
      // BENCHMARK( "no timer" ) { socket.noTimer( MSG, MSG_LENGTH ); };

      CHECK( socket.Close() );
   }

#ifndef _DARWIN
   SECTION( "UDP" )
   {
      benchmark_socket socket( CSimpleSocket::SocketTypeUdp );
      REQUIRE( socket.Open( "127.0.0.1", 12345 ) );

      BENCHMARK( "original" ) { socket.sendOriginal( MSG, MSG_LENGTH ); };
      BENCHMARK( "no loop" ) { socket.noLoop( MSG, MSG_LENGTH ); };
      BENCHMARK( "no timer" ) { socket.noTimer( MSG, MSG_LENGTH ); };
      BENCHMARK( "pure UDP" ) { socket.pureUdp( MSG, MSG_LENGTH ); };

      CHECK( socket.Close() );
   }
#endif
}
