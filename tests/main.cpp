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

    //if (client.GetMulticast())
    //{
    //    printf("Client should not be multicast..\n");
    //    return -1;
    //}

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

    getchar();
    return 1;
}

int EchoServerThread(std::future<void> exitEvent)
{
    CPassiveSocket socket;
    std::unique_ptr<CActiveSocket> pClient = nullptr;

    // Initialize our socket object
    socket.Initialize();

    socket.Listen("127.0.0.1", 6789);

    while (exitEvent.wait_for(std::chrono::milliseconds(10)) == std::future_status::timeout)
    {
        if ((pClient = socket.Accept<std::unique_ptr, CActiveSocket>()) != nullptr)
        {
#ifdef _LINUX
            const char* clientAddr = pClient->GetClientAddr();
            printf("New Client from %s on %d\n",
                   clientAddr,
                   pClient->GetClientPort() );
#elif _WIN32
            printf("New Client from %s on %d\n",
                pClient->GetClientAddr(),
                pClient->GetClientPort());
#endif
            // Receive request from the client.
            if (pClient->Receive(MAX_PACKET))
            {
                // Send response to client and close connection to the client.
                pClient->Send(pClient->GetData(), pClient->GetBytesReceived());
                pClient->Close();
            }
        }
    }

    socket.Close();

    return 1;
}
