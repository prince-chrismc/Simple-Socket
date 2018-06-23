
#include <string>
#include "ActiveSocket.h" // Include header for active socket object definition

constexpr const uint8* operator"" _byte( const char* text, std::size_t ) { return (const uint8 *)text; }
auto WireToText = []( const uint8* text ) constexpr { return (const char*)text; };

int main( int argc, char** argv )
{
   CActiveSocket oSocket; // Instantiate active socket object (defaults to TCP).
   std::string   sTime;

   oSocket.Initialize(); // Initialize our socket object

   if( oSocket.Open( "time-C.timefreq.bldrdoc.gov", 13 ) ) // Attempt connection to known remote server
   {

      if( oSocket.Send( "\n"_byte, 1 ) ) // Send a request the server for the current time.
      {
         const int iBytes = oSocket.Receive( 49 ); // Receive response from the server.
         sTime.assign( WireToText( oSocket.GetData() ), iBytes );
         printf( "%s\n", sTime.c_str() );
      }
   }

   oSocket.Close(); // Close the connection.

   return 1;
}
