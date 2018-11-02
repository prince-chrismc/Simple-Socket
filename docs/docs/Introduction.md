---
id: Intro
title: Introduction
sidebar_label: Introduction
---

### Table of Contents
- [History](#history)
- [Class Overview](#class-overview)

## History
Written by Mark Carrier to provide a mechanism for writing cross platform socket code. This library was originally written to only support blocking
TCP sockets. Over the years it has been extended to support UDP and RAW sockets as well with many contribution from the Dwarf Fortress community.
This library supports:
* Cross platform abstraction
   * Windows
   * Linux
   * Mac OSX
* Multiple IO modes
   * sychronious
   * asychronious
* Supports TCP Streams, UDP Datagrams, Raw Sockets
* Thread Safe and Signal Safe

The library's original release notes can be found [here](https://github.com/DFHack/clsocket/blob/master/ReleaseNotes) for more details.

## Class Overview
Network communications via sockets can be abstracted into two categories of functionality; the active socket and the passive socket.
The active socket object initiates a connection with a known host, whereas the passive socket object waits (or listens) for inbound requests for
communication. The functionality of both objects is identical as far as sending and receiving data. This library makes distinction between the two
objects because the operations for constructing and destructing the two are different.

This library is different from other socket libraries which define TCP sockets, UDP sockets, HTTP sockets, etc.
The reason is the operations required for TCP, UDP, and RAW network communication is identical from a logical stand point.
Thus a program could initially be written employing TCP streams, and then at some future point it could be discovered that UDP datagrams would
satisify the solution. Changing between the two transport protocols would only require changing how the object is instantiated. The remaining code
would in theory require minimal to no changes.

This library avoids abstractions like HTTP socket, or SMTP socket, soley because this type of object mixes the application and the transport layer.
These types of abstractions can be created using this library as a base class.

The simple socket library is comprised of two class which can be used to represent all socket communications.
* [Active Socket Class](Getting-Started#client-app)
* [Passive Socket Class](Getting-Started#server-app)
