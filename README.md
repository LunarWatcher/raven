# raven

A bare minimum C++23 HTTP socket library. Largely meant for use with [magpie](https://github.com/LunarWatcher/magpie), since asio is a leaky, useless pile of shit.

## Core principle: HTTP only

At least on Linux, there's many different kinds of sockets all bundled into the same few socket functions. This means that the core code for interacting with a socket can be completely identical, but how libraries have to be set up to deal with the different kinds of sockets approprately varies a great deal.

The average HTTP server won't need to worry about persistent HTTP sockets, and a connection that survives for long. The average websocket server, however, does. Socket persistence changes the optimisations that can be done, and the tradeoffs that are acceptable. This is also before accounting for socket types.

Asio tries to be everything at once by handling, as far as I can tell, all the different kinds of sockets, and it does a piss-poor job at it all, both on performance and on DevEx.

## Requirements

* A C++23 compiler
* Linux
    * Windows support may be added in the future once I figure out if I hate myself enough to do it
* OpenSSL for encryption once implemented


