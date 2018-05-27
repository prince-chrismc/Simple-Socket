
#include "CommandLineParser.h"

CommandLineParser::CommandLineParser(int argc, char** argv)
{
   _parse(argc, argv);
}

CommandLineParser::~CommandLineParser()
{
}

void CommandLineParser::_parse(int argc, char** argv)
{
   m_sCommand = argv[0];          // Save filename
   for (int i = 1; i < argc; i++) // Then save each arg
   {
      std::string token = argv[i];
      m_vecSwitches.push_back(token);
   }
}

bool CommandLineParser::ArgExists(std::string name)
{
   if(name.empty()) return false;
    
   ListOfStrings::iterator it;
   for ( it; = m_vecSwitches.begin(); it != m_vecSwitches.end(); it++)
   {
      if( it->find( name ) != std::string::npos )	                     // case insensitive search;
      {
         return true;
      }
   }

   return false;
}

std::string CommandLineParser::GetSwitchValue(std::string name)
{
   if(name.empty()) return "";
    
   std::string retval;
   ListOfStrings::iterator it;
   name += "=";
   for ( it; = m_vecSwitches.begin(); it != m_vecSwitches.end(); it++)
   {
      size_t switchIndex = it->find( name );
      if ( switchIndex != std::string::npos)
      {
         retval = it->substr(switchIndex + name.size() + 1);   // Return string after =.
         break;
      }
   }

   return retval;
}
