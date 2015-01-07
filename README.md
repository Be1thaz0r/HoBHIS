HoBHIS
======

ndnSIM implementation of the Hop-by-hop Interest shaping mechanism for Content-Centric Networking

More detailed information about the mechanism can be found here:
http://www-npa.lip6.fr/~rozhnova/papers/nomen2012.pdf

The implementation has been developed and tested for ns-3 version 3.14.1 (3.15).

How to install:

0) Prerequisites install the appropriate packages. Example debian:
	
	aptitude install g++ libboost-dev-all ...

1) Clone the repositories:

	mkdir HoBHIS
	cd HoBHIS
	git clone git@github.com:Be1thaz0r/HoBHIS.git .

2) Compile the code:

	cd HoBHIS/ns-3/
	./waf configure --enable-examples --disable-python
	./waf

If the compilation fails due to warnings, please remove the option [-Werror] from waf-tools/cflags.py

If the module ndnSIM is not built check for the paths to the boost libraries.
This issue typically occurs on Debian 64 since waf is unable to locate them.
A workaround consists in running instead in specifying the boost include and boost lib directories.

	./waf configure --boost-includes=/path/to/boost/includes --boost-lib=/path/to/boost/libs --enable-examples --disable-python
	./waf

Example: under Debian x64:

	./waf configure --boost-lib=/usr/lib/x86_64-linux-gnu --enable-examples --disable-python
	./waf

3) Run tests. For instance:

	./waf --run hobhis-chain

See ndnSIM/examples/HowTo.txt for more details.
You can find more examples and results in ndnSIM/examples folder.

This release includes:

1) HoBHIS

	1.1) mono-conversation

	1.2) multi-conversation

	1.3) random generation of the response delay by server

NB standard ns3 tracing is not tested yet

Please, do not hesitate to contact me in case of any problem/question or some suggestions: Natalya Rozhnova: natalya.rozhnova (at) lip6.fr
