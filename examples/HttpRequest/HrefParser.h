
#include <string>

#pragma once

   class HrefParser
   {
   public:
      static bool STATIC_ParseHrefForProtocol( const std::string& in_krsRelativeUri, std::string& out_roProtocolBuffer );
      static bool STATIC_ParseHrefForHost( const std::string& in_krsRelativeUri, std::string& out_roHostNameBuffer );
      static bool STATIC_ParseHrefForPort( const std::string& in_krsRelativeUri, uint16_t& out_rui16PortNumber );
      static bool STATIC_ParseHrefForUri( const std::string& in_krsRelativeUri, std::string& out_roUriBuffer );
   };
