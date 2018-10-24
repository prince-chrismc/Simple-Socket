/*---------------------------------------------------------------------------*/
/*                                                                           */
/* PassiveSocket.cpp - Passive Socket Implementation                         */
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

#include <memory>
#include "PassiveSocket.h"

CPassiveSocket::CPassiveSocket( CSocketType nType ) : CSimpleSocket( nType )
{
}

CPassiveSocket::~CPassiveSocket()
{
   CSimpleSocket::Close();
}

//------------------------------------------------------------------------------
//
// Listen() -
//
//------------------------------------------------------------------------------
bool CPassiveSocket::Listen( const char *pAddr, uint16 nPort, int32 nConnectionBacklog )
{
   bool           bRetVal = false;

#ifdef _LINUX
   int32          nReuse;
   nReuse = IPTOS_LOWDELAY;

   //--------------------------------------------------------------------------
   // Set the following socket option SO_REUSEADDR.  This will allow the file
   // descriptor to be reused immediately after the socket is closed instead
   // of setting in a TIMED_WAIT state.
   //--------------------------------------------------------------------------
   SETSOCKOPT( m_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&nReuse, sizeof( int32 ) );
   SETSOCKOPT( m_socket, IPPROTO_TCP, IP_TOS, &nReuse, sizeof( int32 ) );
#endif

   memset( &m_stServerSockaddr, 0, SOCKET_ADDR_IN_SIZE );
   m_stServerSockaddr.sin_family = AF_INET;
   m_stServerSockaddr.sin_port = htons( nPort );

   //--------------------------------------------------------------------------
   // If no IP Address (interface ethn) is supplied, then bind to all interface
   // else bind to specified interface.
   //--------------------------------------------------------------------------
   if( ( pAddr == NULL ) || ( !strlen( pAddr ) ) )
   {
      m_stServerSockaddr.sin_addr.s_addr = htonl( INADDR_ANY );
   }
   else
   {
      switch( inet_pton( m_nSocketDomain, pAddr, &m_stServerSockaddr.sin_addr ) )
      {
      case -1: TranslateSocketError();                 return false;
      case 0:  SetSocketError( SocketInvalidAddress ); return false;
      case 1:  break; // Success
      default:
         SetSocketError( SocketEunknown );
         return false;
      }
   }

   m_timer.Initialize();
   m_timer.SetStartTime();

   //--------------------------------------------------------------------------
   // Bind to the specified port
   //--------------------------------------------------------------------------
   if( bind( m_socket, ( struct sockaddr * )&m_stServerSockaddr, SOCKET_ADDR_IN_SIZE ) != CSimpleSocket::SocketError )
   {
      if( m_nSocketType == CSimpleSocket::SocketTypeTcp )
      {
         bRetVal = ( listen( m_socket, nConnectionBacklog ) != CSimpleSocket::SocketError );
      }
      else
      {
         bRetVal = true;
      }
   }

   m_timer.SetEndTime();

   //--------------------------------------------------------------------------
   // If there was a socket error then close the socket to clean out the
   // connection in the backlog.
   //--------------------------------------------------------------------------
   TranslateSocketError();

   if( !bRetVal )
   {
      const CSocketError err = GetSocketError();
      Close();
      SetSocketError( err );
   }
   else
   {
      socklen_t nSockAddrLen( SOCKET_ADDR_IN_SIZE );
      memset( &m_stServerSockaddr, 0, SOCKET_ADDR_IN_SIZE );
      getsockname( m_socket, ( struct sockaddr * )&m_stServerSockaddr, &nSockAddrLen );
   }

   return bRetVal;
}

//------------------------------------------------------------------------------
//
// Accept() -
//
//------------------------------------------------------------------------------
template <template<typename> class SmartPtr, class SocketBase>
auto CPassiveSocket::Accept() -> SmartPtr<SocketBase>
{
   static_assert( std::is_base_of<CSimpleSocket, SocketBase>::value, "SocketBase is not derived from CSimpleSocket" );
   static_assert( std::is_default_constructible<SmartPtr<SocketBase>>::value, "template must be default constructable!" );

   static_assert( std::is_member_object_pointer<SocketBase*( SmartPtr<SocketBase>::* )>::value, "template operator* must return a SocketBase*" );

   static_assert( std::is_constructible<SmartPtr<SocketBase>, std::nullptr_t>::value, "template must be constructable by nullptr" );
   static_assert( std::is_constructible<SmartPtr<SocketBase>, CActiveSocket*>::value, "template must be constructable by CActiveSocket*" );

   static_assert( std::is_assignable<SmartPtr<SocketBase>&, std::nullptr_t>::value, "template must be assignable by nullptr" );

   if( m_nSocketType != CSimpleSocket::SocketTypeTcp )
   {
      SetSocketError( CSimpleSocket::SocketProtocolError );
      return nullptr;
   }

   auto pClientSocket = new CActiveSocket();

   //--------------------------------------------------------------------------
   // Wait for incoming connection.
   //--------------------------------------------------------------------------
   if( pClientSocket != nullptr )
   {
      CSocketError socketErrno = SocketSuccess;

      m_timer.Initialize();
      m_timer.SetStartTime();

      do
      {
         errno = 0;
         socklen_t nSockAddrLen( SOCKET_ADDR_IN_SIZE );
         const SOCKET socket = accept( m_socket, ( struct sockaddr * )&m_stClientSockaddr, &nSockAddrLen );

         if( socket != INVALID_SOCKET )
         {
            pClientSocket->SetSocketHandle( socket );
            pClientSocket->TranslateSocketError();
            socketErrno = pClientSocket->GetSocketError();

            //-------------------------------------------------------------
            // Store client and server IP and port information for this
            // connection.
            //-------------------------------------------------------------
            getpeername( m_socket, ( struct sockaddr * )&pClientSocket->m_stClientSockaddr, &nSockAddrLen );
            memcpy( &pClientSocket->m_stClientSockaddr, &m_stClientSockaddr, SOCKET_ADDR_IN_SIZE );

            memset( &pClientSocket->m_stServerSockaddr, 0, SOCKET_ADDR_IN_SIZE );
            getsockname( m_socket, ( struct sockaddr * )&pClientSocket->m_stServerSockaddr, &nSockAddrLen );
         }
         else
         {
            TranslateSocketError();
            socketErrno = GetSocketError();
         }

      } while( socketErrno == CSimpleSocket::SocketInterrupted );

      m_timer.SetEndTime();

      if( socketErrno != CSimpleSocket::SocketSuccess )
      {
         delete pClientSocket;
         pClientSocket = nullptr;
      }
   }

   return SmartPtr<SocketBase>( pClientSocket );
}

// it's just to avoid link error.
template std::unique_ptr<CSimpleSocket> CPassiveSocket::Accept<std::unique_ptr, CSimpleSocket>();
template std::shared_ptr<CSimpleSocket> CPassiveSocket::Accept<std::shared_ptr, CSimpleSocket>();
template std::unique_ptr<CActiveSocket> CPassiveSocket::Accept<std::unique_ptr, CActiveSocket>();
template std::shared_ptr<CActiveSocket> CPassiveSocket::Accept<std::shared_ptr, CActiveSocket>();
