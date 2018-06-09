
#include "ActiveSocket.h"
#include "HttpRequest.h"
#include "HttpResponse.h"


int main(int argc, char **argv)
{
    CActiveSocket oClient;

    oClient.Initialize();

    HttpRequest oReq(HttpRequestGet, "/", HttpVersion11, "www.google.ca");

    oClient.Open("www.google.ca", 80);

    std::string sRawRequest = oReq.GetWireFormat();
    oClient.Send((uint8*)sRawRequest.c_str(), sRawRequest.size());

    uint8* buffer = new uint8[500];
    oClient.Receive(500, buffer);
}
