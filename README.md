HoBHIS
======

ndnSIM implementation of the Hop-by-hop Interest shaping mechanism for Content-Centric Networking

More detailed information about the mechanism can be found here:
http://www-npa.lip6.fr/~rozhnova/papers/nomen2012.pdf

The implementation has been developed and tested for ns-3 version 3.14.1 (3.15).

How to install:

You have to get and install an ns-3 version first. Then, go to the ns-3 folder

git clone git@github.com:Be1thaz0r/HoBHIS.git ns-3/src/ndnSIM

This release includes:

1) HoBHIS
	1.1) mono-conversation
	1.2) multi-conversation
	1.3) random generation of the response delay by server

2) Interest rate control (Tolerance mechanism)
	2.1) mono-conversation
	2.2) multi-conversation
	2.3) client reaction on control packets (at this moment it is supported by consumer-cbr only)

3) Optional:
	3.1) Dynamic adjustment of the design parameter (at this moment it is supported for mono-conversation scenario)

NB standard ns3 tracing is not tested yet

Please, do not hesitate to contact me in case of any problem/question or some suggestions: Natalya Rozhnova <natalya.rozhnova (at) lip6.fr>

You can find more examples and results in ndnSIM/examples folder.
