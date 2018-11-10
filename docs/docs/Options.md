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


### Is Non-blocking
```cpp
/// Returns blocking/non-blocking state of socket.
/// @return true if the socket is non-blocking, else return false.
bool IsNonblocking() const;
```

### Get Connect Timeout
###### Sec
```cpp
/// Gets the timeout value that specifies the maximum number of seconds a
/// call to CSimpleSocket::Open waits until it completes.
/// @return the length of time in seconds
int32 GetConnectTimeoutSec() const;
```

###### Nanoseconds
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
