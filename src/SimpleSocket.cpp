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

#include <stdexcept>
#include <cstdlib>
#include <functional>

#if defined(_LINUX) || defined (_DARWIN)
#include <netinet/ip.h>
#include <fcntl.h>
#endif

#ifdef _WIN32
#include <io.h>
#include <Ws2tcpip.h>
static constexpr auto IPTOS_LOWDELAY = 0x10;
#endif

static constexpr auto SOCKET_SENDFILE_BLOCKSIZE = 8192;

CSimpleSocket::CSimpleSocket( CSocketType nType ) :
   m_socket( INVALID_SOCKET ),
   m_socketErrno( CSimpleSocket::SocketInvalidSocket ),
   m_sBuffer( 48, '\0' ),
   m_nBufferSize( 0 ),
   m_nSocketDomain( AF_INET ),
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

   switch( nType )
   {
   case CSimpleSocket::SocketTypeTcp:
   case CSimpleSocket::SocketTypeUdp:
      m_nSocketDomain = AF_INET;
      break;
   case CSimpleSocket::SocketTypeTcp6:
   case CSimpleSocket::SocketTypeUdp6:
      m_nSocketDomain = AF_INET6;
      break;
#if defined(_LINUX) && !defined(_DARWIN)
   // Declare socket type raw Ethernet - Ethernet
   case CSimpleSocket::SocketTypeRaw:
      m_nSocketDomain = AF_PACKET;
      break;
#endif
   default:
      m_nSocketType = CSimpleSocket::SocketTypeInvalid;
      break;
   }

   if( !Initialize() )
   {
      throw std::runtime_error( "Failed to create socket!" + DescribeError() );
   }
}

CSimpleSocket::CSimpleSocket( const CSimpleSocket &socket )
{
   SetSocketHandle( socket.m_socket );
   SetSocketError( socket.GetSocketError() );
   m_sBuffer = socket.GetData();
   m_nBufferSize = socket.m_nBufferSize;
   m_nSocketDomain = socket.m_nSocketDomain;
   m_nSocketType = socket.m_nSocketType;
   m_nBytesReceived = socket.GetBytesReceived();
   m_nBytesSent = socket.GetBytesSent();
   m_nFlags = socket.m_nFlags;
   m_bIsMulticast = socket.m_bIsMulticast;
   m_bIsBlocking = socket.m_bIsBlocking;

   SetConnectTimeout( socket.GetConnectTimeoutSec(), socket.GetConnectTimeoutUSec() );
   memcpy( &m_stRecvTimeout, &socket.m_stRecvTimeout, sizeof( struct timeval ) );
   memcpy( &m_stSendTimeout, &socket.m_stSendTimeout, sizeof( struct timeval ) );
   memcpy( &m_stLinger, &socket.m_stLinger, sizeof( struct linger ) );
   memcpy( &m_stClientSockaddr, &socket.m_stClientSockaddr, SOCKET_ADDR_IN_SIZE );
   memcpy( &m_stServerSockaddr, &socket.m_stServerSockaddr, SOCKET_ADDR_IN_SIZE );
   memcpy( &m_stMulticastGroup, &socket.m_stMulticastGroup, SOCKET_ADDR_IN_SIZE );
}

CSimpleSocket::CSimpleSocket( CSimpleSocket&& socket ) noexcept
{
   swap( *this, socket );
}

CSimpleSocket& CSimpleSocket::operator=( CSimpleSocket other )
{
   swap( *this, other );
   return *this;
}

CSimpleSocket& CSimpleSocket::operator=( CSimpleSocket&& other ) noexcept
{
   swap( *this, other );
   return *this;
}

CSimpleSocket::~CSimpleSocket()
{
   Close(); // Checks internally if socket is valid
   // TO DO DEBATE: Should this terminate is Close failed?
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

   swap( lhs.m_stLinger.l_linger, rhs.m_stLinger.l_linger );
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

#ifdef WIN32
   //-------------------------------------------------------------------------
   // Data structure containing general Windows Sockets Info
   //-------------------------------------------------------------------------
   memset( &m_hWSAData, 0, sizeof( m_hWSAData ) );
   WSAStartup( MAKEWORD( 2, 2 ), &m_hWSAData );
#endif

   m_timer.SetStartTime();
   m_socket = socket( m_nSocketDomain, m_nSocketType, 0 ); // Create the basic Socket Handle
   m_timer.SetEndTime();

   TranslateSocketError();

   return IsSocketValid();
}

//-------------------------------------------------------------------------------------------------
void CSimpleSocket::SetSocketError(CSimpleSocket::CSocketError error)
{
   m_socketErrno = error;
}

//-------------------------------------------------------------------------------------------------
//
// BindInterface()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::BindInterface( const char *pInterface )
{
   bool bRetVal = false;

   if( GetMulticast() )
   {
      if( pInterface )
      {
         struct in_addr stInterfaceAddr;
         memset( &stInterfaceAddr, 0, sizeof( stInterfaceAddr ) );
         inet_pton( m_nSocketDomain, pInterface, &stInterfaceAddr.s_addr );

         bRetVal = ( SETSOCKOPT( m_socket, IPPROTO_IP, IP_MULTICAST_IF,
                     &stInterfaceAddr, sizeof( stInterfaceAddr ) )
                     == SocketSuccess );
         TranslateSocketError();
      }
   }
   else
   {
      if( pInterface )
      {
         struct sockaddr_in stInterfaceAddr;
         // Set up the sockaddr structure
         stInterfaceAddr.sin_family = AF_INET;
         inet_pton( m_nSocketDomain, pInterface, &stInterfaceAddr.sin_addr.s_addr );
         stInterfaceAddr.sin_port = 0;

         // Bind the socket using the such that it only use a specified interface
         if( bind( m_socket, (sockaddr*)&stInterfaceAddr, sizeof( stInterfaceAddr ) ) == SocketError )
         {
            TranslateSocketError();
         }
         else
         {
            bRetVal = true;
         }
      }
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// SetMulticast()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetMulticast( bool bEnable, uint8 multicastTTL )
{
   bool bRetVal = false;

   if( GetSocketType() == CSimpleSocket::SocketTypeUdp )
   {
      m_bIsMulticast = bEnable;
      if( SETSOCKOPT( m_socket, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&multicastTTL, sizeof( multicastTTL ) ) == SocketError )
      {
         TranslateSocketError();
         bRetVal = false;
      }
      else
      {
         bRetVal = true;
      }
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
bool CSimpleSocket::JoinMulticast( const char* pGroup, uint16 nPort )
{
   bool bRetVal = true;

   if( GetSocketType() != CSimpleSocket::SocketTypeUdp || !GetMulticast() )
   {
      bRetVal = false;
      m_socketErrno = CSimpleSocket::SocketProtocolError;
   }

   if( bRetVal )
   {
      bRetVal = SetOptionReuseAddr(); // Don't block the multicast address
   }

   if( bRetVal )
   {
      memset( &m_stMulticastGroup, 0, sizeof( m_stMulticastGroup ) );
      m_stMulticastGroup.sin_family = AF_INET;
      m_stMulticastGroup.sin_addr.s_addr = htonl( INADDR_ANY );
      m_stMulticastGroup.sin_port = htons( nPort );

      //--------------------------------------------------------------------------
      // Bind to the specified port
      //--------------------------------------------------------------------------
      bRetVal = ( bind( m_socket, ( struct sockaddr * )&m_stMulticastGroup,
                  sizeof( m_stMulticastGroup ) ) == CSimpleSocket::SocketSuccess );
   }

   if( bRetVal )
   {
      struct ip_mreq stMulticastRequest;

      inet_pton( m_nSocketDomain, pGroup, &stMulticastRequest.imr_multiaddr.s_addr );
      stMulticastRequest.imr_interface.s_addr = htonl( INADDR_ANY );

      //----------------------------------------------------------------------
      // Join the multicast group
      //----------------------------------------------------------------------
      bRetVal = ( SETSOCKOPT( m_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &stMulticastRequest,
                  sizeof( stMulticastRequest ) ) == SocketSuccess );
   }

   if( bRetVal )
   {
       // save group address ( for sending ... rcv TBA )
      inet_pton( m_nSocketDomain, pGroup, &m_stMulticastGroup.sin_addr.s_addr );
   }

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
//
// SetSocketDscp()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetSocketDscp( int32 nDscp )
{
   bool  bRetVal = true;
   int32 nTempVal = nDscp;

   nTempVal <<= 4;
   nTempVal /= 4;

   if( IsSocketValid() )
   {
      if( SETSOCKOPT( m_socket, IPPROTO_IP, IP_TOS, &nTempVal, sizeof( nTempVal ) ) == SocketError )
      {
         TranslateSocketError();
         bRetVal = false;
      }
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
   //return inet_ntoa(m_stClientSockaddr.sin_addr);
   char buff[ 16 ];

   if( inet_ntop( m_nSocketDomain, &m_stClientSockaddr.sin_addr, buff, 16 ) == nullptr )
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
   //return inet_ntoa(m_stServerSockaddr.sin_addr);
   char buff[ 16 ] = { '\0' };

   if( inet_ntop( m_nSocketDomain, &m_stServerSockaddr.sin_addr, buff, 16 ) == nullptr )
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
   char buff[ 16 ] = { '\0' };;

   if( inet_ntop( m_nSocketDomain, &m_stMulticastGroup.sin_addr, buff, 16 ) == nullptr )
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
int32 CSimpleSocket::GetSocketDscp( void )
{
   int32      nTempVal = 0;
   socklen_t  nLen = 0;

   if( IsSocketValid() )
   {
      if( GETSOCKOPT( m_socket, IPPROTO_IP, IP_TOS, &nTempVal, &nLen ) == SocketError )
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
// GetWindowSize()
//
//-------------------------------------------------------------------------------------------------
uint32 CSimpleSocket::GetWindowSize( uint32 nOptionName )
{
   uint32 nTcpWinSize = 0;

   //-------------------------------------------------------------------------
   // no socket given, return system default allocate our own new socket
   //-------------------------------------------------------------------------
   if( IsSocketValid() )
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
uint32 CSimpleSocket::SetWindowSize( uint32 nOptionName, uint32 nWindowSize )
{
    //-------------------------------------------------------------------------
    // no socket given, return system default allocate our own new socket
    //-------------------------------------------------------------------------
   if( IsSocketValid() )
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
   bool  bRetVal = false;
   int32 nTcpNoDelay = 1;

   //----------------------------------------------------------------------
   // Set TCP NoDelay flag to true
   //----------------------------------------------------------------------
   if( SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nTcpNoDelay, sizeof( int32 ) ) == 0 )
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
   bool  bRetVal = false;
   int32 nTcpNoDelay = 0;

   //----------------------------------------------------------------------
   // Set TCP NoDelay flag to false
   //----------------------------------------------------------------------
   if( SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nTcpNoDelay, sizeof( int32 ) ) == 0 )
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
int32 CSimpleSocket::Send( const uint8 *pBuf, size_t bytesToSend )
{
   if( !IsSocketValid() || bytesToSend == 0 || pBuf == nullptr )
   {
      SetSocketError( IsSocketValid() ? SocketInvalidPointer : SocketInvalidSocket );
      m_nBytesSent = -1;
      return m_nBytesSent;
   }

   SetSocketError( SocketSuccess );
   m_nBytesSent = 0;

   std::function<int32()> sendMessage = [] { return -1; };

   switch( m_nSocketType )
   {
   case CSimpleSocket::SocketTypeTcp:
      sendMessage = [ & ] { return SEND( m_socket, pBuf, bytesToSend, 0 ); };
      break;
   case CSimpleSocket::SocketTypeUdp:
      sendMessage = [ & ]
      {
         const sockaddr* addrToSentTo = m_bIsMulticast ? (const sockaddr *)&m_stMulticastGroup : (const sockaddr *)&m_stServerSockaddr;
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
   } while( GetSocketError() == CSimpleSocket::SocketInterrupted );

   m_timer.SetEndTime();

   return m_nBytesSent;
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
   if( IsSocketValid() )
   {
      if( CLOSE( m_socket ) == CSimpleSocket::SocketSuccess )
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
   if( shutdown( m_socket, nShutdown ) == SocketError )
   {
      TranslateSocketError();
      return false;
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
   int32 nTcpNoDelay = 1;
   int32 nCurFlags = 0;
   uint8 tmpbuf = 0;
   bool  bRetVal = false;

   //--------------------------------------------------------------------------
   // Get the current setting of the TCP_NODELAY flag.
   //--------------------------------------------------------------------------
   if( GETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nCurFlags, sizeof( int32 ) ) == 0 )
   {
       //----------------------------------------------------------------------
       // Set TCP NoDelay flag
       //----------------------------------------------------------------------
      if( SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nTcpNoDelay, sizeof( int32 ) ) == 0 )
      {
          //------------------------------------------------------------------
          // Send empty byte stream to flush the TCP send buffer
          //------------------------------------------------------------------
         if( Send( &tmpbuf, 0 ) != CSimpleSocket::SocketError )
         {
            bRetVal = true;
         }

         TranslateSocketError();
      }

      //----------------------------------------------------------------------
      // Reset the TCP_NODELAY flag to original state.
      //----------------------------------------------------------------------
      SETSOCKOPT( m_socket, IPPROTO_TCP, TCP_NODELAY, &nCurFlags, sizeof( int32 ) );
   }

   return bRetVal;
}


//-------------------------------------------------------------------------------------------------
//
// Writev -
//
//-------------------------------------------------------------------------------------------------
int32 CSimpleSocket::Writev( const struct iovec *pVector, size_t nCount )
{
   int32 nBytes = 0;
   int32 nBytesSent = 0;
   int32 i = 0;

   //--------------------------------------------------------------------------
   // Send each buffer as a separate send, windows does not support this
   // function call.
   //--------------------------------------------------------------------------
   for( i = 0; i < (int32)nCount; i++ )
   {
      if( ( nBytes = Send( (uint8 *)pVector[ i ].iov_base, pVector[ i ].iov_len ) ) == CSimpleSocket::SocketError )
      {
         break;
      }

      nBytesSent += nBytes;
   }

   if( i > 0 )
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
int32 CSimpleSocket::Send( const struct iovec *sendVector, int32 nNumItems )
{
   SetSocketError( SocketSuccess );
   m_nBytesSent = 0;

   if( ( m_nBytesSent = WRITEV( m_socket, sendVector, nNumItems ) ) == CSimpleSocket::SocketError )
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
bool CSimpleSocket::SetReceiveTimeout( int32 nRecvTimeoutSec, int32 nRecvTimeoutUsec )
{
   bool bRetVal = true;

   memset( &m_stRecvTimeout, 0, sizeof( struct timeval ) );

   m_stRecvTimeout.tv_sec = nRecvTimeoutSec;
   m_stRecvTimeout.tv_usec = nRecvTimeoutUsec;

   //--------------------------------------------------------------------------
   // Sanity check to make sure the options are supported!
   //--------------------------------------------------------------------------
   if( SETSOCKOPT( m_socket, SOL_SOCKET, SO_RCVTIMEO, &m_stRecvTimeout,
       sizeof( struct timeval ) ) == CSimpleSocket::SocketError )
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
bool CSimpleSocket::SetSendTimeout( int32 nSendTimeoutSec, int32 nSendTimeoutUsec )
{
   bool bRetVal = true;

   memset( &m_stSendTimeout, 0, sizeof( struct timeval ) );
   m_stSendTimeout.tv_sec = nSendTimeoutSec;
   m_stSendTimeout.tv_usec = nSendTimeoutUsec;

   //--------------------------------------------------------------------------
   // Sanity check to make sure the options are supported!
   //--------------------------------------------------------------------------
   if( SETSOCKOPT( m_socket, SOL_SOCKET, SO_SNDTIMEO, &m_stSendTimeout,
       sizeof( struct timeval ) ) == CSimpleSocket::SocketError )
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
   int32 nReuse = IPTOS_LOWDELAY;
   const bool bRetVal = ( SETSOCKOPT( m_socket, SOL_SOCKET, SO_REUSEADDR, &nReuse, sizeof( int32 ) ) == SocketSuccess );
   TranslateSocketError();

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
int32 CSimpleSocket::GetConnectTimeoutSec() const
{
   return m_stConnectTimeout.tv_sec;
}

//-------------------------------------------------------------------------------------------------
int32 CSimpleSocket::GetConnectTimeoutUSec() const
{
   return m_stConnectTimeout.tv_usec;
}

//-------------------------------------------------------------------------------------------------
void CSimpleSocket::SetConnectTimeout( int32 nConnectTimeoutSec, int32 nConnectTimeoutUsec )
{
   m_stConnectTimeout.tv_sec = nConnectTimeoutSec;
   m_stConnectTimeout.tv_usec = nConnectTimeoutUsec;
}

//-------------------------------------------------------------------------------------------------
//
// SetOptionLinger()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetOptionLinger( bool bEnable, uint16 nTime )
{
   bool bRetVal = false;

   m_stLinger.l_onoff = ( bEnable == true ) ? 1 : 0;
   m_stLinger.l_linger = nTime;

   if( SETSOCKOPT( m_socket, SOL_SOCKET, SO_LINGER, &m_stLinger, sizeof( m_stLinger ) ) == 0 )
   {
      bRetVal = true;
   }

   TranslateSocketError();

   return bRetVal;
}

//-------------------------------------------------------------------------------------------------
int32 CSimpleSocket::Receive( uint32 nMaxBytes, uint8* pBuffer )
{
   m_nBytesReceived = 0;

   if( !IsSocketValid() ) // If the socket is invalid then return false.
   {
      SetSocketError( CSimpleSocket::SocketInvalidSocket );
      return m_nBytesReceived;
   }

   uint8* pWorkBuffer = pBuffer;
   if( pBuffer == nullptr )
   {
      m_nBufferSize = nMaxBytes;
      m_sBuffer.assign( nMaxBytes, '\0' );
      pWorkBuffer = reinterpret_cast<uint8*>( &m_sBuffer[ 0 ] ); // Use string's internal memory as the buffer
   }

   SetSocketError( CSimpleSocket::SocketSuccess );

   m_timer.SetStartTime();

   switch( m_nSocketType )
   {
   case CSimpleSocket::SocketTypeTcp:
      do
      {
         m_nBytesReceived = RECV( m_socket, ( pWorkBuffer + m_nBytesReceived ),
                                  nMaxBytes, m_nFlags );
         TranslateSocketError();
      } while( GetSocketError() == CSimpleSocket::SocketInterrupted );
      break;
   case CSimpleSocket::SocketTypeUdp:
      do
      {
         sockaddr_in* addrToRxFrom = m_bIsMulticast ? &m_stClientSockaddr : &m_stServerSockaddr;
         uint32 srcSize = SOCKET_ADDR_IN_SIZE;
         m_nBytesReceived = RECVFROM( m_socket, ( pWorkBuffer + m_nBytesReceived ),
                                      nMaxBytes, 0, addrToRxFrom, &srcSize );
         TranslateSocketError();
      } while( GetSocketError() == CSimpleSocket::SocketInterrupted );
      break;
   default:
      //SetSocketError( CSimpleSocket::SocketProtocolError );
      break;
   }

   m_timer.SetEndTime();
   TranslateSocketError();

   if( m_nBytesReceived == CSimpleSocket::SocketError )
   {
       // Clear the output buffer
      if( pBuffer == nullptr )
      {
         m_sBuffer.clear();
      }
      else
      {
         pBuffer[ 0 ] = '\0';
      }
   }
   else if( pBuffer == nullptr )
   {
      m_sBuffer.erase( m_nBytesReceived );
   }

   return m_nBytesReceived;
}

//-------------------------------------------------------------------------------------------------
std::string CSimpleSocket::GetData() const
{
   return m_sBuffer;
}

//-------------------------------------------------------------------------------------------------
int32 CSimpleSocket::GetBytesReceived() const
{
   return m_nBytesReceived;
}

//-------------------------------------------------------------------------------------------------
int32 CSimpleSocket::GetBytesSent() const
{
   return m_nBytesSent;
}

//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::IsNonblocking() const
{
   return !m_bIsBlocking;
}

//-------------------------------------------------------------------------------------------------
//
// SetNonblocking()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::SetNonblocking( void )
{
   int32  nCurFlags;

#if WIN32
   nCurFlags = 1;

   if( ioctlsocket( m_socket, FIONBIO, (ULONG *)&nCurFlags ) != SocketSuccess )
   {
      TranslateSocketError();
      return false;
   }
#else
   if( ( nCurFlags = fcntl( m_socket, F_GETFL ) ) < 0 )
   {
      TranslateSocketError();
      return false;
   }

   nCurFlags |= O_NONBLOCK;

   if( fcntl( m_socket, F_SETFL, nCurFlags ) != 0 )
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
   int32 nCurFlags;

#if WIN32
   nCurFlags = 0;

   if( ioctlsocket( m_socket, FIONBIO, (ULONG *)&nCurFlags ) != SocketSuccess )
   {
      return false;
   }
#else
   if( ( nCurFlags = fcntl( m_socket, F_GETFL ) ) < 0 )
   {
      TranslateSocketError();
      return false;
   }

   nCurFlags &= ( ~O_NONBLOCK );

   if( fcntl( m_socket, F_SETFL, nCurFlags ) != 0 )
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
int32 CSimpleSocket::SendFile( int32 nOutFd, int32 nInFd, off_t *pOffset, int32 nCount )
{
   int32  nOutCount = CSimpleSocket::SocketError;

   static char szData[ SOCKET_SENDFILE_BLOCKSIZE ];
   int32       nInCount = 0;

   if( SEEK( nInFd, *pOffset, SEEK_SET ) == -1 )
   {
      return -1;
   }

   while( nOutCount < nCount )
   {
      nInCount = ( nCount - nOutCount ) < SOCKET_SENDFILE_BLOCKSIZE ? ( nCount - nOutCount ) : SOCKET_SENDFILE_BLOCKSIZE;

      if( READ( nInFd, szData, nInCount ) != nInCount )
      {
         return -1;
      }

      if( SEND( nOutFd, szData, nInCount, 0 ) != nInCount )
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
#if defined(_LINUX) || defined(_DARWIN)
   switch( errno )
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
   default:
      SetSocketError( CSimpleSocket::SocketEunknown );
      break;
   }
#endif
#ifdef WIN32
   const int32 nError = WSAGetLastError();
   switch( nError )
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
   case WSAEACCES:
   case WSAEAFNOSUPPORT:
   case WSAEINVAL:
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
      SetSocketError( CSimpleSocket::SocketInvalidAddress );
      break;
   case WSAEADDRINUSE:
      SetSocketError( CSimpleSocket::SocketAddressInUse );
      break;
   case WSAEFAULT:
      SetSocketError( CSimpleSocket::SocketInvalidPointer );
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
   switch( err ) {
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
   case CSimpleSocket::SocketEunknown:
      return "Unknown error";
   default:
      return "No such CSimpleSocket error";
   }
}

//-------------------------------------------------------------------------------------------------
std::string CSimpleSocket::DescribeError() const
{
   return DescribeError(m_socketErrno);
}

//-------------------------------------------------------------------------------------------------
//
// Select()
//
//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::Select( int32 nTimeoutSec, int32 nTimeoutUSec )
{
   bool     bRetVal = false;
   timeval *pTimeout = nullptr;
   auto timeout = timeval{ nTimeoutSec , nTimeoutUSec };

   FD_ZERO( &m_errorFds );
   FD_ZERO( &m_readFds );
   FD_ZERO( &m_writeFds );
   FD_SET( m_socket, &m_errorFds );
   FD_SET( m_socket, &m_readFds );
   FD_SET( m_socket, &m_writeFds );

   // If a valid timeout has been specified then set value, otherwise set timeout to NULL which
   // will block until a descriptor is ready for read/write or an error has occurred.
   if( ( nTimeoutSec >= 0 ) || ( nTimeoutUSec >= 0 ) )
   {
      pTimeout = &timeout;
   }

   switch( SELECT( m_socket + 1, &m_readFds, &m_writeFds, &m_errorFds, pTimeout ) )
   {
   case SocketError:
      TranslateSocketError();
      break;
   case 0: // Handle timeout
      SetSocketError( CSimpleSocket::SocketTimedout );
      break;
   default:
      // If a file descriptor (read/write) is set then check the socket error to see if there is a pending error.
      if( FD_ISSET( m_socket, &m_readFds ) || FD_ISSET( m_socket, &m_writeFds ) )
      {
         int32 nError = 0;
         int32 nLen = sizeof( nError );

         if( GETSOCKOPT( m_socket, SOL_SOCKET, SO_ERROR, &nError, &nLen ) == SocketSuccess )
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
   return Select( -1, -1 ); // Specify Blocking Select
}

//-------------------------------------------------------------------------------------------------
bool CSimpleSocket::IsSocketValid() const
{
   return ( m_socket != INVALID_SOCKET );
}
