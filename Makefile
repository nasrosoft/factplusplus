#
# Makefile for FaCT++ project
#

# -- DO NOT CHANGE THE REST OF FILE --
SUBDIRS = Bdd Kernel FaCT++ FaCT++.JNI

include Makefile.include

# Additional targets to build parts of the FaCT++

bdd:
	make -C Bdd

kernel: bdd
	make -C Kernel

#digparser: kernel
#	make -C DIGParser

fpp_lisp: kernel
	make -C FaCT++

fpp_jni: kernel
	make -C FaCT++.JNI

#fpp_dig: digparser
#	make -C FaCT++.DIG

#fpp_server: digparser
#	make -C FaCT++.Server

