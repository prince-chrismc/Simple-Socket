---
layout: default
title: CSimpleSocket
description: Documentation for CSimpleSocket
---
# CSimpleSocket
Provides a platform independent class to for socket development. This class is designed to abstract socket communication development in a
platform independent manner. Socket types:
- CActiveSocket Class
- CPassiveSocket Class

### Enums
##### CShutdownMode
Defines the three possible states for shuting down a socket.

Mode | Description
---|---
Receives | Shutdown Rx socket.
Sends    | Shutdown Tx socket.
Both     | Shutdown both active and passive sockets.

##### CSocketType
Defines the socket types recognized by CSimpleSocket class.

Type| Description|Notes
---|---|---
SocketTypeInvalid | Invalid socket type. |
SocketTypeTcp     | Defines socket as TCP socket. |
SocketTypeUdp     | Defines socket as UDP socket. |
SocketTypeTcp6    | Defines socket as IPv6 TCP socket. | Not Supported
SocketTypeUdp6    | Defines socket as IPv6 UDP socket. | Not Supported
SocketTypeRaw     | Provides raw network protocol access. | Linux Only

##### CSocketError
Defines all error codes handled by the CSimpleSocket class.

Error | Description
---|---
SocketError           | Generic socket error translates to error below.
SocketSuccess        | No socket error.
SocketInvalidSocket       | Invalid socket handle.
SocketInvalidAddress     | Invalid destination address specified.
SocketInvalidPort        | Invalid destination port specified.
SocketConnectionRefused   | No server is listening at remote address.
SocketTimedout           | Timed out while attempting operation.
SocketEwouldblock        | Operation would block if socket were blocking.
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
SocketEunknown             | Unknown error please report to prince.chrismc@gmail.com

### Functions
- GetData
```cpp
/// Get a pointer to internal receive buffer.  The user MUST not free this
/// pointer when finished.  This memory is managed internally by the CSocket
/// class.
/// @return pointer to data if valid, else returns NULL.
uint8* GetData();
/// @return copy of data if valid, else returns empty.
str::string GetData();
```

