
#include <future>
#include "PassiveSocket.h"       // Include header for active socket object definition

#define MAX_PACKET 4096 
#define TEST_PACKET "Test Packet"

int EchoServerThread(std::future<void> exitEvent);

int main(int argc, char **argv)
{
    std::promise<void> exitSignal;
    std::future<void> exitEvent = exitSignal.get_future();

    // Starting Thread & move the future object in lambda function by reference
    std::thread ServerThread(&EchoServerThread, std::move(exitEvent));
    ServerThread.detach();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    CActiveSocket client;
    char result[1024];

    client.Initialize(); // Initialize our socket object 

    if (!client.Open("127.0.0.1", 6789)) // Connect to echo server
    {
        printf("Failed to connect..\n");
        return -1;
    }

    uint16 serverPort = client.GetServerPort();
    if (serverPort != 6789)
    {
        printf("Server Port Mismatch..\n");
        return -1;
    }

    const char* serverAddr = client.GetServerAddr();
    if (strcmp(serverAddr, "127.0.0.1"))
    {
        printf("Server Address Mismatch..\n");
        return -1;
    }

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
            printf("Connection lost...\n");
            return -1;
        }
    }

    client.Close();

    exitSignal.set_value();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 1;
}

int EchoServerThread(std::future<void> exitEvent)
{
    CPassiveSocket socket;
    CActiveSocket *pClient = NULL;

    // Initialize our socket object 
    socket.Initialize();

    socket.Listen("127.0.0.1", 6789);

    while (exitEvent.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout)
    {
        if ((pClient = socket.Accept()) != NULL)
        {
            const char* clientAddr = pClient->GetClientAddr();
            printf("New Client from %s on %d\n", 
                   clientAddr,
                   pClient->GetClientPort() );

            // Receive request from the client.
            if (pClient->Receive(MAX_PACKET))
            {
                // Send response to client and close connection to the client.
                pClient->Send(pClient->GetData(), pClient->GetBytesReceived());
                pClient->Close();
            }

            delete pClient;
        }
    }

    socket.Close();

    return 1;
}
