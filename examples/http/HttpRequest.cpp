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
#include  <algorithm>

/*  EXAMPLE REQUEST

GET /x-nmos/node/v1.0/self/ HTTP/1.1
cache-control: no-cache
Postman-Ton: 05197dbd-b271-4d43-8c47-0c3e0e4a9e01
User-Agent: PostmanRuntime/6.3.2
Accept: *\/*
Host: 25.25.34.25:12345
accept-encoding: gzip,deflate
Connection: ep-alive

----------------------------------------------------------------------------

GET /x-nmos/node/v1.0/ HTTP/1.1
Host: 25.25.34.25:12345
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:58.0) Gecko/20100101 Firefox/58.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*\/*;q=0.8
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip,deflate
Connection: ep-alive
Upgrade-Insecure-Requests: 1

*/

#define CRLF "\r\n"

#define OPTIONS_STRING "OPTIONS"
#define GET_STRING "GET"
#define HEAD_STRING "HEAD"
#define POST_STRING "POST"
#define PUT_STRING "PUT"
#define DELETE_STRING "DELETE"
#define TRACE_STRING "TRACE"
#define CONNECT_STRING "CONNECT"
#define PATCH_STRING "PATCH"

#define HTTP_VERSION_10_STRING "HTTP/1.0"
#define HTTP_VERSION_11_STRING "HTTP/1.1"

#define HTTP_HOST "Host: "
#define HTTP_CONTENT_TYPE "Content-Type: "
#define HTTP_CONTENT_LENGTH "Content-Length: "

#define HTTP_BODY_SEPERATOR "\r\n\r\n"
#define SIZE_OF_CRLF ( sizeof( CRLF ) - 1 )
#define SIZE_OF_HTTP_BODY_SEPERATOR ( sizeof( HTTP_BODY_SEPERATOR ) - 1 )

// Thanks to https://stackoverflow.com/a/1798170/8480874
std::string trim( const std::string& str,
                  const std::string& whitespace /*= " \t"*/ )
{
   const auto strBegin = str.find_first_not_of( whitespace );
   if( strBegin == std::string::npos )
      return ""; // no content

   const auto strEnd = str.find_last_not_of( whitespace );
   const auto strRange = strEnd - strBegin + 1;

   return str.substr( strBegin, strRange );
}

std::string reduce( const std::string& str,
                    const std::string& fill /*= " "*/,
                    const std::string& whitespace /*= " \t"*/ )
{
    // trim first
   auto result = trim( str, whitespace );

   // replace sub ranges
   auto beginSpace = result.find_first_of( whitespace );
   while( beginSpace != std::string::npos )
   {
      const auto endSpace = result.find_first_not_of( whitespace, beginSpace );
      const auto range = endSpace - beginSpace;

      result.replace( beginSpace, range, fill );

      const auto newStart = beginSpace + fill.length();
      beginSpace = result.find_first_of( whitespace, newStart );
   }

   return result;
}

HttpRequest::HttpRequest( const HttpRequestMethod & in_kreMethod, const std::string & in_krsRequestUri,
                          const HttpVersion & in_kreVersion, const std::string & in_krsHostAndPort ) :
   HttpRequest( in_kreMethod, in_krsRequestUri, in_kreVersion, in_krsHostAndPort, HttpContentInvalid, { ( in_kreVersion == HttpVersion11 ) ? "Connection: keep-alive" : ( in_kreVersion == HttpVersion10 ) ? "Connection: closed" : "",
                "Cache-Control: no-cache", "Accept: */*", "Accept-Encoding: deflate" } )
{
}

HttpRequest::HttpRequest( const HttpRequestMethod & in_kreMethod, const std::string & in_krsRequestUri,
                          const HttpVersion & in_kreVersion, const std::string & in_krsHostAndPort,
                          const HttpContentType & in_kreContentType, const std::initializer_list<std::string>& in_kroMessageHeaders ) :
   m_eMethod( in_kreMethod ),
   m_sRequestUri( in_krsRequestUri ),
   m_eVersion( in_kreVersion ),
   m_sHostAndPort( in_krsHostAndPort ),
   m_eContentType( in_kreContentType ),
   m_vecsMessageHeaders( in_kroMessageHeaders ),
   m_sBody( "" )
{
}

bool HttpRequest::IsValidRequest() const
{
   return ( m_eMethod != HttpRequestInvalid ) || ( m_eMethod != HttpRequestLast ) ||
      ( !m_sRequestUri.empty() ) || ( m_eVersion != HttpVersionInvalid ) || ( m_eVersion != HttpVersionLast );
}

void HttpRequest::SetContentType( const HttpContentType & in_kreContentType )
{
   m_eContentType = in_kreContentType;
}

void HttpRequest::AddMessageHeader( const std::string & in_krsFeildName, const std::string & in_krsFeildValue )
{
   m_vecsMessageHeaders.emplace_back( reduce( in_krsFeildName, "-" ) + ": " + reduce( in_krsFeildValue ) );
}

void HttpRequest::AppendMessageBody( const std::string & in_krsToAdd )
{
   m_sBody.append( in_krsToAdd );
}

std::string HttpRequest::GetRequestLine() const
{
   return STATIC_MethodAsString( m_eMethod ) + " " + m_sRequestUri + " " + STATIC_VersionAsString( m_eVersion ) + CRLF + "Host: " + m_sHostAndPort + CRLF;
}

std::string HttpRequest::GetHeaders() const
{
   std::string sCostumHeaders( "User-Agent: clsocket_example/1.0" );
   sCostumHeaders += CRLF;
   sCostumHeaders += ( "Content-Length: " + std::to_string( m_sBody.size() ) + CRLF );

   if( m_eContentType != HttpContentInvalid )
      sCostumHeaders += CRLF + HttpRequest::STATIC_ContentTypeAsString( m_eContentType );

   for( auto& sMessageHeader : m_vecsMessageHeaders )
      sCostumHeaders += ( sMessageHeader + CRLF );

   return sCostumHeaders;
}

std::string HttpRequest::GetWireFormat() const
{
   return GetRequestLine() + GetHeaders() + CRLF + m_sBody;
}

std::string HttpRequest::STATIC_MethodAsString( const HttpRequestMethod & in_kreMethod )
{
   switch( in_kreMethod )
   {
   case HttpRequestOptions: return OPTIONS_STRING;
   case HttpRequestGet: return GET_STRING;
   case HttpRequestHead: return HEAD_STRING;
   case HttpRequestPost: return POST_STRING;
   case HttpRequestPut: return PUT_STRING;
   case HttpRequestDelete: return DELETE_STRING;
   case HttpRequestTrace: return TRACE_STRING;
   case HttpRequestConnect: return CONNECT_STRING;
   case HttpRequestPatch: return PATCH_STRING;
   default:
      return "";
   }
}

std::string HttpRequest::STATIC_VersionAsString( const HttpVersion & in_kreVersion )
{
   switch( in_kreVersion )
   {
   case HttpVersion10: return HTTP_VERSION_10_STRING;
   case HttpVersion11: return HTTP_VERSION_11_STRING;
   default:
      return "";
   }
}

std::string HttpRequest::STATIC_ContentTypeAsString( const HttpContentType & in_kreContentType )
{
   switch( in_kreContentType )
   {
   case HttpContentText: return std::string( HTTP_CONTENT_TYPE ) + "text;";
   case HttpContentHtml: return std::string( HTTP_CONTENT_TYPE ) + "application/html; text/html;";
   case HttpContentJson: return std::string( HTTP_CONTENT_TYPE ) + "application/json; text/json;";
   default: return "";
   }
}

//---------------------------------------------------------------------------------------------------------------------
std::string HttpRequest::STATIC_ContentLengthToString( size_t in_ullContentLength )
{
   return HTTP_CONTENT_LENGTH + std::to_string( in_ullContentLength );
}

//---------------------------------------------------------------------------------------------------------------------
//
// HttpRequestParser
//
//---------------------------------------------------------------------------------------------------------------------
HttpRequest HttpRequestParser::GetHttpRequest()
{
   HttpRequest oRequest( STATIC_ParseForMethod( m_sRequestToParse ),
                         STATIC_ParseForRequestUri( m_sRequestToParse ),
                         STATIC_ParseForVersion( m_sRequestToParse ),
                         STATIC_ParseForHostAndPort( m_sRequestToParse ),
                         STATIC_ParseForContentType( m_sRequestToParse ),
                         {} );

   STATIC_AppenedParsedHeaders( oRequest, m_sRequestToParse );

   oRequest.AppendMessageBody( STATIC_ParseForBody( m_sRequestToParse ) );

   return oRequest;
}

HttpRequestMethod HttpRequestParser::STATIC_ParseForMethod( const std::string & in_krsRequest )
{
   if( in_krsRequest.find( OPTIONS_STRING ) == 0 ) return HttpRequestOptions;
   if( in_krsRequest.find( GET_STRING ) == 0 ) return HttpRequestGet;
   if( in_krsRequest.find( HEAD_STRING ) == 0 ) return HttpRequestHead;
   if( in_krsRequest.find( POST_STRING ) == 0 ) return HttpRequestPost;
   if( in_krsRequest.find( PUT_STRING ) == 0 ) return HttpRequestPut;
   if( in_krsRequest.find( DELETE_STRING ) == 0 ) return HttpRequestDelete;
   if( in_krsRequest.find( TRACE_STRING ) == 0 ) return HttpRequestTrace;
   if( in_krsRequest.find( CONNECT_STRING ) == 0 ) return HttpRequestConnect;
   if( in_krsRequest.find( PATCH_STRING ) == 0 ) return HttpRequestPatch;

   return HttpRequestInvalid;
}

std::string HttpRequestParser::STATIC_ParseForRequestUri( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return "";

   std::string sRequestLine = in_krsRequest.substr( 0, in_krsRequest.find( " HTTP" ) );

   size_t ulOffset = HttpRequest::STATIC_MethodAsString( STATIC_ParseForMethod( sRequestLine ) ).size() + 1;

   return sRequestLine.substr( ulOffset );
}

HttpVersion HttpRequestParser::STATIC_ParseForVersion( const std::string & in_krsRequest )
{
   if( in_krsRequest.find( HTTP_VERSION_10_STRING ) != std::string::npos ) return HttpVersion10;
   if( in_krsRequest.find( HTTP_VERSION_11_STRING ) != std::string::npos ) return HttpVersion11;

   return HttpVersionInvalid;
}

std::string HttpRequestParser::STATIC_ParseForHostAndPort( const std::string & in_krsRequest )
{
   if( !in_krsRequest.empty() )
   {
      size_t ulOffset = in_krsRequest.find( HTTP_HOST ) + sizeof( HTTP_HOST ) - 1;
      size_t ulEnd = in_krsRequest.find( CRLF, ulOffset );
      return in_krsRequest.substr( ulOffset, ulEnd - ulOffset );
   }

   return "";
}

HttpContentType HttpRequestParser::STATIC_ParseForContentType( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return HttpContentInvalid;

   if( in_krsRequest.find( HTTP_CONTENT_TYPE ) == std::string::npos ) return HttpContentInvalid;

   size_t ulOffset = in_krsRequest.find( HTTP_CONTENT_TYPE ) + sizeof( HTTP_CONTENT_TYPE ) - 1;
   size_t ulEnd = in_krsRequest.find( CRLF, ulOffset );
   std::string sContentTypeLine = in_krsRequest.substr( ulOffset, ulEnd - ulOffset );

   size_t ulTextPos = sContentTypeLine.find( "text" );
   size_t ulHtmlPos = sContentTypeLine.find( "text/html" );
   size_t ulJsonPos = sContentTypeLine.find( "text/json" );
   size_t ulHtmlAppPos = sContentTypeLine.find( "application/html" );
   size_t ulJsonAppPos = sContentTypeLine.find( "application/json" );

   size_t ulMinPos = std::min( { ulTextPos, ulHtmlPos, ulJsonPos, ulHtmlAppPos, ulJsonAppPos } );

   if( ulMinPos == ulHtmlPos ) return HttpContentHtml;
   if( ulMinPos == ulJsonPos ) return HttpContentJson;
   if( ulMinPos == ulHtmlAppPos ) return HttpContentHtml;
   if( ulMinPos == ulJsonAppPos ) return HttpContentJson;
   if( ulMinPos == ulTextPos ) return HttpContentText;

   return HttpContentInvalid;
}

std::string HttpRequestParser::STATIC_ParseForBody( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return "";

   size_t ulOffset = in_krsRequest.find( std::string( CRLF ) + CRLF ) + ( sizeof( CRLF ) - 1 ) * 2;

   if( ulOffset == in_krsRequest.size() ) return "";

   return in_krsRequest.substr( ulOffset );
}

void HttpRequestParser::STATIC_AppenedParsedHeaders( HttpRequest & io_roRequest, const std::string & in_krsRequest )
{
   for( std::string sRawHeaders = in_krsRequest.substr( in_krsRequest.find( CRLF ) + sizeof( CRLF ) - 1 );
        sRawHeaders.find( CRLF );
        sRawHeaders = sRawHeaders.substr( sRawHeaders.find( CRLF ) + sizeof( CRLF ) - 1 ) )
   {
      std::string sNextHeader = sRawHeaders.substr( 0, sRawHeaders.find( CRLF ) );
      const size_t iSeperatorIndex = sNextHeader.find( ':' );

      if( iSeperatorIndex != std::string::npos )
         io_roRequest.AddMessageHeader( sNextHeader.substr( 0, iSeperatorIndex ), sNextHeader.substr( iSeperatorIndex + 1 ) );
   }
}

//---------------------------------------------------------------------------------------------------------------------
//
// HttpRequestParserAdvanced
//
//---------------------------------------------------------------------------------------------------------------------
bool HttpRequestParserAdvance::AppendRequestData( const std::string & in_krsData )
{
   return STATIC_AppendData( in_krsData, m_sHttpHeader, m_sRequestBody );
}

//---------------------------------------------------------------------------------------------------------------------
HttpRequest HttpRequestParserAdvance::GetHttpRequest()
{
   HttpRequest oRequest( HttpRequestParser::STATIC_ParseForMethod( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForRequestUri( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForVersion( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForHostAndPort( m_sHttpHeader ),
                         HttpRequestParser::STATIC_ParseForContentType( m_sHttpHeader ),
                         {} );

   HttpRequestParser::STATIC_AppenedParsedHeaders( oRequest, m_sHttpHeader );

   oRequest.AppendMessageBody( m_sRequestBody );

   return oRequest;
}

//---------------------------------------------------------------------------------------------------------------------
size_t HttpRequestParserAdvance::STATIC_ParseForContentLength( const std::string & in_krsHttpHeader )
{
   if( in_krsHttpHeader.empty() ) return 0;

   size_t sizeStart = in_krsHttpHeader.find( HTTP_CONTENT_LENGTH ) + sizeof( HTTP_CONTENT_LENGTH ) - 1;
   size_t sizeEnd = in_krsHttpHeader.find( CRLF, sizeStart );

   std::string sContentLength = in_krsHttpHeader.substr( sizeStart, sizeEnd - sizeStart );

   if( sContentLength.length() && sContentLength.find_first_not_of( "0123456789" ) == std::string::npos )
   {
      return std::stoull( sContentLength );
   }

   return 0;
}

//---------------------------------------------------------------------------------------------------------------------
bool HttpRequestParserAdvance::STATIC_IsHeaderComplete( const std::string & in_krsHttpHeader )
{
   if( in_krsHttpHeader.size() > SIZE_OF_HTTP_BODY_SEPERATOR )
   {
      return ( in_krsHttpHeader.substr( in_krsHttpHeader.size() - SIZE_OF_HTTP_BODY_SEPERATOR ) == HTTP_BODY_SEPERATOR );
   }

   return false;
}

//---------------------------------------------------------------------------------------------------------------------
bool HttpRequestParserAdvance::STATIC_AppendData( const std::string & in_krsData, std::string & io_krsHttpHeader, std::string & io_krsHttpBody )
{
   if( in_krsData.empty() ) return false;

   if( STATIC_IsHeaderComplete( io_krsHttpHeader ) )
   {
      io_krsHttpBody.append( in_krsData );
      return( io_krsHttpBody.size() == STATIC_ParseForContentLength( io_krsHttpHeader ) );
   }

   size_t ullSeperatorIndex = in_krsData.find( HTTP_BODY_SEPERATOR );
   if( ullSeperatorIndex == std::string::npos )
   {
      io_krsHttpHeader.append( in_krsData );
   }
   else
   {
      io_krsHttpHeader.append( in_krsData.substr( 0, ullSeperatorIndex + SIZE_OF_HTTP_BODY_SEPERATOR ) );
      io_krsHttpBody.append( in_krsData.substr( ullSeperatorIndex + SIZE_OF_HTTP_BODY_SEPERATOR ) );
   }
   return STATIC_IsHeaderComplete( io_krsHttpHeader ) && ( io_krsHttpBody.size() == STATIC_ParseForContentLength( io_krsHttpHeader ) );
}
