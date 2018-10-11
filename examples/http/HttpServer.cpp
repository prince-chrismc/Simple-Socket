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

#include "HttpServer.h"

using namespace std::chrono_literals;

HttpServer::HttpServer( HttpVersion version /*= HttpVersion11*/ ) : m_eVersion( version )
{

}

HttpServer::~HttpServer()
{

}

bool HttpServer::Launch( std::string_view addr, int32 nPort )
{
   bool bRetVal = m_oSocket.Initialize();

   if( bRetVal )
   {
      bRetVal = m_oSocket.Listen( addr.data(), nPort );
   }

   if( bRetVal )
   {
      std::thread( [ this ] {
         auto oExitEvent = m_oExitEvent.get_future();
         while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
         {
            std::unique_ptr<CActiveSocket> pClient;
            if( ( pClient = m_oSocket.Accept() ) != nullptr ) // Wait for an incomming connection
            {
               m_vecClients.push_back( std::move( pClient ) );

               m_vecClients.back().get(); // TODO: Create read thread...
            }
         }
                   } ).detach();
   }

   return bRetVal;
}

bool HttpServer::Close()
{
   bool bRetVal = m_oSocket.Shutdown(CSimpleSocket::Both);

   if( bRetVal )
   {
      bRetVal = m_oSocket.Close();
   }

   m_oExitEvent.set_value();

   return bRetVal;
}
