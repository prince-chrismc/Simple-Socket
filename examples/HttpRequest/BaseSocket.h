
#pragma once

class BaseActiveSocket abstract
{
public:
   virtual bool Open(const unsigned char* in_kpszAddress, unsigned short in_ushPort) = 0;
   virtual bool Send(const unsigned char* in_kpszBuffer, size_t in_sizeBytesToSend) = 0;
   virtual unsigned short Receive(short* out_pshSocketStatus, unsigned int in_uiMaxBytes = 1) = 0;
   virtual unsigned char* GetData() = 0;
   virtual bool Close() = 0;
};

class BasePassiveSocket abstract
{
public:
   virtual BaseActiveSocket* Accept() = 0;
   virtual bool Listen(const unsigned char* in_kpszAddress, unsigned short in_ushPort) = 0;
   virtual bool Close() = 0;
};

class BaseSocketCreator abstract
{
public:
   virtual BaseActiveSocket* NewBaseActiveSocket() = 0;
   virtual BasePassiveSocket* NewBasePassiveSocket() = 0;
};
