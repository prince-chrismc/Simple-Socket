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

#include <string>
#include <future>
#include "PassiveSocket.h" // Include header for both passive and active socket object definition

using namespace std::chrono_literals;

auto WireToText = []( const uint8* text ) constexpr { return (const char*)text; };

static constexpr const int32 NEXT_BYTE = 1;
static constexpr const char* TEST_PACKET = "Test Packet";
static constexpr const char* LOCAL_HOST = "127.0.0.1";

// ---------------------------------------------------------------------------------------------
// Message Utility Code
// ---------------------------------------------------------------------------------------------
class AsyncMessage final
{
   friend class AsyncMessageBuilder;
public:
   AsyncMessage( const std::string& sMessage ) : m_sMessage( std::to_string( sMessage.size() ) + "\n" + sMessage ) {}
   AsyncMessage( const AsyncMessage& oNewMessage ) { m_sMessage = oNewMessage.m_sMessage; }
   AsyncMessage( const AsyncMessage&& oNewMessage ) { m_sMessage = oNewMessage.m_sMessage; }
   ~AsyncMessage() = default;

   void operator=( const AsyncMessage& oNewMessage ) { m_sMessage = oNewMessage.m_sMessage; }
   void operator=( const AsyncMessage&& oNewMessage ) { m_sMessage = oNewMessage.m_sMessage; }

   constexpr const std::string& ToString() const { return m_sMessage; }

   const uint8* GetWireFormat() const { return (const uint8*)m_sMessage.c_str(); }
   size_t GetWireFormatSize() const { return m_sMessage.size(); }

private:
   std::string m_sMessage;
};

class AsyncMessageBuilder final
{
public:
   AsyncMessageBuilder() : m_oMessage( "" ), m_iExpectedSize( incomplete ) { m_oMessage.m_sMessage.clear(); }
   explicit AsyncMessageBuilder( const AsyncMessage& oMessage ) : m_oMessage( oMessage ), m_iExpectedSize( incomplete ) { _ParseMessage(); }
   AsyncMessageBuilder( const AsyncMessageBuilder& ) = delete;
   AsyncMessageBuilder( const AsyncMessageBuilder&& ) = delete;
   ~AsyncMessageBuilder() = default;

   const AsyncMessageBuilder& operator=( const AsyncMessageBuilder& ) = delete;
   const AsyncMessageBuilder& operator=( const AsyncMessageBuilder&& ) = delete;

   static constexpr size_t incomplete = -1;

   constexpr bool IsComplete() const
   {
      return m_iExpectedSize != incomplete
         && m_iExpectedSize == m_oMessage.m_sMessage.size();
   }

   size_t Append( const uint8* pData, const size_t iSize )
   {
      m_oMessage.m_sMessage.append( WireToText( pData ), iSize );
      if( ! IsComplete() ) _ParseMessage();
      return m_iExpectedSize;
   }

   const AsyncMessage&& ExtractMessage()
   {
      m_oMessage.m_sMessage = std::to_string( m_iExpectedSize ) + "\n" + m_oMessage.m_sMessage;
      m_iExpectedSize = incomplete;
      return std::move( m_oMessage );
   }

private:
   AsyncMessage m_oMessage;
   size_t m_iExpectedSize;

   void _ParseMessage()
   {
      const size_t iNewLineIndex = m_oMessage.m_sMessage.find_first_of( '\n' );
      if( iNewLineIndex != std::string::npos )
      {
         m_iExpectedSize = std::stoull( m_oMessage.m_sMessage.substr( 0, iNewLineIndex ) );
         m_oMessage.m_sMessage = m_oMessage.m_sMessage.substr( iNewLineIndex + 1 );
      }
   }
};


int main( int argc, char** argv )
{
   std::promise<void> oExitSignal;

   std::promise<uint16> oPortEvent;
   auto oPortRetval = oPortEvent.get_future();

   // ---------------------------------------------------------------------------------------------
   // Server Code
   // ---------------------------------------------------------------------------------------------
   auto oRetval = std::async( std::launch::async, [ oExitEvent = oExitSignal.get_future(), &oPortEvent ]() {
      CPassiveSocket oSocket;

      oSocket.Initialize(); // Initialize our socket object
      oSocket.SetNonblocking(); // Configure this socket to be non-blocking
      oSocket.Listen( LOCAL_HOST, 0 ); // Bind to local host on port any port

      oPortEvent.set_value( oSocket.GetServerPort() );

      while( oExitEvent.wait_for( 10ms ) == std::future_status::timeout )
      {
         std::unique_ptr<CActiveSocket> pClient = nullptr;
         if( ( pClient = oSocket.AcceptUniqueOwnership() ) != nullptr ) // Wait for an incomming connection
         {
            pClient->SetNonblocking(); // Configure new client connection to be non-blocking

            AsyncMessageBuilder oBuilder;

            do
            {
               pClient->Receive( NEXT_BYTE * 3 ); // Receive next byte of request from the client.
               oBuilder.Append( pClient->GetData(), pClient->GetBytesReceived() ); // Gather Message in a buffer
            } while( ! oBuilder.IsComplete() );

            AsyncMessage oEchoMessage( oBuilder.ExtractMessage() );
            pClient->Send( oEchoMessage.GetWireFormat(), oEchoMessage.GetWireFormatSize() ); // Send response to client and close connection to the client.
            pClient->Select(); // Wait for the client to read the message
            pClient->Close(); // Close socket since we have completed transmission
         }
      }

      oSocket.Close(); // Release the bound socket. Must be done to exit blocking accept call
   }
   );

   const uint16 nPort = oPortRetval.get();

   // ---------------------------------------------------------------------------------------------
   // Client Code
   // ---------------------------------------------------------------------------------------------
   CActiveSocket oClient;

   oClient.Initialize(); // Initialize our socket object
   oClient.SetNonblocking(); // Configure this socket to be non-blocking

   if( oClient.Open( LOCAL_HOST, nPort ) ) // Attempt connection to local server and reported port
   {
      AsyncMessage oMessage( TEST_PACKET );
      if( oClient.Send( oMessage.GetWireFormat(), oMessage.GetWireFormatSize() ) ) // Send a message the server
      {
         oClient.Select();

         size_t iTotalBytes = 0;
         while( iTotalBytes != oMessage.GetWireFormatSize() )
         {
            const int iBytesReceived = oClient.Receive( NEXT_BYTE ); // Receive one byte of response from the server.

            if( iBytesReceived > 0 )
            {
               std::string sResult;
               iTotalBytes += iBytesReceived;
               sResult.assign( WireToText( oClient.GetData() ), iBytesReceived );
               printf( "received %d bytes: '%s'\n", iBytesReceived, sResult.c_str() );
            }
         }
      }
   }

   oClient.Close(); // Close the connection.
   oExitSignal.set_value();
   oRetval.get();

   return 1;
}
