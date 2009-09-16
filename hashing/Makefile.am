# ==================================================================
# © Copyright IBM Corp. 2007
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
#
# You can obtain a current copy of the Eclipse Public License from
# http://www.opensource.org/licenses/eclipse-1.0.php
#
# Author:	Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
#
# ==================================================================

# providerdir defines where provider libraries will be installed 
# (@PROVIDERDIR@ is set by the configure script). 
providerdir = @PROVIDERDIR@

###############################################################################
AM_CFLAGS = -g \
    -Wall \
    -Werror

AM_CPPFLAGS = \
    -D_GNU_SOURCE \
	-DDEBUG \
    -DSBLIM_DEBUG\
    -D_COMPILE_UNIX \
    -DSYSCONFDIR='"$(sysconfdir)"'


################################################################################
lib_LTLIBRARIES = \
    libuniquekey.la 

libuniquekey_la_LDFLAGS = \
    -avoid-version

libuniquekey_la_SOURCES = \
    libuniquekey.c

noinst_HEADERS = \
    libuniquekey.h
