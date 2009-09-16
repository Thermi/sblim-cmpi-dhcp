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
# Authors:      Ashoka Rao.S <ashoka.rao@in.in.ibm.com>
#		        Riyashmon Haneefa <riyashh1@in.ibm.com>
# Description:  Automake input file for resource access layer.
# ==================================================================

AM_CFLAGS = -Wall 
#AM_CFLAGS = -Wall -g -DDEBUG -DTRACE

LEX = flex
LFLAGS = -i

YACC = bison
YFLAGS = -d --verbose

CC = cc
#CFLAGS = -g -DDEBUG -DTRACE


# SBLIM Include Directory
sblimincdir = $(includedir)/sblim

# SBLIM Headers to be installed
sbliminc_HEADERS =

# docdir defines where the documentation goes
docdir = $(datadir)/doc/$(PACKAGE)-$(VERSION)

# Non-Installable Header Files
noinst_HEADERS =

# Configuration files
sysconf_DATA =

# Start from an empty extra distribution file list
EXTRA_DIST =

# Start from an empty list of programs not to install
noinst_PROGRAMS =

# System wide libraries
lib_LTLIBRARIES = 

######################################################################################
# Automake instructions for 'resource-access' dir

smt_dhcp_ra_support.conf: smt_dhcp_ra_support.conf.pre
	sed -e 's,[@]SCRIPTDIR[@],$(pkgdatadir),g' -e s?$$\{prefix\}?$(prefix)?g -e s?$$\{exec_prefix\}?$(prefix)?g $< > $@

# Adding new generated files to file list for clean up
CLEANFILES = \
	dhcpd.conf.parser.tab.c \
	dhcpd.conf.parser.tab.h \
	lex.conf.c \
	smt_dhcp_ra_support.conf \
	smt_dhcp_ra_support.conf.pre \
	dhcpd.conf.parser.output

# Files for /etc directory
sysconf_DATA += smt_dhcp_ra_support.conf

pkgdata_SCRIPTS = \
    smt_dhcp_ra_status.sh 

pkgdata_DATA = 

# LibRaTools libraries
lib_LTLIBRARIES += \
	 libRaToolsDhcp.la 

# LibRaTools sources
libRaToolsDhcp_la_SOURCES = \
    provider-support.c  \
    ra-support.c \
    smt_dhcp_ra_service.c
    
nodist_libRaToolsDhcp_la_SOURCES = \
    dhcpd.conf.parser.tab.c \
    dhcpd.conf.parser.tab.h \
    lex.conf.c

libRaToolsDhcp_la_CPPFLAGS = \
    -fPIC \
    -DPROVIDER_CONFFILE=\"$(sysconfdir)/smt_dhcp_ra_support.conf\" \
    -D_GNU_SOURCE \
    -I${srcdir}/../include/ \
    -I${srcdir}/../hashing/

libRaToolsDhcp_la_CFLAGS = \
    -DTRACE -g -DDEBUG -Wall

libRaToolsDhcp_la_LIBADD = \
    ${top_builddir}/hashing/libuniquekey.la

libRaToolsDhcp_la_LDFLAGS = \
    -fPIC -lpthread 

# Installable Header Files
noinst_HEADERS += \
    provider-support.h \
    smt_dhcp_ra_service.h \
    smt_dhcp_ra_scripts.h \
    ra-support.h

# Parser binaries for testing
noinst_PROGRAMS += confTest #confParseTest zoneParseTest confLexTest zoneLexTest #apiTest

confTest_SOURCES = \
        conftest.c 

confTest_CPPFLAGS = \
        -D_GNU_SOURCE \
        -DDEBUG_YACC \
        -I${srcdir}/../include/ \
	-I${srcdir}/../hashing/
       

confTest_CFLAGS = \
        -DTRACE -g -DDEBUG -DDEBUG_YACCC -Wall

confTest_LDADD  = \
        libRaToolsDhcp.la


# We must explicitly add the list of scripts to the 
EXTRA_DIST += $(pkgdata_DATA) $(pkgdata_SCRIPTS) dhcpd.conf.lexer.l dhcpd.conf.parser.y #zone.lexer.l zone.parser.y


lex.conf.c: dhcpd.conf.lexer.l
	$(LEX) $(LFLAGS) $<

dhcpd.conf.parser.tab.c: dhcpd.conf.parser.y
	$(YACC) $(YFLAGS) $<

