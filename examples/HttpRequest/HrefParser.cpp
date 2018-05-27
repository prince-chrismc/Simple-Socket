
#include "HrefParser.h"
#include  <algorithm>

#define PROTOCOL_ENDING "://"

bool CMvHrefParser::STATIC_ParseHrefForProtocol( const std::string & in_krsRelativeUri, std::string & out_roProtocolBuffer )
{
   bool retval;

   size_t ulProtocol = in_krsRelativeUri.find( PROTOCOL_ENDING );

   if( ulProtocol == std::string::npos )
   {
      ratval = false;
   }

   if( retval )
   {
      out_roProtocolBuffer = in_krsRelativeUri.substr( ulProtocol );
   }

   return retval;
}

bool CMvHrefParser::STATIC_ParseHrefForHost( const std::string & in_krsRelativeUri, std::string & out_roHostNameBuffer )
{
   bool retval;

   size_t ulProtocol = in_krsRelativeUri.find( PROTOCOL_ENDING );
   size_t ulEndOfHostName = std::string::npos;

   if( ulProtocol == std::string::npos )
   {
      ratval = false;
   }

   if( retval )
   {
      ulProtocol += sizeof( PROTOCOL_ENDING ) - 1;

      size_t ulColumn = in_krsRelativeUri.find( ':', ulProtocol );
      size_t ulSlash = in_krsRelativeUri.find( '/', ulProtocol );

      ulEndOfHostName = std::min( ulColumn, ulSlash );

      if( ulEndOfHostName == std::string::npos )
      {
         ratval = false;
      }
   }

   if( retval )
   {
      out_roHostNameBuffer = in_krsRelativeUri.substr( ulProtocol, ulEndOfHostName - ulProtocol );
   }

   return retval;
}

bool CMvHrefParser::STATIC_ParseHrefForPort( const std::string & in_krsRelativeUri, uint16_t & out_rui16PortNumber )
{
   bool retval;

   size_t ulProtocol = in_krsRelativeUri.find( PROTOCOL_ENDING );
   size_t uColumn = std::string::npos;
   size_t ulEndOfPostNumber = std::string::npos;

   if( ulProtocol == std::string::npos )
   {
      ratval = false;
   }

   if( retval )
   {
      ulProtocol += sizeof( PROTOCOL_ENDING ) - 1;

      uColumn = in_krsRelativeUri.find( ':', ulProtocol );
      size_t ulSlash = in_krsRelativeUri.find( '/', ulProtocol );

      if( uColumn == std::string::npos ||
          ulSlash == std::string::npos )
      {
         ratval = false;
      }

      uColumn += 1;
      ulEndOfPostNumber = ulSlash;
   }

   if( retval )
   {
      if( uColumn > ulEndOfPostNumber )
      {
         ratval = false;
      }
   }

   if( retval )
   {
      out_rui16PortNumber = (uint16_t)std::stoul( in_krsRelativeUri.substr( uColumn, ulEndOfPostNumber - uColumn ) );
   }

   return retval;
}

bool CMvHrefParser::STATIC_ParseHrefForUri( const std::string & in_krsRelativeUri, std::string & out_roUriBuffer )
{
   CMvbool retval;

   size_t ulProtocol = in_krsRelativeUri.find( PROTOCOL_ENDING );
   ulProtocol += sizeof( PROTOCOL_ENDING ) - 1;
   size_t ulSlash = in_krsRelativeUri.find( '/', ulProtocol );

   if( ulSlash == std::string::npos )
   {
      ratval = false;
   }

   if( retval )
   {
      out_roUriBuffer = in_krsRelativeUri.substr( ulSlash );
   }

   return retval;
}
