---
title: CSimpleSocket
layout: Single
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
- Receives ///< Shutdown passive socket.
- Sends    ///< Shutdown active socket.
- Both     ///< Shutdown both active and passive sockets.
