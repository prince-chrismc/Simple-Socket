---
id: Core
title: Core Socket Functionality
---
`CSimpleSocket` provides a platform independent class for application layer development. This class is designed to abstract socket communication development in a
platform _and_ protocol independent manner.

### Table of Contents
- Active Socket
   - Open
- Passive Socket
   - Listen
   - Accept
- Functionality
   - Receive
   - Get Data
   - Shutdown
   - Close
- Status
   - Is Socket Valid
   - Select
   - Describe Error
- Information
   - Get Client Address
   - Get Server Address
   - Get Joined Group
   - Get Socket Error
   - Get Bytes Received
   - Get Bytes Sent

## Active Socket
```cpp
/// Provides a platform independent class to create an active socket.
/// An active socket is used to create a socket which connects to a server.
/// This type of object would be used when an application needs to send/receive
/// data with a server.
class CActiveSocket : public CSimpleSocket
```

### Open
```cpp
/// Established a connection to the address specified by pAddr.
/// Connection-based protocol sockets (CSocket::SocketTypeTcp) may
/// successfully call Open() only once, however; connectionless protocol
/// sockets (CSocket::SocketTypeUdp) may use Open() multiple times to
/// change their association.
///  @param pAddr specifies the destination address to connect.
///  @param nPort specifies the destination port.
///  @return true if successful connection made, otherwise false.
virtual bool Open( const char *pAddr, uint16 nPort );
```

## Passive Socket
```cpp
/// Provides a platform independent class to create a passive socket.
/// A passive socket is used to create a "listening" socket.  This type
/// of object would be used when an application needs to wait for
/// inbound connections.  Support for CSimpleSocket::SocketTypeTcp,
/// CSimpleSocket::SocketTypeUdp, and CSimpleSocket::SocketTypeRaw is handled
/// in a similar fashion.  The big difference is that the method
/// CPassiveSocket::Accept should not be called on the latter two socket
/// types.
class CPassiveSocket : public CSimpleSocket
```

### Listen
```cpp
/// Create a listening socket at local ip address 'x.x.x.x' or 'localhost'
/// if pAddr is NULL or empty, use ANY_ADDR
/// if nPort is 0, use any open port.
/// NOTE: This operation can only be called once!
///
///  @param pAddr specifies the IP address on which to listen.
///  @param nPort specifies the port on which to listen.
///  @param nConnectionBacklog specifies connection queue backlog (default 30,000)
///  @return true if a listening socket was created.
///      If not successful, the false is returned and one of the following error
///      conditions will be set: CPassiveSocket::SocketAddressInUse, CPassiveSocket::SocketProtocolError,
///      CPassiveSocket::SocketInvalidSocket.  The following socket errors are for Linux/Unix
///      derived systems only: CPassiveSocket::SocketInvalidSocketBuffer
bool Listen( const char *pAddr, uint16 nPort, int32 nConnectionBacklog = 30000 );
```

### Accept
```cpp
/// Extracts the first connection request on the queue of pending
/// connections and creates a newly connected socket.  Used with
/// CSocketType CSimpleSocket::SocketTypeTcp.  It is the responsibility of
/// the caller to delete the returned object when finished.
///  @template SmartPtr can be either std::shared_ptr or std::unique_ptr
///  @template SocketBase can be either CSimpleSocket or CActiveSocket
///  @return if successful a pointer to a newly created CActiveSocket object
///          will be returned and the internal error condition of the CPassiveSocket
///          object will be CPassiveSocket::SocketSuccess.  If an error condition was encountered
///          the nullptr will be returned and one of the following error conditions will be set:
///    CPassiveSocket::SocketEwouldblock, CPassiveSocket::SocketInvalidSocket,
///    CPassiveSocket::SocketConnectionAborted, CPassiveSocket::SocketInterrupted
///    CPassiveSocket::SocketProtocolError, CPassiveSocket::SocketFirewallError
auto Accept()-> std::unique_ptr<CActiveSocket>;
```

## Functionality
### Receive
The internal buffer is only valid until the next call to Receive() returns, or until the object goes out of scope.
```cpp
/// Attempts to receive a block of data on an established connection.
/// @param nMaxBytes maximum number of bytes to receive.
/// @param pBuffer, memory where to receive the data,
///        - NULL receives to internal buffer returned with GetData()
///        - Non-NULL receives directly there, but GetData() will return empty!
/// @return number of bytes actually received.
/// @return of zero means the connection has been shutdown on the other side.
/// @return of -1 means that an error has occurred.
int32 Receive(uint32 nMaxBytes = 1, uint8 * pBuffer = nullptr);
```

### Get Data
```cpp
/// Get a pointer to internal receive buffer. This memory is managed
/// internally by the CSocket class.
/// @return copy of data if valid, else returns empty.
const std::string& GetData();
```

### Shutdown
```cpp
/// Shutdown shutdown socket send and/or receive operations
/// @param nShutdown specifies the type of shutdown.
/// @return true if successfully shutdown otherwise returns false.
bool Shutdown(CShutdownMode nShutdown);
```
> NOTE: this method is able to allowing blocking sockets to close **ONLY** if the socket currently is not in a blocking state; in other works if your application is waiting for a message close will most liekly be required to exit the blocking call.

### Close
```cpp
/// Close socket
/// @return true if successfully closed otherwise returns false.
bool Close();
```

## Status
### Is Socket Valid
```cpp
/// Does the current instance of the socket object contain a valid socket
/// descriptor.
///  @return true if the socket object contains a valid socket descriptor.
bool IsSocketValid() const;
```

### Select
```cpp
/// Examine the socket descriptor currently managed by this instance to see
/// whether some of its file descriptors are ready for reading, for writing,
/// or have an exceptional condition pending,
/// @param nTimeoutSec timeout in seconds for select.
/// @param nTimeoutUSec timeout in micro seconds for select.
/// @return true if socket has data ready, or false if timed out or error pending.
bool Select(int32 nTimeoutSec, int32 nTimeoutUSec);
```

```cpp
/// Block until an event happens on the managed socket descriptors.
/// @return true if socket has data ready, or false if not ready or timed out.
bool Select();
```

### Describe Error
```cpp
/// Returns a human-readable description of the last error code of a socket
std::string DescribeError() const;
```
## Information
### Get Socket Type
```cpp
/// Return socket descriptor
///  @return socket descriptor which is a signed 32 bit integer.
CSocketType GetSocketType() const;
```

### Get Client Address
```cpp
 /// Returns clients Internet host address as a string in standard numbers-and-dots notation.
 ///  @return IP address or empty if invalid
std::string GetClientAddr();
```

### Get Client Port
```cpp
/// Returns the port number on which the client is connected.
///  @return client port number.
uint16_t GetClientPort() const;
```

### Get Server Address
```cpp
/// Returns server Internet host address as a string in standard numbers-and-dots notation.
///  @return IP address or empty if invalid
std::string GetServerAddr();
```

### Get Server Port
```cpp
/// Returns the port number on which the server is connected.
///  @return server port number.
uint16_t GetServerPort() const;
```

### Get Joined Group
```cpp
/// Returns the multi-cast group's address as a string in standard numbers-and-dots notation.
///  @return IP address or empty if invalid
std::string GetJoinedGroup();
```
### Get Socket Error
```cpp
/// Returns the last error that occured for the instace of the CSimpleSocket
/// instance.  This method should be called immediately to retrieve the
/// error code for the failing mehtod call.
///  @return last error that occured.
CSocketError GetSocketError() const;
```

### Get Bytes Received
```cpp
/// Returns the number of bytes received on the last call to
/// CSocket::Receive().
/// @return number of bytes received.
int32 GetBytesReceived() const;
```

### Get Bytes Sent
```cpp
/// Returns the number of bytes sent on the last call to
/// CSocket::Send().
/// @return number of bytes sent.
int32 GetBytesSent() const;
```
