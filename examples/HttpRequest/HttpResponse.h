
#pragma once

#include <string>
#include "Constants.h"

class HttpResponse
{
public:
   HttpResponse( const HttpVersion & in_kreVersion, const HttpStatus & in_kreStatusCode, const std::string & in_krsReasonPhrase );
   ~HttpResponse();

   void SetContentType( const HttpContentType& in_kreContentType );
   void AppendMessageBody( const std::string & in_krsToAdd );

   const HttpVersion&     GetVersion() const { return m_eVersion; }
   const HttpStatus&      GetStatusCode() const { return m_eStatusCode; }
   const std::string&        GetPhrase() const { return m_sReasonPhrase; }
   const HttpContentType& GetContentType() const { return m_eContentType; }
   const std::string&        GetBody() const { return m_sBody; }

   // Returns the response line ( minimal requirement )
   std::string GetResponseLine() const;

   // Returns the basic information in the response
   std::string GetRawResponse() const;

   // Returns a proper response with the basic information provided
   std::string GetWireFormat() const;

private:
   HttpVersion m_eVersion;
   HttpStatus m_eStatusCode;
   std::string m_sReasonPhrase;

   // Optional
   HttpContentType m_eContentType;
   std::string m_sBody;
};

class HttpResponseParser
{
public:
   HttpResponseParser( const std::string& in_krsRequest ) : m_sResponseToParse( in_krsRequest ) { }

   HttpResponse GetHttpResponse();

   static HttpStatus STATIC_ParseForStatus( const std::string& in_krsRequest );
   static std::string STATIC_ParseForReasonPhrase( const std::string& in_krsRequest );

private:
   std::string m_sResponseToParse;
};