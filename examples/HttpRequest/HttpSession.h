/*

(c) Copyright Matrox Electronic Systems Ltd., 2018. All rights reserved.

This code and information is provided "as is" without warranty of any kind,
either expressed or implied, including but not limited to the implied
warranties of merchantability and/or fitness for a particular purpose.

Disclaimer: Matrox Electronic Systems Ltd. reserves the right to make
changes in specifications and code at any time and without notice.
No responsibility is assumed by Matrox Electronic Systems Ltd. for
its use; nor for any infringements of patents or other rights of
third parties resulting from its use. No license is granted under
any patents or patent rights of Matrox Electronic Systems Ltd.

*/

#pragma once
#include "BaseSocket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpSession
{
public:
   HttpSession( BaseSocketCreator* in_poSocketCreator );
   ~HttpSession();

   HRESULT Connect( const std::string& in_krsHostName, uint16_t in_ui16PortNumber );
   HRESULT Reconnect();

   HRESULT SendRequest( HttpRequestMethod in_eRequestMethod, const std::string& in_krsRelativeUri );
   HRESULT SendRequest( const HttpRequest& in_kroHttpRequest );
   HRESULT GetResponse( HttpResponse& out_roHttpResponse );

protected:
   enum HttpSessionStatus
   {
      keHttpSessionInvalid,
      keHttpSessionConnected,
      keHttpSessionSendRequest,
      keHttpSessionGetResponse,
      keHttpSessionReconnectRequired,
   };

   BaseSocketCreator* m_poSocketCreator;
   BaseActiveSocket* m_poSocket;
   std::string m_sHostName;
   uint16_t m_ui16RemotePortNumber;
   HttpSessionStatus m_eSssionStatus;
};
