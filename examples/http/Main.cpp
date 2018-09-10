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

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ActiveSocket.h"

int main(int argc, char** argv)
{
    CActiveSocket oClient;
    HttpResponseParserAdvance oParser;

    bool retval = oClient.Initialize();

    if (retval)
    {
        retval = oClient.Open("www.google.ca", 80);
    }

    if (retval)
    {
        HttpRequest oReq(HttpRequestGet, "/", HttpVersion11, "www.google.ca");
        oReq.SetContentType(HttpContentHtml);
        std::string sRawRequest = oReq.GetWireFormat();
        retval = oClient.Send((uint8*)sRawRequest.c_str(), sRawRequest.size());
    }

    if (retval)
    {
        int32 bytes_rcvd = -1;
        do
        {
            int32 bytes_rcvd = oClient.Receive(1024);
        } while( ! oParser.AppendResponseData( std::string( (const char*)oClient.GetData(), 
                                               bytes_rcvd ) ) );
    }

    HttpResponse oRes = oParser.GetHttpResponse();

    oClient.Close();

    return 0;
}
