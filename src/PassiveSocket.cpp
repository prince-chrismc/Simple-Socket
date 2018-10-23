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

   memset( &m_stServerSockaddr, 0, sizeof( m_stServerSockaddr ) );
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
   if( bind( m_socket, ( struct sockaddr * )&m_stServerSockaddr, sizeof( m_stServerSockaddr ) ) != CSimpleSocket::SocketError )
   {
      if( m_nSocketType == CSimpleSocket::SocketTypeTcp )
      {
         if( listen( m_socket, nConnectionBacklog ) != CSimpleSocket::SocketError )
         {
            bRetVal = true;
         }
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
      CSocketError err = GetSocketError();
      Close();
      SetSocketError( err );
   }
   else
   {
      socklen_t nSockLen = sizeof( struct sockaddr );

      memset( &m_stServerSockaddr, 0, nSockLen );
      getsockname( m_socket, ( struct sockaddr * )&m_stServerSockaddr, &nSockLen );
   }

   return bRetVal;
}

//------------------------------------------------------------------------------
//
// Accept() -
//
//------------------------------------------------------------------------------
template <template <typename T> class SmartPtr>
auto CPassiveSocket::Accept() -> SmartPtr<CActiveSocket>
{
   static_assert( std::is_member_object_pointer<CActiveSocket*( SmartPtr<CActiveSocket>::* )>::value, "template operator* must return a CActiveSocket*" );
   static_assert( std::is_default_constructible<SmartPtr<CActiveSocket>>::value, "template must be default constructable!" );
   //static_assert( std::is_constructible<SmartPtr<CActiveSocket>, std::nullptr_t, CActiveSocket*>::value, "template must be constructable by nullptr and CActiveSocket*" );
   static_assert( std::is_assignable<SmartPtr<CActiveSocket>&, std::nullptr_t>::value, "template must be assignable by nullptr" );
   //static_assert( std::is_assignable<SmartPtr<CActiveSocket>&, CActiveSocket*>::value, "template must be assignable by CActiveSocket*" );
   //static_assert( std::is_member_function_pointer<decltype(&SmartPtr<CActiveSocket>::operator->)>::value, "A::member is not a member function." );
   //static_assert( std::is_invocable<decltype( &SmartPtr<CActiveSocket>::operator-> )>::value, "A::member is not a member function." );
   //static_assert( std::is_member_function_pointer<&SmartPtr<CActiveSocket>::reset>::value, "A::member is not a member function." );

   uint32         nSockLen;
   SmartPtr<CActiveSocket> pClientSocket( nullptr );
   SOCKET         socket = CSimpleSocket::SocketError;

   if( m_nSocketType != CSimpleSocket::SocketTypeTcp )
   {
      SetSocketError( CSimpleSocket::SocketProtocolError );
      return pClientSocket;
   }

   pClientSocket.reset( new CActiveSocket() );

   //--------------------------------------------------------------------------
   // Wait for incoming connection.
   //--------------------------------------------------------------------------
   if( pClientSocket != nullptr )
   {
      CSocketError socketErrno = SocketSuccess;

      m_timer.Initialize();
      m_timer.SetStartTime();

      nSockLen = sizeof( m_stClientSockaddr );

      do
      {
         errno = 0;
         socket = accept( m_socket, ( struct sockaddr * )&m_stClientSockaddr, (socklen_t *)&nSockLen );

         if( socket != INVALID_SOCKET )
         {
            pClientSocket->SetSocketHandle( socket );
            pClientSocket->TranslateSocketError();
            socketErrno = pClientSocket->GetSocketError();
            socklen_t nSockLen = sizeof( struct sockaddr );

            //-------------------------------------------------------------
            // Store client and server IP and port information for this
            // connection.
            //-------------------------------------------------------------
            getpeername( m_socket, ( struct sockaddr * )&pClientSocket->m_stClientSockaddr, &nSockLen );
            memcpy( (void *)&pClientSocket->m_stClientSockaddr, (void *)&m_stClientSockaddr, nSockLen );

            memset( &pClientSocket->m_stServerSockaddr, 0, nSockLen );
            getsockname( m_socket, ( struct sockaddr * )&pClientSocket->m_stServerSockaddr, &nSockLen );
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
         pClientSocket = nullptr;
      }
   }

   return pClientSocket;
}

// No need to call this TemporaryFunction() function,
// it's just to avoid link error.
void TemporaryFunction()
{
   CPassiveSocket oSocket;
   auto shared = oSocket.Accept<std::shared_ptr>();
   auto unique = oSocket.Accept<std::unique_ptr>();
}
