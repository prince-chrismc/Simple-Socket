
#include "HttpSession.h"

HttpSession::HttpSession( BaseSocketCreator* in_poSocketCreator ) : m_poSocketCreator( in_poSocketCreator ), m_poSocket( NULL )
, m_sHostName( "" ), m_ui16RemotePortNumber( 0 ), m_eSssionStatus( keHttpSessionInvalid )
{
   m_poSocket = m_poSocketCreator->NewBaseActiveSocket();
}

HttpSession::~HttpSession()
{
   if( m_poSocket )
   {
      m_poSocket->Close();
      delete m_poSocket;
      m_poSocket = NULL;
   }

   m_eSssionStatus = keHttpSessionInvalid;
}

bool HttpSession::Connect( const std::string& in_krsHostName, uint16_t in_ui16PortNumber )
{
   bool retval;

   retval = m_poSocket->Open( (const unsigned char*)in_krsHostName.c_str(), in_ui16PortNumber );

   if( retval )
   {
      m_sHostName = in_krsHostName;
      m_ui16RemotePortNumber = in_ui16PortNumber;
      m_eSssionStatus = keHttpSessionConnected;
   }

   return retval;
}

bool HttpSession::Reconnect()
{
   bool retval;

   if( m_poSocket )
   {
      m_poSocket->Close();
      delete m_poSocket;
      m_poSocket = NULL;
   }

   m_poSocket = m_poSocketCreator->NewBaseActiveSocket();

   retval = Connect( m_sHostName, m_ui16RemotePortNumber );

   return retval;
}

bool HttpSession::SendRequest( HttpRequestMethod in_eRequestMethod, const std::string & in_krsRelativeUri )
{
   bool retval;

   if( m_eSssionStatus == keHttpSessionInvalid )
   {
      retval = false;
   }

   if( retval )
   {
      HttpRequest oRequest( in_eRequestMethod, in_krsRelativeUri, keHttpVersion11, m_sHostName + std::to_string( (long long)m_ui16RemotePortNumber ) );

      std::string sWrireRequest = oRequest.GetWireFormat();
      retval = m_poSocket->Send( (const unsigned char*)sWrireRequest.c_str(), sWrireRequest.size() );
   }

   return retval;
}

bool HttpSession::SendRequest( const HttpRequest & in_kroHttpRequest )
{
   bool retval;

   if( m_eSssionStatus == keHttpSessionInvalid )
   {
      retval = false;
   }

   if( retval )
   {
      std::string sWrireRequest = in_kroHttpRequest.GetWireFormat();
      retval = m_poSocket->Send( (const unsigned char*)sWrireRequest.c_str(), sWrireRequest.size() );
   }

   return retval;
}

bool HttpSession::GetResponse( HttpResponse & out_roHttpResponse )
{
   bool retval;

   if( m_eSssionStatus == keHttpSessionInvalid )
   {
      retval = false;
   }

   std::string sResponse = "";
   short shSocketError = 0;
   unsigned short shBytesRcvd = 0;
   if( retval )
   {
      shBytesRcvd = m_poSocket->Receive( &shSocketError, 3000 );
      if( shBytesRcvd == 0 || shSocketError < 0 )
      {
         retval = false;
      }
   }

   if( retval )
   {
      do
      {
         sResponse.append( (const char*)m_poSocket->GetData() );
         shBytesRcvd = m_poSocket->Receive( &shSocketError, 3000 );
      } while( shBytesRcvd > 0 && shSocketError == 0 );
   }

   if( retval )
   {
      HttpResponseParser oParser( sResponse );
      out_roHttpResponse = oParser.GetHttpResponse();

      if( HttpRequestParser::STATIC_ParseForVersion( sResponse ) == keHttpVersion10 )
         m_eSssionStatus = keHttpSessionReconnectRequired;
   }

   return retval;
}
