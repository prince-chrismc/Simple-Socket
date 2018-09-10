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

/*
 *
 * Parses command line. Every element of the command line must be separated by space.
 * Additional interpretation is as follows:
 * defined on CL as "/a=b" = pair
 * defined on CL as "/a"   = switch
 * defined on CL as "a"    = non-interpreted
 * The '/' can also be a '-'
 * Arguments are case insensitive.
 *
 */

#pragma once

#include <string>
#include <vector>

class CommandLineParser
{
public:
    CommandLineParser(int argc, char** argv);
    ~CommandLineParser() = default;

    bool        DoesSwitchExists(const std::string& name);
    std::string GetPairValue(std::string name);
    std::string GetNonInterpted(const size_t index);

private:
    void Parse(int argc, char** argv);

    std::vector<std::string>  m_vecArgs;   // all args written on command line
    std::string               m_sCommand;  // the name of the program invoked
};
