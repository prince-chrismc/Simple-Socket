
#include "HttpRequest.h"

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

#include  <algorithm>

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

HttpRequest::HttpRequest( const HttpRequestMethod & in_kreMethod, const std::string & in_krsRequestUri,
                                const HttpVersion & in_kreVersion, const std::string & in_krsHostAndPort ) :
   m_eMethod( in_kreMethod ),
   m_sRequestUri( in_krsRequestUri ),
   m_eVersion( in_kreVersion ),
   m_sHostAndPort( in_krsHostAndPort ),
   m_eContentType( HttpContentInvalid ),
   m_sBody( "" )
{
}

bool HttpRequest::IsValidRequest() const
{
   return ( m_eMethod != HttpRequestInvalid ) || ( m_eMethod != HttpRequestLast ) ||
      ( ! m_sRequestUri.empty() ) || ( m_eVersion != HttpVersionInvalid ) || ( m_eVersion != HttpVersionLast );
}

void HttpRequest::SetContentType( const HttpContentType & in_kreContentType )
{
   m_eContentType = in_kreContentType;
}

void HttpRequest::AppendMessageBody( const std::string & in_krsToAdd )
{
   m_sBody.append( in_krsToAdd.c_str() );
}

std::string HttpRequest::GetRawRequest() const
{
   return STATIC_MethodAsString( m_eMethod ) + " " + m_sRequestUri + " " + STATIC_VersionAsString( m_eVersion ) + CRLF + "Host: " + m_sHostAndPort + CRLF;
}

std::string HttpRequest::GetWireFormat() const
{
   return GetRawRequest() + "User-Agent: clsoct_example/1.0" + CRLF + "cache-control: no-cache" + CRLF +
      "Accept: */*" + CRLF + "Accept-Encoding: gzip,deflate" + CRLF +
      "Connection: ep-alive" + CRLF + "Content-Length: " + std::to_string( ( unsigned long long )m_sBody.size() ) +
      CRLF + STATIC_ContentTypeAsString( m_eContentType ) + CRLF + CRLF + m_sBody;
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

HttpRequest HttpRequestParser::GetHttpRequest()
{
   HttpRequest oRequest( STATIC_ParseForMethod( m_sRequestToParse ),
                          STATIC_ParseForRequestUri( m_sRequestToParse ),
                          STATIC_ParseForVersion( m_sRequestToParse ),
                          STATIC_ParseForHostAndPort( m_sRequestToParse ) );

   oRequest.SetContentType( STATIC_ParseForContentType( m_sRequestToParse ) );

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
   if( ! in_krsRequest.empty() )
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
   size_t ulJHtmlPos = sContentTypeLine.find( "html" );
   size_t ulJsonPos = sContentTypeLine.find( "json" );

   size_t ulMinPos = std::min( ulTextPos, ulJHtmlPos );
   ulMinPos = std::min( ulMinPos, ulJsonPos );

   if( ulMinPos == ulTextPos ) return HttpContentText;
   else if( ulMinPos == ulJHtmlPos ) return HttpContentHtml;
   else if( ulMinPos == ulJsonPos ) return HttpContentJson;
   else return HttpContentInvalid;
}

std::string HttpRequestParser::STATIC_ParseForBody( const std::string & in_krsRequest )
{
   if( in_krsRequest.empty() ) return "";

   size_t ulOffset = in_krsRequest.find( std::string( CRLF ) + CRLF ) + ( sizeof( CRLF ) - 1 ) * 2;

   if( ulOffset == in_krsRequest.size() ) return "";

   return in_krsRequest.substr( ulOffset );
}
