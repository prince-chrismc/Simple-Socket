
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "ActiveSocket.h"


int main(int argc, char **argv)
{
    bool retval = true;
    CActiveSocket oClient;

    retval = oClient.Initialize();

    if (retval)
        retval = oClient.Open("www.google.ca", 80);

    if (retval)
    {
        HttpRequest oReq(HttpRequestGet, "/", HttpVersion11, "www.google.ca");
        oReq.SetContentType(HttpContentHtml);
        std::string sRawRequest = oReq.GetWireFormat();
        retval = oClient.Send((uint8*)sRawRequest.c_str(), sRawRequest.size());
    }

    std::string sData;
    while (retval)
    {
        uint8* buffer = new uint8[5120];
        int32 bytes_rcvd = oClient.Receive(5120, buffer);
        if (bytes_rcvd > 0) sData.append((char*)buffer, bytes_rcvd);
        if (bytes_rcvd == 5120) retval = false;

        delete[] buffer;
    }

    HttpResponseParser oParser(sData);
    HttpResponse oRes = oParser.GetHttpResponse();

    oClient.Close();

    return 0;
}
