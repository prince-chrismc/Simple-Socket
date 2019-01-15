---
id: Options
title: Common Socket Options
---

### Table of Contents
- Functions
   - Is Non-blocking
   - Get Connect Timeout
   - Set Connect Timeout

## Functions
### Set Multicast
```cpp
/// Enable/disable multicast for a socket. This options is only valid for
/// socket descriptors of type CSimpleSocket::SocketTypeUdp.
/// @return true if multicast was set or false if socket type is not
/// CSimpleSocket::SocketTypeUdp and the error will be set to
/// CSimpleSocket::SocketProtocolError
bool SetMulticast( bool bEnable, uint8_t multicastTTL = 1 );
```

### Get Multicast
```cpp
/// Return true if socket is multicast or false is socket is unicast
/// @return true if multicast is enabled
bool GetMulticast() const;
```

### Is Non-blocking
```cpp
/// Returns blocking/non-blocking state of socket.
/// @return true if the socket is non-blocking, else return false.
bool IsNonblocking() const;
```

### Set blocking
```cpp
/// Set the socket to blocking.
/// @return true if successful set to blocking, else return false;
bool SetBlocking();
```

### set Non-blocking
```cpp
/// Set the socket as non-blocking.
/// @return true if successful set to non-blocking, else return false;
bool SetNonblocking();
```

### Get Connect Timeout
###### Seconds
```cpp
/// Gets the timeout value that specifies the maximum number of seconds a
/// call to CSimpleSocket::Open waits until it completes.
/// @return the length of time in seconds
int32 GetConnectTimeoutSec() const;
```

###### Microseconds
```cpp
/// Gets the timeout value that specifies the maximum number of microseconds
/// a call to CSimpleSocket::Open waits until it completes.
/// @return the length of time in microseconds
int32 GetConnectTimeoutUSec() const;
```

### Set Connect Timeout
```cpp
/// Sets the timeout value that specifies the maximum amount of time a call
/// to CSimpleSocket::Receive waits until it completes. Use the method
/// CSimpleSocket::SetReceiveTimeout to specify the number of seconds to wait.
/// If a call to CSimpleSocket::Receive has blocked for the specified length of
/// time without receiving additional data, it returns with a partial count
/// or CSimpleSocket::GetSocketError set to CSimpleSocket::SocketEwouldblock if no data
/// were received.
/// @param nConnectTimeoutSec of timeout in seconds.
/// @param nConnectTimeoutUsec of timeout in microseconds.
/// @return true if socket connection timeout was successfully set.
void SetConnectTimeout(int32 nConnectTimeoutSec, int32 nConnectTimeoutUsec);
```

### Get Receive Timeout
###### Seconds
```cpp
/// Gets the timeout value that specifies the maximum number of seconds a
/// a call to CSimpleSocket::Receive waits until it completes.
/// @return the length of time in seconds
int32_t GetReceiveTimeoutSec() const;
```

###### Microseconds
```cpp
/// Gets the timeout value that specifies the maximum number of microseconds
/// a call to CSimpleSocket::Receive waits until it completes.
/// @return the length of time in microseconds
int32_t GetReceiveTimeoutUSec() const
```

### Set Receive Timeout
```cpp
/// Sets the timeout value that specifies the maximum amount of time a call
/// to CSimpleSocket::Receive waits until it completes. Use the method
/// CSimpleSocket::SetReceiveTimeout to specify the number of seconds to wait.
/// If a call to CSimpleSocket::Receive has blocked for the specified length of
/// time without receiving additional data, it returns with a partial count
/// or CSimpleSocket::GetSocketError set to CSimpleSocket::SocketEwouldblock if no data
/// were received.
///  @param nRecvTimeoutSec of timeout in seconds.
///  @param nRecvTimeoutUsec of timeout in microseconds.
///  @return true if socket timeout was successfully set.
bool SetReceiveTimeout
```

### Get Send Timeout
###### Seconds
```cpp
/// Gets the timeout value that specifies the maximum number of seconds a
/// a call to CSimpleSocket::Send waits until it completes.
/// @return the length of time in seconds
int32_t GetSendTimeoutSec() const { return m_stSendTimeout.tv_sec; }
```

###### Microseconds
```cpp
/// Gets the timeout value that specifies the maximum number of microseconds
/// a call to CSimpleSocket::Send waits until it completes.
/// @return the length of time in microseconds
int32_t GetSendTimeoutUSec() const { return m_stSendTimeout.tv_usec; }
```

### Set Send Timeout
```cpp
/// Gets the timeout value that specifies the maximum amount of time a call
/// to CSimpleSocket::Send waits until it completes.
/// @return the length of time in seconds
bool SetSendTimeout( int32_t nSendTimeoutSec, int32_t nSendTimeoutUsec = 0 );
```
