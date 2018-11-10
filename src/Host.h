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

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Type Definition Macros                                                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <winsock2.h>
#ifndef __WORDSIZE
static_assert( sizeof( void* ) == 8 || sizeof( void* ) == 4 );
constexpr auto __WORDSIZE = ( sizeof( void* ) == 8 ) ? 64 : ( sizeof( void* ) == 4 ) ? 32 : 0;
#endif

using uint8 = unsigned char;
using int8 = char;
using uint16 = unsigned short;
using int16 = short;
using uint32 = unsigned int;
using int32 = int;
using uint64 = unsigned long long;
using int64 = long long;

#if defined(_LINUX) || defined(_DARWIN)
using SOCKET = int;
#elif defined( _WIN32 )
struct iovec
{
   void*  iov_base;
   size_t iov_len;
};

using socklen_t = int;
#endif

/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Socket Macros                                                             */
/*                                                                           */
/*---------------------------------------------------------------------------*/


#ifdef _WIN32

#define SHUT_RD                0
#define SHUT_WR                1
#define SHUT_RDWR              2
#define READ(a,b,c)            _read(a,b,c)
#define SEEK(a,b,c)            _lseek(a,b,c)
#define RECV(a,b,c,d)          recv(a, (char *)b, c, d)
#define RECVFROM(a,b,c,d,e,f)  recvfrom(a, (char *)b, c, d, (sockaddr *)e, (int *)f)
#define RECV_FLAGS             MSG_WAITALL
#define SELECT(a,b,c,d,e)      select((int32)a,b,c,d,e)
#define SEND(a,b,c,d)          send(a, (const char *)b, (int)c, d)
#define SENDTO(a,b,c,d,e,f)    sendto(a, (const char *)b, (int)c, d, e, f)
#define SEND_FLAGS             0
#define SENDFILE(a,b,c,d)      sendfile(a, b, c, d)
#define SET_SOCKET_ERROR(x,y)  errno=y
#define SOCKET_ERROR_INTERUPT  EINTR
#define SOCKET_ERROR_TIMEDOUT  EAGAIN
#define WRITE(a,b,c)           write(a,b,c)
#define WRITEV(a,b,c)          Writev(b, c)
#define GETSOCKOPT(a,b,c,d,e)  getsockopt(a,b,c,(char *)d, (int *)e)
#define SETSOCKOPT(a,b,c,d,e)  setsockopt(a,b,c,(char *)d, (int)e)
#define GETHOSTBYNAME(a)       gethostbyname(a)

#elif defined(_LINUX) || defined(_DARWIN)

#define ACCEPT(a,b,c)          accept(a,b,c)
#define CONNECT(a,b,c)         connect(a,b,c)
#define CLOSE(a)               close(a)
#define READ(a,b,c)            pread(a,b,c,0)
#define SEEK(a,b,c)            lseek(a,b,c)
#define RECV(a,b,c,d)          recv(a, (void *)b, c, d)
#define RECVFROM(a,b,c,d,e,f)  recvfrom(a, (char *)b, c, d, (sockaddr *)e, f)
#define RECV_FLAGS             MSG_WAITALL
#define SELECT(a,b,c,d,e)      select(a,b,c,d,e)
#define SEND(a,b,c,d)          send(a, (const int8 *)b, c, d)
#define SENDTO(a,b,c,d,e,f)    sendto(a, (const int8 *)b, c, d, e, f)
#define SEND_FLAGS             0
#define SENDFILE(a,b,c,d)      sendfile(a, b, c, d)
#define SET_SOCKET_ERROR(x,y)  errno=y
#define SOCKET_ERROR_INTERUPT  EINTR
#define SOCKET_ERROR_TIMEDOUT  EAGAIN
#define WRITE(a,b,c)           write(a,b,c)
#define WRITEV(a,b,c)          writev(a, b, c)
#define GETSOCKOPT(a,b,c,d,e)  getsockopt((int)a,(int)b,(int)c,(void *)d,(socklen_t *)e)
#define SETSOCKOPT(a,b,c,d,e)  setsockopt((int)a,(int)b,(int)c,(const void *)d,(int)e)
#define GETHOSTBYNAME(a)       gethostbyname(a)

#endif

#ifdef _WIN32

static constexpr bool WINSOCK = _WIN32;
static constexpr auto POSIX = false;

#elif defined(_LINUX) || defined(_DARWIN)

static constexpr auto WINSOCK = false;
static constexpr auto POSIX = true;

#endif

namespace  SimpleSocket
{
   static constexpr int SOCKET_ADDR_IN_SIZE = sizeof( sockaddr_in );

   inline auto ACCEPT = []( SOCKET socket, sockaddr_in& address )->SOCKET
   {
      socklen_t addrLen = SOCKET_ADDR_IN_SIZE;
      return accept( socket, reinterpret_cast<sockaddr*>( &address ), &addrLen );
   };

   inline auto CONNECT = []( SOCKET socket, sockaddr_in& address )
   {
      return connect( socket, reinterpret_cast<sockaddr*>( &address ), SOCKET_ADDR_IN_SIZE );
   };

   inline auto CLOSE = []( SOCKET socket)
   {
      if constexpr ( WINSOCK )
      {
         return closesocket( socket );
      }
      else if constexpr ( POSIX )
      {
         return close( socket );
      }

      return -1;
   };
}

