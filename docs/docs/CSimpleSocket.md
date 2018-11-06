---
id: CSimpleSocket
title: CSimpleSocket
---
Provides a platform independent class to for socket development. This class is designed to abstract socket communication development in a
platform independent manner. Socket types:
- CActiveSocket Class
- CPassiveSocket Class

### Table of Contents
- Enums
   - CShutdownMode
   - CSocketType
   - CSocketError
- Functions
   - Get Data
   - Get Client Address
   - Get Server Address
   - Get Joined Group

## Enums
### CShutdownMode
Defines the three possible states for shuting down a socket.

Mode | Description
---|---
Receives | Shutdown Rx socket.
Sends    | Shutdown Tx socket.
Both     | Shutdown both active and passive sockets.

### CSocketType
Defines the socket types recognized by CSimpleSocket class.

Type| Description|Notes
---|---|---
SocketTypeInvalid | Invalid socket type. | -
SocketTypeTcp     | Defines socket as TCP socket. | -
SocketTypeUdp     | Defines socket as UDP socket. | -
SocketTypeTcp6    | Defines socket as IPv6 TCP socket. | Not Supported
SocketTypeUdp6    | Defines socket as IPv6 UDP socket. | Not Supported
SocketTypeRaw     | Provides raw network protocol access. | Linux Only

### CSocketError
Defines all error codes handled by the CSimpleSocket class.

Error | Description
---|---
SocketError               | Generic socket error translates to error below.
SocketSuccess             | No socket error.
SocketInvalidSocket       | Invalid socket handle.
SocketInvalidAddress      | Invalid destination address specified.
SocketInvalidPort         | Invalid destination port specified.
SocketConnectionRefused   | No server is listening at remote address.
SocketTimedout            | Timed out while attempting operation.
SocketEwouldblock         | Operation would block if socket were blocking.
SocketNotconnected        | Currently not connected.
SocketEinprogress         | Socket is non-blocking and the connection cannot be completed immediately
SocketInterrupted         | Call was interrupted by a signal that was caught before a valid connection arrived.
SocketConnectionAborted   | The connection has been aborted.
SocketProtocolError       | Invalid protocol for operation.
SocketFirewallError       | Firewall rules forbid connection.
SocketInvalidSocketBuffer | The receive buffer point outside the process's address space.
SocketConnectionReset     | Connection was forcibly closed by the remote host.
SocketAddressInUse        | Address already in use.
SocketInvalidPointer      | Pointer type supplied as argument is invalid.
SocketEunknown            | Unknown error please report to prince.chrismc@gmail.com

## Functions
### Get Data
```cpp
/// Get a pointer to internal receive buffer.  The user MUST not free this
/// pointer when finished.  This memory is managed internally by the CSocket
/// class.
/// @return copy of data if valid, else returns empty.
str::string GetData();
```
### Receive
The internal buffer is only valid until the next call to Receive(), a call to Close(), or until the object goes out of scope.
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
int32 GetBytesSent();
```
### Is Non-blocking
```cpp
/// Returns blocking/non-blocking state of socket.
/// @return true if the socket is non-blocking, else return false.
bool IsNonblocking() const;
```

### Get Connect Timeout Sec
```cpp
/// Gets the timeout value that specifies the maximum number of seconds a
/// call to CSimpleSocket::Open waits until it completes.
/// @return the length of time in seconds
int32 GetConnectTimeoutSec() const;
```

### Get Connect Timeout Nanoseconds
```cpp
/// Gets the timeout value that specifies the maximum number of microseconds
/// a call to CSimpleSocket::Open waits until it completes.
/// @return the length of time in microseconds
int32 GetConnectTimeoutUSec() const;
```
