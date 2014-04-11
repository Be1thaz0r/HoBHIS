HoBHIS
======

ndnSIM implementation of the Hop-by-hop Interest shaping mechanism for Content-Centric Networking

More detailed information about the mechanism can be found here:
http://www-npa.lip6.fr/~rozhnova/papers/nomen2012.pdf

The implementation has been developed and tested for ns-3 version 3.14.1 (3.15).

How to install:

1. Install boost library:

wget http://downloads.sourceforge.net/project/boost/boost/1.53.0/boost_1_53_0.tar.bz2

tar jxf boost_1_53_0.tar.bz2

cd boost_1_53_0

./bootstrap.sh

sudo ./b2 --prefix=/usr/local install

2. Clone the repositories:

git clone git://github.com/cawka/ns-3-dev-ndnSIM.git ns-3

git clone git@github.com:Be1thaz0r/HoBHIS.git ns-3/src/ndnSIM

3. Compile the code:

cd ns-3/

git checkout ns-3.17-ndnSIM-0.5

./waf configure --boost-includes=/usr/local/include --boost-libs=/usr/local/lib --enable-examples --disable-python

./waf

4. Run tests. For instance:

./waf --run hobhis-baseline-const-rtt

You can find more examples and results in ndnSIM/examples folder.

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

