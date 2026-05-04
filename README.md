# raven

A bare minimum C++23 HTTP socket library. Largely meant for use with [magpie][magpie], since asio is a leaky, useless pile of shit.

## Core principle: HTTP only

At least on Linux, there's many different kinds of sockets all bundled into the same few socket functions. This means that the core code for interacting with a socket can be completely identical, but how libraries have to be set up to deal with the different kinds of sockets approprately varies a great deal.

The average HTTP server won't need to worry about persistent HTTP sockets, and a connection that survives for long. The average websocket server, however, does. Socket persistence changes the optimisations that can be done, and the tradeoffs that are acceptable. This is also before accounting for socket types.

Asio tries to be everything at once by handling, as far as I can tell, all the different kinds of sockets, and it does a piss-poor job at it all, both on performance and on DevEx.

## Requirements

* A C++23 compiler
* Linux
    * Windows support may be added in the future once I figure out if I hate myself enough to do it
* OpenSSL for encryption once implemented

## Performance

Single-threaded load on an 8-threaded server (16-threaded host machine), with everything built in debug mode yields ~5240 req/sec (6618 req/sec in release mode). A singular benchmark exists to provide stability testing against single-threaded workloads, as it forces the server into various multithreaded workload distributions that are well-suited for uncovering bugs. More in-depth testing using multiple threads may be added in the future for the same reason.

No further performance testing will be performed by me. Stability testing is my primary concern, as this library primarily lays the foundation for [magpie][magpie], and asio does not make for a solid foundation.

The library is event-driven, and largely designed for use with [nghttp2][nghttp2] or similarly styled libraries, where one event-driven system provides data to a second event-driven system. Further async-support, particularly coroutines, is out of scope for this library, but can in theory be supported by libraries using raven. This is largely because C++'s coroutines are abysmal, and without support for async IO in consumer libraries (read: in file reading, SQL libraries, etc.), there's no point in implementing it on the server itself, since the average server won't be able to take advantage of coroutines.

Throwing more threads at the problem is also a fine solution. The threads used internally by raven are created once, and context switching has become comparatively cheap on modern operating systems. This only works as long as the workload is IO-bound and not compute-bound.

[magpie]: https://codeberg.org/LunarWatcher/magpie
[nghttp2]: https://github.com/nghttp2/nghttp2
