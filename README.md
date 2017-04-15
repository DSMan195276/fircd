[![Stories in Ready](https://badge.waffle.io/dsman195276/fircd.png?label=ready&title=Ready)](https://waffle.io/dsman195276/fircd)
fircd
=====

[![Build Status](https://travis-ci.org/DSMan195276/fircd.png?branch=master)](https://travis-ci.org/DSMan195276/fircd) [![Stories in Ready](https://badge.waffle.io/dsman195276/fircd.png?label=ready&title=Ready)](https://waffle.io/dsman195276/fircd)

Configurable File-system based IRC Client daemon

What is fircd?
==============

Fircd is an IRC Client that runs as a daemon, exposing your IRC connections
using a file-based API. Interaction with the daemon happens mainly through
sending text to named-pipes that fircd is in control of.

For example, an IRC session may precede something like this:

```
/tmp/irc <-- fircd's configured root
  |
  | - in (named-pipe) <-- This pipe allows you to send commands directly to
  |                       fircd (Ex. For connecting or disconnecting to
  |                       networks.)
  |
  | - fn (directory) <-- A named-network configured in fircd's configuration
  | |                    file
  | |
  | | - in (named-pipe) <-- named pipe for interacting with the network (Ex.
  | |                       joining/parting channels)
  | |
  | | - #fircd (directory) <-- directory representing a channel on this network
  | | |
  | | | - in (named-pipe) <-- Writing text to this pipe writes it as a message
  | | |                       to this channel
  | | |
  | | | - out (file) <-- Text contained inside is a human-readable log of
  | | |                  everything that has happened in the channel
  | | |
  | | | - msgs (file) <-- Like 'out', but only messages sent
  | | |
  | | | - online (file) <-- contains a list of the users current on this channel
  | | |
  | | | - topic (file) <-- contains the current topic
  | | | ...
  | |
  | | - #archlinux (directory) <-- Another channel
  | | | ...
  | |
  | | ...
  |
  | - irc.gamesurge.net (directory) <-- A network connected to after fircd started (Not named)
  | | ...
```

Depending on settings, directories can be removed as you leave
channels/networks, or they can be kept there along with their files (And will
then be added to when you join that channel again). In that way, fircd can also
be used for simple structured IRC logging. Also possible is writing simple IRC
bots which interact with fircd (And this can be done easily in any language
which can read/write files). There is an intent that various clients can be
written to make-use of fircd's file-systsem (Ex. Ncurses client, GTK client,
etc...), however the file-system API needs to stablize more before that's a
viable option.

Compiling
=========

fircd uses a simple Makefile setup for compiling. A compatable C compiler
capable of compiling C99 must be installed (Ex. gcc or clang). fircd's source
makes heavy use of POSIX to do everything related to networking and the
file-system, and thus you must have a POSIX compliant machine to compile and
run fircd.

A simple compile and install can be done like this:
```
make
make install
```

That will install both fircd and associated documentation (man files). If you
need more customized compilation options, see ./config.mk .

fircd is GPLv2 licensed.
