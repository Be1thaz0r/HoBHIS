HoBHIS
======

ndnSIM implementation of the Hop-by-hop Interest shaping mechanism for Content-Centric Networking

More detailed information about the mechanism can be found here:
http://www-npa.lip6.fr/~rozhnova/papers/nomen2012.pdf

The implementation has been developed and tested for ns-3 version 3.14.1 (3.15).

How to install:

1) Clone the repositories:

	1.1) mkdir HoBHIS
	1.2) cd HoBHIS
	1.3) git clone git@github.com:Be1thaz0r/HoBHIS.git .

2) Compile the code:

	2.1) cd HoBHIS/ns-3/

	2.2) ./waf configure --enable-examples --disable-python

	2.3) ./waf

3) Run tests. For instance:

	3.1) ./waf --run hobhis-chain

See ndnSIM/examples/HowTo.txt for more details.
You can find more examples and results in ndnSIM/examples folder.

This release includes:

1) HoBHIS

	1.1) mono-conversation

	1.2) multi-conversation

	1.3) random generation of the response delay by server

NB standard ns3 tracing is not tested yet

Please, do not hesitate to contact me in case of any problem/question or some suggestions: Natalya Rozhnova: natalya.rozhnova (at) lip6.fr
