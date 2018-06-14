
#include <thread>
#include "PassiveSocket.h"

#define MAX_PACKET  4096
#define TEST_PACKET "Test Packet"

struct thread_data
{
    const char *pszServerAddr;
    short int   nPort;
    int         nNumBytesToReceive;
    int         nTotalPayloadSize;
};


void *CreateTCPEchoServer(void *param)
{
    CPassiveSocket socket;
    CActiveSocket *pClient = NULL;
    struct thread_data *pData = (struct thread_data *)param;
    int            nBytesReceived = 0;

    socket.Initialize();
    socket.Listen(pData->pszServerAddr, pData->nPort);

    if ((pClient = socket.Accept()) != NULL)
    {
        while (nBytesReceived != pData->nTotalPayloadSize)
        {
            if (nBytesReceived += pClient->Receive(pData->nNumBytesToReceive))
            {
                pClient->Send((const uint8 *)pClient->GetData(), pClient->GetBytesReceived());
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        delete pClient;
    }

    socket.Close();

    return NULL;
}

int main(int argc, char **argv)
{
    std::thread*       thread = nullptr;
    thread_data* thData = new thread_data();
    CActiveSocket      client;
    char result[1024];

    thData->pszServerAddr = "127.0.0.1";
    thData->nPort = 6789;
    thData->nNumBytesToReceive = 1;
    thData->nTotalPayloadSize = (int)strlen(TEST_PACKET);

    thread = new std::thread(CreateTCPEchoServer, thData);
    thread->detach();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    client.Initialize();
    client.SetNonblocking();

    if (client.Open("127.0.0.1", 6789))
    {
        if (client.Send((uint8 *)TEST_PACKET, strlen(TEST_PACKET)))
        {
            int numBytes = -1;
            int bytesReceived = 0;

            client.Select();

            while (bytesReceived != strlen(TEST_PACKET))
            {
                numBytes = client.Receive(MAX_PACKET);

                if (numBytes > 0)
                {
                    bytesReceived += numBytes;
                    memset(result, 0, 1024);
                    memcpy(result, client.GetData(), numBytes);
                    printf("received %d bytes: '%s'\n", numBytes, result);
                }
                else
                {
                    printf("Received %d bytes\n", numBytes);
                }
            }
        }
    }

    thread->join();
    delete thread;
    delete thData;
}
