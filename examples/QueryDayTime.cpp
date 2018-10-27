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

#include "ActiveSocket.h" // Include header for active socket object definition
#include <iostream>

int main( int argc, char** argv )
{
   CActiveSocket oSocket; // Instantiate active socket object (defaults to TCP).

   oSocket.Initialize(); // Initialize our socket object

   if( oSocket.Open( "time-C.timefreq.bldrdoc.gov", 13 ) ) // Attempt connection to known remote server
   {
      if( oSocket.Send( "\n"_bytes, 1 ) ) // Send a request the server for the current time.
      {
         if( oSocket.Receive( 48 ) ) // Receive response from the server.
            std::cout << oSocket.GetData() << std::endl;
         else
            std::cout << "Unable to obtain time!" << std::endl;
      }
   }

   oSocket.Close(); // Close the connection.

   return 1;
}
