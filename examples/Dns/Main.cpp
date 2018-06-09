
#include "ActiveSocket.h"       // Include header for active socket object definition
#include <string>

#define MAX_PACKET 4096 

int main(int argc, char **argv)
{
    bool retval = false;
    CActiveSocket oClient(CSimpleSocket::SocketTypeUdp);

    retval = oClient.Initialize();

    if(retval)
    {
        retval = oClient.Open("8.8.8.8", 53);
    }

    if(retval)
    {
        retval = oClient.Send((const uint8*)"\n", sizeof("\n"));
    }

    if(retval)
    {
        retval = ( oClient.Receive(49) > 0 );
    }

    if(retval)
    {
        std::string sData( (const char*)oClient.GetData());
    }

    oClient.Close();

    return 0;
}
