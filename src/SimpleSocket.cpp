/*---------------------------------------------------------------------------*/
/*                                                                           */
/* CSimpleSocket.cpp - CSimpleSocket Implementation                          */
/*                                                                           */
/* Author : Mark Carrier (mark@carrierlabs.com)                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/* Copyright (c) 2007-2009 CarrierLabs, LLC.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * 4. The name "CarrierLabs" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    mark@carrierlabs.com.
 *
 * THIS SOFTWARE IS PROVIDED BY MARK CARRIER ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL MARK CARRIER OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *----------------------------------------------------------------------------*/

#include "SimpleSocket.h"

#include <cstdlib>
#include <functional>
#include <stdexcept>

#if defined( _LINUX ) || defined( _DARWIN )
#include <fcntl.h>
#include <netinet/ip.h>
#endif

#ifdef _WIN32
#include <Ws2tcpip.h>
#include <io.h>
static constexpr auto IPTOS_LOWDELAY = 0x10;
#endif

static constexpr auto SOCKET_SENDFILE_BLOCKSIZE = 8192;

CSimpleSocket::CSimpleSocket( CSocketType nType ) :
   m_socket( INVALID_SOCKET ),
   m_socketErrno( CSimpleSocket::SocketInvalidSocket ),
   m_sBuffer( 48, '\0' ),
   m_nBufferSize( 48 ),
   m_nSocketDomain( AF_UNSPEC ),
   m_nSocketType( nType ),
   m_nBytesReceived( -1 ),
   m_nBytesSent( -1 ),
   m_nFlags( 0 ),
   m_bIsBlocking( true ),
   m_bIsMulticast( false )
{
   SetConnectTimeout( 1, 0 );
   SetReceiveTimeout( 0, 0 );
   SetSendTimeout( 0, 0 );
   SetOptionLinger( false, 0 );
   memset( &m_stClientSockaddr, 0, SOCKET_ADDR_IN_SIZE );
   memset( &m_stServerSockaddr, 0, SOCKET_ADDR_IN_SIZE );
   memset( &m_stMulticastGroup, 0, SOCKET_ADDR_IN_SIZE );

   switch ( nType )
   {
   case CSimpleSocket::SocketTypeTcp:
   case CSimpleSocket::SocketTypeUdp:
      m_nSocketDomain = AF_INET;
      break;
      // case CSimpleSocket::SocketTypeTcp6:
      // case CSimpleSocket::SocketTypeUdp6:
      //   m_nSocketDomain = AF_INET6;
      //   break;

      // Declare socket type raw Ethernet - Ethernet
      //#if defined( _LINUX ) || defined( _DARWIN )
      //   case CSimpleSocket::SocketTypeRaw:
      //      // case CSimpleSocket::SocketTypeRaw6:
      //      m_nSocketDomain = AF_PACKET;
      //      break;
      //#else
      //   case CSimpleSocket::SocketTypeRaw:
      //      m_nSocketDomain = AF_INET;
      //      break;
      //      // case CSimpleSocket::SocketTypeRaw6:
      //      //   m_nSocketDomain = AF_INET6;
      //      //   break;
      //#endif

   default:
      m_nSocketType = CSimpleSocket::SocketTypeInvalid;
      break;
   }

   if ( !Initialize() )
   {
      throw std::runtime_error( "Failed to create socket! " + DescribeError() );
   }
}

CSimpleSocket::CSimpleSocket( CSimpleSocket&& socket ) noexcept :
   m_socket( INVALID_SOCKET ),
   m_socketErrno( CSimpleSocket::SocketInvalidSocket ),
   m_nBufferSize( 0 ),
   m_nSocketDomain( AF_UNSPEC ),
   m_nSocketType( CSimpleSocket::SocketTypeInvalid ),
   m_nBytesReceived( -1 ),
   m_nBytesSent( -1 ),
   m_nFlags( 0 ),
   m_bIsBlocking( true ),
   m_bIsMulticast( false )
{
   swap( *this, socket );
   return;
}

CSimpleSocket& CSimpleSocket::operator=( CSimpleSocket&& other ) noexcept
{
   swap( *this, other );
   return *this;
}

CSimpleSocket::~CSimpleSocket()
{
   Close();   // Checks internally if socket is valid
   // TO DO DEBATE: Should this terminate if Close failed?
}

void swap( CSimpleSocket& lhs, CSimpleSocket& rhs ) noexcept
{
   // enable ADL (not necessary in our case, but good practice)
   using std::swap;

   swap( lhs.m_socket, rhs.m_socket );
   swap( lhs.m_socketErrno, rhs.m_socketErrno );
   swap( lhs.m_sBuffer, rhs.m_sBuffer );
   swap( lhs.m_nBufferSize, rhs.m_nBufferSize );
   swap( lhs.m_nSocketDomain, rhs.m_nSocketDomain );
   swap( lhs.m_nSocketType, rhs.m_nSocketType );
   swap( lhs.m_nBytesReceived, rhs.m_nBytesReceived );
   swap( lhs.m_nBytesSent, rhs.m_nBytesSent );
   swap( lhs.m_nFlags, rhs.m_nFlags );
   swap( lhs.m_bIsMulticast, rhs.m_bIsMulticast );
   swap( lhs.m_bIsBlocking, rhs.m_bIsBlocking );

   swap( lhs.m_stConnectTimeout.tv_sec, rhs.m_stConnectTimeout.tv_sec );
   swap( lhs.m_stConnectTimeout.tv_usec, rhs.m_stConnectTimeout.tv_usec );
   swap( lhs.m_stRecvTimeout.tv_sec, rhs.m_stRecvTimeout.tv_sec );
   swap( lhs.m_stRecvTimeout.tv_usec, rhs.m_stRecvTimeout.tv_usec );
   swap( lhs.m_stSendTimeout.tv_sec, rhs.m_stSendTimeout.tv_sec );
   swap( lhs.m_stSendTimeout.tv_usec, rhs.m_stSendTimeout.tv_usec );

   swap( lhs.m_stLinger.l_onoff, rhs.m_stLinger.l_onoff );
   swap( lhs.m_stLinger.l_linger, rhs.m_stLinger.l_linger );

   swap( lhs.m_stClientSockaddr, rhs.m_stClientSockaddr );
   swap( lhs.m_stServerSockaddr, rhs.m_stServerSockaddr );
   swap( lhs.m_stMulticastGroup, rhs.m_stMulticastGroup );
}

//-------------------------------------------------------------------------------------------------
//
// Initialize() - Initialize socket class
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::Initialize()
{
   errno = CSimpleSocket::SocketSuccess;

#ifdef _WIN32
   // Data structure containing general Windows Sockets Info
   memset( &m_hWSAData, 0, sizeof( m_hWSAData ) );
   WSAStartup( MAKEWORD( 2, 2 ), &m_hWSAData );
#endif

   m_timer.SetStartTime();

   // TO DO : Zero may not work with SOCK_RAW
   m_socket = socket( m_nSocketDomain, m_nSocketType, 0 );   // Create the basic Socket Handle

   m_timer.SetEndTime();

   TranslateSocketError();

   return IsSocketValid();
}

//-------------------------------------------------------------------------------------------------
//
// BindInterface()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::BindInterface( const char* pInterface )
{
   return GetMulticast() ? BindMulticastInterface( pInterface ) : BindUnicastInterface( pInterface );
}

bool CSimpleSocket::BindUnicastInterface( const char* pInterface )
{
   bool bRetVal = ( pInterface != nullptr && pInterface[ 0 ] != '\0' );

   if ( bRetVal )
   {
      sockaddr_in stInterfaceAddr{};
      // Set up the sockaddr structure
      stInterfaceAddr.sin_family = static_cast<decltype( m_stServerSockaddr.sin_family )>( m_nSocketDomain );
      inet_pton( m_nSocketDomain, pInterface, &stInterfaceAddr.sin_addr.s_addr );
      stInterfaceAddr.sin_port = 0;

      // Bind the socket using the such that it only use a specified interface
      bRetVal = ( BIND( m_socket, &stInterfaceAddr, SOCKET_ADDR_IN_SIZE ) == SocketSuccess );
      TranslateSocketError();
   }

   return bRetVal;
}

bool CSimpleSocket::BindMulticastInterface( const char* pInterface )
{
   bool bRetVal = ( pInterface != nullptr && pInterface[ 0 ] != '\0' );

   if ( bRetVal )
   {
      in_addr stInterfaceAddr{};
      memset( &stInterfaceAddr, 0, sizeof( stInterfaceAddr ) );
      inet_pton( m_nSocketDomain, pInterface, &stInterfaceAddr.s_addr );

      bRetVal = ( SETSOCKOPT( m_socket, IPPROTO_IP, IP_MULTICAST_IF, &stInterfaceAddr, sizeof( stInterfaceAddr ) ) ==
                  SocketSuccess );
      TranslateSocketError();
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// SetMulticast()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetMulticast( bool bEnable, uint8_t multicastTTL )
{
   bool bRetVal = false;

   if ( GetSocketType() == CSimpleSocket::SocketTypeUdp )
   {
      m_bIsMulticast = bEnable;
      bRetVal = ( SETSOCKOPT( m_socket, IPPROTO_IP, IP_MULTICAST_TTL, &multicastTTL, sizeof( multicastTTL ) ) ==
                  SocketSuccess );
      TranslateSocketError();
   }
   else
   {
      m_socketErrno = CSimpleSocket::SocketProtocolError;
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// JoinMulticast()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::JoinMulticast( const char* pGroup, uint16_t nPort )
{
   bool bRetVal = true;

   if ( GetSocketType() != CSimpleSocket::SocketTypeUdp || !GetMulticast() )
   {
      bRetVal = false;
      m_socketErrno = CSimpleSocket::SocketProtocolError;
   }

   if ( bRetVal )
   {
      bRetVal = SetOptionReuseAddr();   // Don't block the multicast address
   }

   if ( bRetVal )
   {
      memset( &m_stMulticastGroup, 0, sizeof( m_stMulticastGroup ) );
      m_stMulticastGroup.sin_family = AF_INET;
      m_stMulticastGroup.sin_addr.s_addr = htonl( INADDR_ANY );
      m_stMulticastGroup.sin_port = htons( nPort );

      // Bind to the specified port
      bRetVal = ( BIND( m_socket, &m_stMulticastGroup, SOCKET_ADDR_IN_SIZE ) == SocketSuccess );
   }

   if ( bRetVal )
   {
      ip_mreq stMulticastRequest{};

      inet_pton( m_nSocketDomain, pGroup, &stMulticastRequest.imr_multiaddr.s_addr );
      stMulticastRequest.imr_interface.s_addr = htonl( INADDR_ANY );

      // Join the multicast group
      bRetVal =
          ( SETSOCKOPT( m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &stMulticastRequest, sizeof( stMulticastRequest ) ) ==
            SocketSuccess );
   }

   if ( bRetVal )
   {
      // Save group address
      inet_pton( m_nSocketDomain, pGroup, &m_stMulticastGroup.sin_addr.s_addr );

      // Save local info
      socklen_t nSockLen = SOCKET_ADDR_IN_SIZE;
      memset( &m_stClientSockaddr, 0, SOCKET_ADDR_IN_SIZE );
      GETSOCKNAME( m_socket, &m_stClientSockaddr, &nSockLen );
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// GetClientAddr()
//
//-------------------------------------------------------------------------------------------------
std::string CSimpleSocket::GetClientAddr()
{
   char buff[ 16 ] = { '\0' };

   if ( inet_ntop( m_nSocketDomain, &m_stClientSockaddr.sin_addr, buff, 16 ) == nullptr )
   {
      TranslateSocketError();
      return DescribeError();
   }

   return buff;
}

//-------------------------------------------------------------------------------------------------
//
// GetServerAddr()
//
//-------------------------------------------------------------------------------------------------
std::string CSimpleSocket::GetServerAddr()
{
   char buff[ 16 ] = { '\0' };

   if ( inet_ntop( m_nSocketDomain, &m_stServerSockaddr.sin_addr, buff, 16 ) == nullptr )
   {
      TranslateSocketError();
      return DescribeError();
   }

   return buff;
}

//-------------------------------------------------------------------------------------------------
//
// GetJoinedGroup()
//
//-------------------------------------------------------------------------------------------------
std::string CSimpleSocket::GetJoinedGroup()
{
   char buff[ 16 ] = { '\0' };

   if ( inet_ntop( m_nSocketDomain, &m_stMulticastGroup.sin_addr, buff, 16 ) == nullptr )
   {
      TranslateSocketError();
      return DescribeError();
   }

   return buff;
}

//-------------------------------------------------------------------------------------------------
//
// GetSocketDscp()
//
//-------------------------------------------------------------------------------------------------
int32_t CSimpleSocket::GetSocketDscp()
{
   int32_t nTempVal = 0;
   socklen_t nLen = 0;

   if ( IsSocketValid() )
   {
      if ( GETSOCKOPT( m_socket, IPPROTO_IP, IP_TOS, &nTempVal, &nLen ) == SocketError )
      {
         TranslateSocketError();
      }

      nTempVal *= 4;
      nTempVal >>= 4;
   }

   return nTempVal;
}

//-------------------------------------------------------------------------------------------------
//
// SetSocketDscp()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetSocketDscp( int32_t nDscp )
{
   bool bRetVal = true;
   int32_t nTempVal = nDscp;

   nTempVal <<= 4;
   nTempVal /= 4;

   if ( IsSocketValid() )
   {
      if ( SETSOCKOPT( m_socket, IPPROTO_IP, IP_TOS, &nTempVal, sizeof( nTempVal ) ) == SocketError )
      {
         TranslateSocketError();
         bRetVal = false;
      }
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// GetWindowSize()
//
//-------------------------------------------------------------------------------------------------
uint32_t CSimpleSocket::GetWindowSize( uint32_t nOptionName )
{
   uint32_t nTcpWinSize = 0;

   //-------------------------------------------------------------------------
   // no socket given, return system default allocate our own new socket
   //-------------------------------------------------------------------------
   if ( IsSocketValid() )
   {
      socklen_t nLen = sizeof( nTcpWinSize );

      //---------------------------------------------------------------------
      // query for buffer size
      //---------------------------------------------------------------------
      GETSOCKOPT( m_socket, SOL_SOCKET, nOptionName, &nTcpWinSize, &nLen );
      TranslateSocketError();
   }
   else
   {
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
   }

   return nTcpWinSize;
}

//-------------------------------------------------------------------------------------------------
//
// SetWindowSize()
//
//-------------------------------------------------------------------------------------------------
uint32_t CSimpleSocket::SetWindowSize( uint32_t nOptionName, uint32_t nWindowSize )
{
   //-------------------------------------------------------------------------
   // no socket given, return system default allocate our own new socket
   //-------------------------------------------------------------------------
   if ( IsSocketValid() )
   {
      SETSOCKOPT( m_socket, SOL_SOCKET, nOptionName, &nWindowSize, sizeof( nWindowSize ) );
      TranslateSocketError();
   }
   else
   {
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
   }

   return nWindowSize;
}

//-------------------------------------------------------------------------------------------------
//
// DisableNagleAlgorithm()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::DisableNagleAlgoritm()
{
   bool bRetVal = false;
   int32_t nTcpNoDelay = 1;

   //----------------------------------------------------------------------
   // Set TCP NoDelay flag to true
   //----------------------------------------------------------------------
   if ( SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nTcpNoDelay, sizeof( int32_t ) ) == 0 )
   {
      bRetVal = true;
   }

   TranslateSocketError();

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// EnableNagleAlgorithm()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::EnableNagleAlgoritm()
{
   bool bRetVal = false;
   int32_t nTcpNoDelay = 0;

   //----------------------------------------------------------------------
   // Set TCP NoDelay flag to false
   //----------------------------------------------------------------------
   if ( SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nTcpNoDelay, sizeof( int32_t ) ) == 0 )
   {
      bRetVal = true;
   }

   TranslateSocketError();

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// Send() - Send data on a valid socket
//
//-------------------------------------------------------------------------------------------------
int32_t CSimpleSocket::Send( const uint8_t* pBuf, size_t bytesToSend )
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

   switch ( m_nSocketType )
   {
   case CSimpleSocket::SocketTypeTcp:
      sendMessage = [&] { return SEND( m_socket, pBuf, bytesToSend, 0 ); };
      break;
   case CSimpleSocket::SocketTypeUdp:
      sendMessage = [&] {
         const auto addrToSentTo = reinterpret_cast<const sockaddr*>( GetUdpTxAddrBuffer() );
         return SENDTO( m_socket, pBuf, bytesToSend, 0, addrToSentTo, SOCKET_ADDR_IN_SIZE );
      };
      break;
   default:
      break;
   }

   m_timer.SetStartTime();

   // Check error condition and attempt to resend if call was interrupted by a signal.
   do
   {
      m_nBytesSent = sendMessage();
      TranslateSocketError();
   } while ( GetSocketError() == CSimpleSocket::SocketInterrupted );

   m_timer.SetEndTime();

   return m_nBytesSent;
}

int32_t CSimpleSocket::Send( std::string buffer )
{
   return Send( reinterpret_cast<const uint8_t*>( buffer.c_str() ), buffer.length() );
}

//-------------------------------------------------------------------------------------------------
//
// Close() - Close socket and free up any memory allocated for the socket
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::Close()
{
   bool bRetVal = false;

   // if socket handle is currently valid, close and then invalidate
   if ( IsSocketValid() )
   {
      if ( CLOSE( m_socket ) == CSimpleSocket::SocketSuccess )
      {
         m_socket = INVALID_SOCKET;
         bRetVal = true;
      }
      TranslateSocketError();
   }
   else
   {
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// Shtudown()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::Shutdown( CShutdownMode nShutdown )
{
   if ( shutdown( m_socket, nShutdown ) == SocketError )
   {
      TranslateSocketError();

      // Shutdown failed because there was no connection.
      // This is typically cause by a the remote sending FIN
      // before the local side has finished.
      return ( m_socketErrno == SocketNotconnected );
   }

   return true;
}

//-------------------------------------------------------------------------------------------------
//
// Flush()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::Flush()
{
   int32_t nTcpNoDelay = 1;
   int32_t nCurFlags = 0;
   uint8_t tmpbuf = 0;
   bool bRetVal = false;

   //--------------------------------------------------------------------------
   // Get the current setting of the TCP_NODELAY flag.
   //--------------------------------------------------------------------------
   if ( GETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nCurFlags, sizeof( int32_t ) ) == 0 )
   {
      //----------------------------------------------------------------------
      // Set TCP NoDelay flag
      //----------------------------------------------------------------------
      if ( SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nTcpNoDelay, sizeof( int32_t ) ) == 0 )
      {
         //------------------------------------------------------------------
         // Send empty byte stream to flush the TCP send buffer
         //------------------------------------------------------------------
         if ( Send( &tmpbuf, 0 ) != CSimpleSocket::SocketError )
         {
            bRetVal = true;
         }

         TranslateSocketError();
      }

      //----------------------------------------------------------------------
      // Reset the TCP_NODELAY flag to original state.
      //----------------------------------------------------------------------
      SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nCurFlags, sizeof( int32_t ) );
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// Writev -
//
//-------------------------------------------------------------------------------------------------
int32_t CSimpleSocket::Writev( const struct iovec* pVector, size_t nCount )
{
   int32_t nBytes = 0;
   int32_t nBytesSent = 0;
   int32_t i = 0;

   //--------------------------------------------------------------------------
   // Send each buffer as a separate send, windows does not support this
   // function call.
   //--------------------------------------------------------------------------
   for ( i = 0; i < (int32_t)nCount; i++ )
   {
      if ( ( nBytes = Send( (uint8_t*)pVector[ i ].iov_base, pVector[ i ].iov_len ) ) == CSimpleSocket::SocketError )
      {
         break;
      }

      nBytesSent += nBytes;
   }

   if ( i > 0 )
   {
      Flush();
   }

   return nBytesSent;
}

//-------------------------------------------------------------------------------------------------
//
// Send() - Send data on a valid socket via a vector of buffers.
//
//-------------------------------------------------------------------------------------------------
int32_t CSimpleSocket::Send( const struct iovec* sendVector, int32_t nNumItems )
{
   SetSocketError( SocketSuccess );
   m_nBytesSent = 0;

   if ( ( m_nBytesSent = WRITEV( m_socket, sendVector, nNumItems ) ) == CSimpleSocket::SocketError )
   {
      TranslateSocketError();
   }

   return m_nBytesSent;
}

//-------------------------------------------------------------------------------------------------
//
// SetReceiveTimeout()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetReceiveTimeout( int32_t nRecvTimeoutSec, int32_t nRecvTimeoutUsec )
{
   bool bRetVal = true;

   memset( &m_stRecvTimeout, 0, sizeof( struct timeval ) );

   m_stRecvTimeout.tv_sec = nRecvTimeoutSec;
   m_stRecvTimeout.tv_usec = nRecvTimeoutUsec;

   //--------------------------------------------------------------------------
   // Sanity check to make sure the options are supported!
   //--------------------------------------------------------------------------
   if ( SETSOCKOPT( m_socket, SOL_SOCKET, SO_RCVTIMEO, &m_stRecvTimeout, sizeof( struct timeval ) ) ==
        CSimpleSocket::SocketError )
   {
      bRetVal = false;
      TranslateSocketError();
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// SetSendTimeout()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetSendTimeout( int32_t nSendTimeoutSec, int32_t nSendTimeoutUsec )
{
   bool bRetVal = true;

   memset( &m_stSendTimeout, 0, sizeof( timeval ) );
   m_stSendTimeout.tv_sec = nSendTimeoutSec;
   m_stSendTimeout.tv_usec = nSendTimeoutUsec;

   if ( SETSOCKOPT( m_socket, SOL_SOCKET, SO_SNDTIMEO, &m_stSendTimeout, sizeof( timeval ) ) == SocketError )
   {
      bRetVal = false;
      TranslateSocketError();
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// SetOptionReuseAddr()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetOptionReuseAddr()
{
   int32_t nReuse = IPTOS_LOWDELAY;
   bool bRetVal = ( SETSOCKOPT( m_socket, SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof( int32_t ) ) == SocketSuccess );

   if ( bRetVal && m_nSocketType == SocketTypeTcp )
   {
      bRetVal = ( SETSOCKOPT( m_socket, IPPROTO_TCP, IP_TOS, &nReuse, sizeof( int32_t ) ) == SocketSuccess );
   }
   TranslateSocketError();

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
void CSimpleSocket::SetConnectTimeout( int32_t nConnectTimeoutSec, int32_t nConnectTimeoutUsec )
{
   m_stConnectTimeout.tv_sec = nConnectTimeoutSec;
   m_stConnectTimeout.tv_usec = nConnectTimeoutUsec;
}

//-------------------------------------------------------------------------------------------------
//
// SetOptionLinger()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetOptionLinger( bool bEnable, uint16_t nTime )
{
   bool bRetVal = false;

   m_stLinger.l_onoff = ( bEnable ) ? 1 : 0;
   m_stLinger.l_linger = nTime;

   if ( SETSOCKOPT( m_socket, SOL_SOCKET, SO_LINGER, &m_stLinger, sizeof( m_stLinger ) ) == 0 )
   {
      bRetVal = true;
   }

   TranslateSocketError();

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
int32_t CSimpleSocket::Receive( uint32_t nMaxBytes, uint8_t* pBuffer )
{
   if ( !IsSocketValid() )
   {
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
      m_nBytesReceived = SocketError;
      return m_nBytesReceived;
   }

   if ( nMaxBytes == 0 )
   {
      m_nBytesReceived = nMaxBytes;
      return m_nBytesReceived;
   }

   uint8_t* pWorkBuffer = pBuffer;
   if ( pBuffer == nullptr )
   {
      m_nBufferSize = nMaxBytes;
      m_sBuffer.assign( nMaxBytes, '\0' );
      pWorkBuffer = reinterpret_cast<uint8_t*>( &m_sBuffer[ 0 ] );   // Use string's internal memory as the buffer
   }

   SetSocketError( CSimpleSocket::SocketSuccess );
   m_nBytesReceived = 0;

   std::function<int32_t()> receivePacket = [] { return -1; };

   switch ( m_nSocketType )
   {
   case CSimpleSocket::SocketTypeTcp:
      receivePacket = [&] { return RECV( m_socket, ( pWorkBuffer + m_nBytesReceived ), nMaxBytes, m_nFlags ); };
      break;
   case CSimpleSocket::SocketTypeUdp:
      receivePacket = [&] {
         uint32_t srcSize = SOCKET_ADDR_IN_SIZE;
         return RECVFROM( m_socket, ( pWorkBuffer + m_nBytesReceived ), nMaxBytes, 0, GetUdpRxAddrBuffer(), &srcSize );
      };
      break;
   default:
      // SetSocketError( CSimpleSocket::SocketProtocolError );
      break;
   }

   m_timer.SetStartTime();

   do
   {
      m_nBytesReceived = receivePacket();
      TranslateSocketError();
   } while ( GetSocketError() == CSimpleSocket::SocketInterrupted );

   m_timer.SetEndTime();

   if ( m_nBytesReceived == CSimpleSocket::SocketError )
   {
      // Clear the output buffer
      if ( pBuffer == nullptr )
      {
         m_sBuffer.clear();
      }
      else
      {
         pBuffer[ 0 ] = '\0';
      }
   }
   else if ( pBuffer == nullptr )
   {
      m_sBuffer.erase( m_nBytesReceived );
   }

   return m_nBytesReceived;
}

//-------------------------------------------------------------------------------------------------
//
// SetNonblocking()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetNonblocking()
{
   int32_t nCurFlags;

#if WIN32
   nCurFlags = 1;

   if ( ioctlsocket( m_socket, FIONBIO, (ULONG*)&nCurFlags ) != SocketSuccess )
   {
      TranslateSocketError();
      return false;
   }
#else
   if ( ( nCurFlags = fcntl( m_socket, F_GETFL ) ) < 0 )
   {
      TranslateSocketError();
      return false;
   }

   nCurFlags |= O_NONBLOCK;

   if ( fcntl( m_socket, F_SETFL, nCurFlags ) != 0 )
   {
      TranslateSocketError();
      return false;
   }
#endif

   m_bIsBlocking = false;

   return true;
}

//-------------------------------------------------------------------------------------------------
//
// SetBlocking()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetBlocking( void )
{
   int32_t nCurFlags;

#if WIN32
   nCurFlags = 0;

   if ( ioctlsocket( m_socket, FIONBIO, (ULONG*)&nCurFlags ) != SocketSuccess )
   {
      return false;
   }
#else
   if ( ( nCurFlags = fcntl( m_socket, F_GETFL ) ) < 0 )
   {
      TranslateSocketError();
      return false;
   }

   nCurFlags &= ( ~O_NONBLOCK );

   if ( fcntl( m_socket, F_SETFL, nCurFlags ) != 0 )
   {
      TranslateSocketError();
      return false;
   }
#endif
   m_bIsBlocking = true;

   return true;
}

//-------------------------------------------------------------------------------------------------
//
// SendFile() - stands-in for system provided sendfile
//
//-------------------------------------------------------------------------------------------------
int32_t CSimpleSocket::SendFile( int32_t nOutFd, int32_t nInFd, off_t* pOffset, int32_t nCount )
{
   int32_t nOutCount = CSimpleSocket::SocketError;

   static char szData[ SOCKET_SENDFILE_BLOCKSIZE ];
   int32_t nInCount = 0;

   if ( SEEK( nInFd, *pOffset, SEEK_SET ) == -1 )
   {
      return -1;
   }

   while ( nOutCount < nCount )
   {
      nInCount =
          ( nCount - nOutCount ) < SOCKET_SENDFILE_BLOCKSIZE ? ( nCount - nOutCount ) : SOCKET_SENDFILE_BLOCKSIZE;

      if ( READ( nInFd, szData, nInCount ) != nInCount )
      {
         return -1;
      }

      if ( SEND( nOutFd, szData, nInCount, 0 ) != nInCount )
      {
         return -1;
      }

      nOutCount += nInCount;
   }

   *pOffset += nOutCount;

   TranslateSocketError();

   return nOutCount;
}

//-------------------------------------------------------------------------------------------------
//
// TranslateSocketError() -
//
//-------------------------------------------------------------------------------------------------
void CSimpleSocket::TranslateSocketError( void )
{
#if defined( _LINUX ) || defined( _DARWIN )
   switch ( errno )
   {
   case EXIT_SUCCESS:
      SetSocketError( CSimpleSocket::SocketSuccess );
      break;
   case ENOTCONN:
      SetSocketError( CSimpleSocket::SocketNotconnected );
      break;
   case ENOTSOCK:
   case EBADF:
   case EACCES:
   case EAFNOSUPPORT:
   case EMFILE:
   case ENFILE:
   case ENOBUFS:
   case ENOMEM:
   case EPROTONOSUPPORT:
   case EPIPE:
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
      break;
   case ECONNREFUSED:
      SetSocketError( CSimpleSocket::SocketConnectionRefused );
      break;
   case ETIMEDOUT:
      SetSocketError( CSimpleSocket::SocketTimedout );
      break;
   case EINPROGRESS:
      SetSocketError( CSimpleSocket::SocketEinprogress );
      break;
   case EWOULDBLOCK:
      //        case EAGAIN:
      SetSocketError( CSimpleSocket::SocketEwouldblock );
      break;
   case EINTR:
      SetSocketError( CSimpleSocket::SocketInterrupted );
      break;
   case ECONNABORTED:
      SetSocketError( CSimpleSocket::SocketConnectionAborted );
      break;
   case EINVAL:
      SetSocketError( SocketInvalidOperation );
      break;
   case EPROTO:
      SetSocketError( CSimpleSocket::SocketProtocolError );
      break;
   case EPERM:
      SetSocketError( CSimpleSocket::SocketFirewallError );
      break;
   case EFAULT:
      SetSocketError( CSimpleSocket::SocketInvalidSocketBuffer );
      break;
   case ECONNRESET:
   case ENOPROTOOPT:
      SetSocketError( CSimpleSocket::SocketConnectionReset );
      break;
   case EADDRINUSE:
      SetSocketError( CSimpleSocket::SocketAddressInUse );
      break;
   case EISCONN:
      SetSocketError( CSimpleSocket::SocketAlreadyConnected );
      break;
   default:
      SetSocketError( CSimpleSocket::SocketEunknown );
      break;
   }
#endif
#ifdef WIN32
   const int32_t nError = WSAGetLastError();
   switch ( nError )
   {
   case EXIT_SUCCESS:
      SetSocketError( CSimpleSocket::SocketSuccess );
      break;
   case WSAEBADF:
   case WSAENOTCONN:
      SetSocketError( CSimpleSocket::SocketNotconnected );
      break;
   case WSAEINTR:
      SetSocketError( CSimpleSocket::SocketInterrupted );
      break;
   case WSAEINVAL:
      SetSocketError( SocketInvalidOperation );
      break;
   case WSAEACCES:
   case WSAEAFNOSUPPORT:
   case WSAEMFILE:
   case WSAENOBUFS:
   case WSAEPROTONOSUPPORT:
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
      break;
   case WSAECONNREFUSED:
      SetSocketError( CSimpleSocket::SocketConnectionRefused );
      break;
   case WSAETIMEDOUT:
      SetSocketError( CSimpleSocket::SocketTimedout );
      break;
   case WSAEINPROGRESS:
      SetSocketError( CSimpleSocket::SocketEinprogress );
      break;
   case WSAECONNABORTED:
      SetSocketError( CSimpleSocket::SocketConnectionAborted );
      break;
   case WSAEWOULDBLOCK:
      SetSocketError( CSimpleSocket::SocketEwouldblock );
      break;
   case WSAENOTSOCK:
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
      break;
   case WSAECONNRESET:
      SetSocketError( CSimpleSocket::SocketConnectionReset );
      break;
   case WSANO_DATA:
   case WSAEADDRNOTAVAIL:
   case WSAHOST_NOT_FOUND:
      SetSocketError( CSimpleSocket::SocketInvalidAddress );
      break;
   case WSAEADDRINUSE:
      SetSocketError( CSimpleSocket::SocketAddressInUse );
      break;
   case WSAEFAULT:
      SetSocketError( CSimpleSocket::SocketInvalidPointer );
      break;
   case WSAEISCONN:
      SetSocketError( CSimpleSocket::SocketAlreadyConnected );
      break;
   default:
      SetSocketError( CSimpleSocket::SocketEunknown );
      break;
   }
#endif
}

//-------------------------------------------------------------------------------------------------
//
// DescribeError()
//
//-------------------------------------------------------------------------------------------------
std::string CSimpleSocket::DescribeError( CSocketError err )
{
   switch ( err )
   {
   case CSimpleSocket::SocketError:
      return "Generic socket error translates to error below.";
   case CSimpleSocket::SocketSuccess:
      return "No socket error.";
   case CSimpleSocket::SocketInvalidSocket:
      return "Invalid socket handle.";
   case CSimpleSocket::SocketInvalidAddress:
      return "Invalid destination address specified.";
   case CSimpleSocket::SocketInvalidPort:
      return "Invalid destination port specified.";
   case CSimpleSocket::SocketConnectionRefused:
      return "No server is listening at remote address.";
   case CSimpleSocket::SocketTimedout:
      return "Timed out while attempting operation.";
   case CSimpleSocket::SocketEwouldblock:
      return "Operation would block if socket were blocking.";
   case CSimpleSocket::SocketNotconnected:
      return "Currently not connected.";
   case CSimpleSocket::SocketEinprogress:
      return "Socket is non-blocking and the connection cannot be completed immediately";
   case CSimpleSocket::SocketInterrupted:
      return "Call was interrupted by a signal that was caught before a valid connection arrived.";
   case CSimpleSocket::SocketConnectionAborted:
      return "The connection has been aborted.";
   case CSimpleSocket::SocketProtocolError:
      return "Invalid protocol for operation.";
   case CSimpleSocket::SocketFirewallError:
      return "Firewall rules forbid connection.";
   case CSimpleSocket::SocketInvalidSocketBuffer:
      return "The receive buffer point outside the process's address space.";
   case CSimpleSocket::SocketConnectionReset:
      return "Connection was forcibly closed by the remote host.";
   case CSimpleSocket::SocketAddressInUse:
      return "Address already in use.";
   case CSimpleSocket::SocketInvalidPointer:
      return "Pointer type supplied as argument is invalid.";
   case CSimpleSocket::SocketInvalidOperation:
      return "An invalid argument was provide for the requested action.";
   case CSimpleSocket::SocketEunknown:
      return "Unknown error";
   default:
      return "No such CSimpleSocket error";
   }
}

//-------------------------------------------------------------------------------------------------
//
// Select()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::Select( int32_t nTimeoutSec, int32_t nTimeoutUSec )
{
   bool bRetVal = false;
   timeval* pTimeout = nullptr;
   auto timeout = timeval{ nTimeoutSec, nTimeoutUSec };

   FD_ZERO( &m_errorFds );
   FD_ZERO( &m_readFds );
   FD_ZERO( &m_writeFds );
   FD_SET( m_socket, &m_errorFds );
   FD_SET( m_socket, &m_readFds );
   FD_SET( m_socket, &m_writeFds );

   // If a valid timeout has been specified then set value, otherwise set timeout to NULL which
   // will block until a descriptor is ready for read/write or an error has occurred.
   if ( ( nTimeoutSec >= 0 ) || ( nTimeoutUSec >= 0 ) )
   {
      pTimeout = &timeout;
   }

   switch ( SELECT( m_socket + 1, &m_readFds, &m_writeFds, &m_errorFds, pTimeout ) )
   {
   case SocketError:
      TranslateSocketError();
      break;
   case 0:   // Handle timeout
      SetSocketError( CSimpleSocket::SocketTimedout );
      break;
   default:
      // If a file descriptor (read/write) is set then check the socket error to see if there is a pending error.
      if ( FD_ISSET( m_socket, &m_readFds ) || FD_ISSET( m_socket, &m_writeFds ) )
      {
         int32_t nError = 0;
         int32_t nLen = sizeof( nError );

         if ( GETSOCKOPT( m_socket, SOL_SOCKET, SO_ERROR, &nError, &nLen ) == SocketSuccess )
         {
            errno = nError;
            bRetVal = ( nError == 0 );
         }

         TranslateSocketError();
      }
      break;
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::Select()
{
   return Select( -1, -1 );   // Specify Blocking Select
}
