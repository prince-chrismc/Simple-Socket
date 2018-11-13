---
id: Core
title: Core Socket Functionality
---
`CSimpleSocket` provides a platform independent class for application layer development. This class is designed to abstract socket communication development in a
platform _and_ protocol independent manner.

### Table of Contents
- Active Socket
- Functionality
   - Open # Active Only
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

## Functionality
### Open
> Note: This function is only available on `CActiveSocket`
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
virtual int32 Receive(uint32 nMaxBytes = 1, uint8 * pBuffer = nullptr);
```

### Get Data
```cpp
/// Get a pointer to internal receive buffer. This memory is managed
/// internally by the CSocket class.
/// @return copy of data if valid, else returns empty.
str::string GetData();
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
### Get Client Address
```cpp
 /// Returns clients Internet host address as a string in standard numbers-and-dots notation.
 ///  @return IP address or empty if invalid
std::string GetClientAddr();
```

### Get Server Address
```cpp
/// Returns server Internet host address as a string in standard numbers-and-dots notation.
///  @return IP address or empty if invalid
std::string GetServerAddr();
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
