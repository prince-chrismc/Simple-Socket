
#include "ActiveSocket.h"       // Include header for active socket object definition

#define MAX_PACKET 4096
#define TEST_PACKET "Test Packet"

int main(int argc, char **argv)
{
    CActiveSocket client;
    char result[1024];
    
    client.Initialize(); // Initialize our socket object 

    if (client.Open("127.0.0.1", 6789)) // Connect to echo server
    {
        if (client.Send((uint8 *)TEST_PACKET, strlen(TEST_PACKET)))
        {
            int numBytes = -1;
            int bytesReceived = 0;

            numBytes = client.Receive(MAX_PACKET);
            if (numBytes > 0)
            {
                memset(result, 0, 1024);
                memcpy(result, client.GetData(), numBytes);
                printf("received %d bytes: '%s'\n", numBytes, result);
            }
            else
            {
                printf("Connection disconnected...\n");
            }
        }
    }

    client.Close();

    return 1;
}
