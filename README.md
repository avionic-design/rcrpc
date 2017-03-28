# About the remote-control RPC extension #

This repository contains _rcrpc_, the _remote-control_ RPC extension. It adds an
RPC server to _remote-control_, allowing the hardware it runs on to be
controlled remotely. It is this extension which makes the name of
_remote-control_ really meaningful.

Keep in mind this extension exposes _remote-control_ to the network without any
form of security measures. This keeps things simple and is useful enough for
closed environments, but NEVER use this extensions in networks you do not
completely control!

The RPC extension uses _librpc_ to start an RPC server and is, apart from
initialisation, completely self-sufficient. The RPC API is a bit outdated by now
as support for older hardware was removed from or has never been implemented in
_remote-control_, but most of it works as advertised. The library itself is
loaded by _remote-control_ at runtime. The _remote-control-client_ binary can be
used to test and verify the library when _remote-control_ is running.

The RPC API has originally been integrated in _remote-control_ itself but was
split out as an extension some time after version 1.0.2.

# Building #

This project is based on Autotools, so building is mostly executing
`autogen.sh` with the desired configuration options. If you do not build the
extension from within _remote-control_ itself, _pkg-config_ is used to detect
_libremote-control_ and the development headers. If you do build from within
_remote-control_ but still want external versions, pass the
_--enable-standalone_ flag to configure.

Apart from the usual dependencies like Autotools and things building things,
internally developed libraries are available from our [public FTP][adftp]:

* [librpc](http://ftp.avionic-design.de/pub/librpc/)
* [remote-control](http://ftp.avionic-design.de/pub/remote-control/)

# Development #

Issues may be reported via [GitHub][bugs-github]. Pull requests are welcome,
bear in mind though we might have different plans or use cases.

  [adftp]: http://ftp.avionic-design.de/pub/ "Avionic Design public FTP server"
  [pbs2]: https://github.com/avionic-design/pbs-stage2 "Platform Build System"
  [pbsad]: https://github.com/avionic-design/pbs-platform-avionic-design
    "AD platform for PBS"
  [bugs-github]: https://github.com/avionic-design/rcrpc/issues
