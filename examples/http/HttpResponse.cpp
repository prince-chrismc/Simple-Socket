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

#include "HttpRequest.h"
#include "HttpResponse.h"
#include <algorithm>

/*

HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache/2.2.14 (Win32)
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Content-Length: 88
Content-Type: text/html
Connection: Closed

------------------------------------------------------

HTTP/1.1 200 OK
Cache-Control: no-cache
Server: libnhttpd
Date: Wed Jul 4 15:38:53 2012
Connection: ep-Alive:
Content-Type: application/json;charset=UTF-8
Content-Length: 286

*/

#define CRLF "\r\n"

HttpResponse::HttpResponse( const HttpVersion & in_kreVersion, const HttpStatus & in_kreStatusCode,
                                  const std::string & in_krsReasonPhrase ) :
   m_eVersion( in_kreVersion ),
   m_eStatusCode( in_kreStatusCode ),
   m_sReasonPhrase( in_krsReasonPhrase ),
   m_eContentType( HttpContentInvalid ),
   m_sBody( "" )
{
   std::remove( m_sReasonPhrase.begin(), m_sReasonPhrase.end(), '\r' );
   std::remove( m_sReasonPhrase.begin(), m_sReasonPhrase.end(), '\n' );
}

HttpResponse::~HttpResponse()
{
}

void HttpResponse::SetContentType( const HttpContentType & in_kreContentType )
{
   m_eContentType = in_kreContentType;
}

void HttpResponse::AppendMessageBody( const std::string & in_krsToAdd )
{
   m_sBody.append( in_krsToAdd.c_str() );
}

std::string HttpResponse::GetResponseLine() const
{
   return HttpRequest::STATIC_VersionAsString( m_eVersion ) + " " + std::to_string( ( unsigned long long )m_eStatusCode ) + " " +
      m_sReasonPhrase + CRLF;
}

std::string HttpResponse::GetRawResponse() const
{
   return GetResponseLine() + HttpRequest::STATIC_ContentTypeAsString( m_eContentType ) + CRLF + CRLF + m_sBody;
}

std::string HttpResponse::GetWireFormat() const
{
   return GetResponseLine() + "User-Agent: clsoct_example/1.0" + CRLF + "cache-control: no-cache" + CRLF +
      "Connection: ep-alive" + CRLF + "Content-Length: " + std::to_string( ( unsigned long long )m_sBody.size() ) +
      CRLF + HttpRequest::STATIC_ContentTypeAsString( m_eContentType ) + CRLF + CRLF + m_sBody;
}

HttpResponse HttpResponseParser::GetHttpResponse()
{
   HttpResponse oResponse( HttpRequestParser::STATIC_ParseForVersion( m_sResponseToParse ),
                             STATIC_ParseForStatus( m_sResponseToParse ),
                             STATIC_ParseForReasonPhrase( m_sResponseToParse ) );

   oResponse.SetContentType( HttpRequestParser::STATIC_ParseForContentType( m_sResponseToParse ) );

   oResponse.AppendMessageBody( HttpRequestParser::STATIC_ParseForBody( m_sResponseToParse ) );

   return oResponse;
}

HttpStatus HttpResponseParser::STATIC_ParseForStatus( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return HttpStatusInvalid;

   size_t ulStart = in_krsRequest.find( " " ) + sizeof( " " ) - 1;
   size_t ulEnd = in_krsRequest.find( " ", ulStart );

   long long llCode = std::stoull( in_krsRequest.substr( ulStart, ulEnd - ulStart ) );

   return HttpStatus( llCode );
}

std::string HttpResponseParser::STATIC_ParseForReasonPhrase( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return "";

   size_t ulOffset = in_krsRequest.find( " " ) + sizeof( " " ) - 1;
   ulOffset = in_krsRequest.find( " ", ulOffset ) + sizeof( " " ) - 1;
   size_t ulEnd = in_krsRequest.find( CRLF, ulOffset );
   return in_krsRequest.substr( ulOffset, ulEnd - ulOffset );
}
