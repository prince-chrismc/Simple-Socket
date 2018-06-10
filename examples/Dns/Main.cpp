
#include <string>
#include "DnsQuery.h"
#include "ActiveSocket.h"       // Include header for active socket object definition

#define MAX_PACKET 4096

int main(int argc, char **argv)
{
    bool retval = false;
    CActiveSocket oClient(CSimpleSocket::SocketTypeUdp);

    retval = oClient.Initialize();

    if(retval)
        retval = oClient.Open("8.8.8.8", 53);

    if(retval)
        retval = oClient.Send((const uint8*)"\n", sizeof("\n"));

    std::string sData;
    while (retval)
    {
        uint8* buffer = new uint8[5120];
        int32 bytes_rcvd = oClient.Receive(5120, buffer);
        if (bytes_rcvd > 0) sData.append((char*)buffer, bytes_rcvd);
        if (bytes_rcvd == 5120) retval = false;

        delete[] buffer;
    }

    oClient.Close();

    return 0;
}
