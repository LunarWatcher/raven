# raven

A bare minimum C++23 socket library. Largely meant for use with [magpie](https://github.com/LunarWatcher/magpie), since asio is a leaky, useless pile of shit.

This library is specifically focused on sockets for web use, i.e. TCP and UDP sockets, and not any other sockets. This may be added in the future if it makes sense to do so, but the primary goal is to provide support for a web framework that's in desperate need of a socket framework that isn't horseshit.

## Requirements

* A C++23 compiler
* (In the future, maybe): OpenSSL for encryption
