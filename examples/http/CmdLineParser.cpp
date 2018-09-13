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

#include "CmdLineParser.h"
#include <algorithm>
//#include <execution>

CommandLineParser::CommandLineParser( int argc, char** argv )
{
   Parse( argc, argv );
}

void CommandLineParser::Parse( int argc, char** argv )
{
   m_sCommand = argv[ 0 ]; // Save filename
   for( int i = 1; i < argc; i++ ) // Then save each arg
   {
      m_vecArgs.emplace_back( argv[ i ] );
   }
}

bool CommandLineParser::DoesSwitchExists( const std::string& name )
{
   if( name.empty() ) return false;

   return std::find_if( /*std::execution::par_unseq,*/ m_vecArgs.begin(), m_vecArgs.end(), [ &name = name ]( const std::string& arg )->bool
                        {
                           return ( arg.front() == '/' || arg.front() == '-' ) && ( arg.find( name ) != std::string::npos );
                        } ) != std::end( m_vecArgs );
}

std::string CommandLineParser::GetPairValue( std::string name )
{
   if( name.empty() ) return "";

   name += "=";
   std::string retval;
   for( auto& pair : m_vecArgs )
   {
      const size_t switch_index = pair.find( name );
      if( switch_index != std::string::npos )
      {
         retval = pair.substr( switch_index + name.size() + 1 ); // Return string after =.
         break;
      }
   }

   return retval;
}

std::string CommandLineParser::GetNonInterpted( size_t index )
{
   if( m_vecArgs.size() > index ) return "";

   size_t nCounter = 0;
   std::string retval;

   for( auto& arg : m_vecArgs )
   {
      if( arg.front() == '/' || arg.front() == '-' ) continue;
      if( ++nCounter == index ) retval = arg;
   }

   return retval;
}
