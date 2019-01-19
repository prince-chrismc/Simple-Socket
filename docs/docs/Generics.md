---
id: Generics
title: General Defitions
---
General Defitions for the usage of and interaction with sockets.

### Table of Contents
- Functions
   - length
   - DescribeError
- Enums
   - CShutdownMode
   - CSocketType
   - CSocketError

## Functions
```cpp
// Calculates the length of a C string at compilation
auto constexpr length( const char* str )->long { return *str ? 1 + length( str + 1 ) : 0; }
```

```cpp
/// Returns a human-readable description of the given error code
static std::string DescribeError(CSocketError err);
```

## Enums
### CShutdownMode
Defines the three possible states for shuting down a socket.

```cpp
/// Shutdown shut down socket send and receive operations
///    CShutdownMode::Receives - Disables further receive operations.
///    CShutdownMode::Sends    - Disables further send operations.
///    CShutdownBoth::Both     - Disables further send and receive operations.
```

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
SocketInvalidOperation    | An invalid argument was provide for the requested action.
SocketAlreadyConnected    | A requested action was not possible because of socket state.
SocketRoutingError        | OS could not resolve route for requested operation
SocketEunknown            | Unknown error please report to prince.chrismc@gmail.com
