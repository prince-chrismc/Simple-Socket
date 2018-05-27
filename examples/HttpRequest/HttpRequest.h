
#pragma once

#include <string>
#include "Constants.h"

enum HttpRequestMethod
{
   keHttpRequestInvalid = 0,
   keHttpRequestOptions,
   keHttpRequestGet,
   keHttpRequestHead,
   keHttpRequestPost,
   keHttpRequestPut,
   keHttpRequestDelete,
   keHttpRequestTrace,
   keHttpRequestConnect,
   keHttpRequestPatch,
   keHttpRequestLast
};

class HttpRequest
{
public:
   HttpRequest( const HttpRequestMethod & in_kreMethod, const std::string & in_krsRequestUri,
                   const HttpVersion & in_kreVersion, const std::string & in_krsHostAndPort );
   ~HttpRequest() { }

   bool IsValidRequest() const;

   void SetContentType( const HttpContentType& in_kreContentType );
   void AppendMessageBody( const std::string & in_krsToAdd );

   const HttpRequestMethod& GetMethod() const { return m_eMethod; }
   const std::string&          GetUri() const { return m_sRequestUri; }
   const HttpVersion&       GetVersion() const { return m_eVersion; }
   const std::string&          GetHostAndPort() const { return m_sHostAndPort; }
   const HttpContentType&   GetContentType() const { return m_eContentType; }
   const std::string&          GetBody() const { return m_sBody; }

   // Returns the basic information in request format does not include body
   std::string GetRawRequest() const;

   // Returns a proper request with the basic information provided
   std::string GetWireFormat() const;

   static std::string STATIC_MethodAsString( const HttpRequestMethod& in_kreMethod );
   static std::string STATIC_VersionAsString( const HttpVersion& in_kreVersion );
   static std::string STATIC_ContentTypeAsString( const HttpContentType& in_kreContentType );

private:
   HttpRequestMethod m_eMethod;
   std::string m_sRequestUri;
   HttpVersion m_eVersion;
   std::string m_sHostAndPort;

   // Optional
   HttpContentType m_eContentType;
   std::string m_sBody;
};

class HttpRequestParser
{
public:
   HttpRequestParser( const std::string& in_krsRequest ) : m_sRequestToParse( in_krsRequest ) { }

   HttpRequest GetHttpRequest();

   static HttpRequestMethod STATIC_ParseForMethod( const std::string& in_krsRequest );
   static std::string STATIC_ParseForRequestUri( const std::string& in_krsRequest );
   static HttpVersion STATIC_ParseForVersion( const std::string& in_krsRequest );
   static std::string STATIC_ParseForHostAndPort( const std::string& in_krsRequest );
   static HttpContentType STATIC_ParseForContentType( const std::string& in_krsRequest );
   static std::string STATIC_ParseForBody( const std::string& in_krsRequest );

private:
   std::string m_sRequestToParse;
};
