/*
Parses command line. Every element of the command line must be separated by space.
Additional interpretation is as follows:
defined on CL as "/a=b" = pair
defined on CL as "/a"   = switch
defined on CL as "a"    = non-interpreted
The '/' can also be a '-'
Arguments are case insensitive.
*/

#pragma once

#include <string>
#include <vector>

class CommandLineParser
{
public:
    CommandLineParser(int argc, char** argv);
    ~CommandLineParser() = default;

    bool        DoesArgExists(const std::string& name);
    std::string GetPairValue(std::string name);

private:
    void _parse(int argc, char** argv);

    std::vector<std::string>  m_vecArgs;   // all args written on command line
    std::string    m_sCommand;             // the name of the program invoked
};
