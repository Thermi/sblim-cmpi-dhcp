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
# Author:       Wolfgang Taphorn <taphorn (at) de.ibm.com>
# Description:  Automake input file for cmpi provider and resource
#               access layer.
# ==================================================================

#Include local M4 macros
ACLOCAL_AMFLAGS = -I m4

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

# Scripts that are used in the libraries
pkgdata_SCRIPTS =

# System wide libraries
lib_LTLIBRARIES =

# providerdir defines where provider libraries will be installed (@PROVIDERDIR@ is set by the configure script). 
providerdir = @PROVIDERDIR@

# Start from an empty list for files to clean up
CLEANFILES = include/sblim-dhcp.h

# Start from an empty extra distribution file list
EXTRA_DIST = sblim-cmpi-dhcp.spec

# Pre Build requirements need to be compiled prior anything else
BUILT_SOURCES = include/sblim-dhcp.h

# Build subdirectories
if TESTSUITE_SUPPORT
  TESTCASE_DIR = test
endif
SUBDIRS = $(TESTCASE_DIR) po man hashing resource-access provider 


######################################################################################
# Automake instructions for documentation
doc_DATA = README AUTHORS COPYING

######################################################################################
# We must explicity add the RPM spec file to the distribution package
#EXTRA_DIST += $(PACKAGE).spec

######################################################################################
# We must explicity take care of the common header file to search/replace everything 
# correctly
include/sblim-dhcp.h: include/sblim-dhcp.h.pre
	sed -e 's,[@]SCRIPTDIR[@],$(pkgdatadir),g' -e s?$$\{prefix\}?$(prefix)?g -e s?$$\{exec_prefix\}?$(prefix)?g $< > $@
#	test -d "/etc/ld.so.conf.d" && cp include/wbem-smt.conf /etc/ld.so.conf.d/

noinst_HEADERS += include/sblim-dhcp.h

######################################################################################
# Generate the *.gmo's during make
all-local:
	$(MAKE) update-gmo -C po

#######################################################################################
# Automake instructions for ./model subdir

######################################################################################
SCHEMAS = \
    model/Linux_DHCPService.mof \
    model/Linux_DHCPServiceConfiguration.mof \
    model/Linux_DHCPEntity.mof \
    model/Linux_DHCPGroup.mof \
    model/Linux_DHCPPool.mof \
    model/Linux_DHCPHost.mof \
    model/Linux_DHCPSubnet.mof \
    model/Linux_DHCPSharednet.mof \
    model/Linux_DHCPGlobal.mof \
    model/Linux_DHCPOptions.mof \
    model/Linux_DHCPParams.mof \
    model/Linux_DHCPGroupsForEntity.mof \
    model/Linux_DHCPHostsForEntity.mof \
    model/Linux_DHCPPoolsForEntity.mof \
    model/Linux_DHCPSubnetsForEntity.mof \
    model/Linux_DHCPSharednetsForEntity.mof \
    model/Linux_DHCPOptionsForEntity.mof \
    model/Linux_DHCPParamsForEntity.mof \
    model/Linux_DHCPGlobalForService.mof \
    model/Linux_DHCPRegisteredProfile.mof \
    model/Linux_DHCPElementConformsToProfile.mof \
    model/Linux_DHCPServiceConfigurationForService.mof

######################################################################################
REGISTRATIONS = \
    model/registration/Linux_DHCPService.registration \
    model/registration/Linux_DHCPServiceConfiguration.registration \
    model/registration/Linux_DHCPGroup.registration \
    model/registration/Linux_DHCPPool.registration \
    model/registration/Linux_DHCPHost.registration \
    model/registration/Linux_DHCPSubnet.registration \
    model/registration/Linux_DHCPSharednet.registration \
    model/registration/Linux_DHCPGlobal.registration \
    model/registration/Linux_DHCPOptions.registration \
    model/registration/Linux_DHCPParams.registration \
    model/registration/Linux_DHCPGroupsForEntity.registration \
    model/registration/Linux_DHCPHostsForEntity.registration \
    model/registration/Linux_DHCPPoolsForEntity.registration \
    model/registration/Linux_DHCPSubnetsForEntity.registration \
    model/registration/Linux_DHCPSharednetsForEntity.registration \
    model/registration/Linux_DHCPOptionsForEntity.registration \
    model/registration/Linux_DHCPParamsForEntity.registration \
    model/registration/Linux_DHCPGlobalForService.registration \
    model/registration/Linux_DHCPServiceConfigurationForService.registration


######################################################################################
INTEROP_SCHEMAS = \
	model/Linux_DHCPService.mof \
    model/Linux_DHCPRegisteredProfile.mof \
    model/Linux_DHCPElementConformsToProfile.mof

######################################################################################
INTEROP_REGISTRATIONS = \
    model/registration/Linux_DHCPRegisteredProfile.registration \
    model/registration/Linux_DHCPElementConformsToProfile.registration

######################################################################################

# We must explicity add all the schema files to the distribution package
pkgdata_DATA = $(SCHEMAS) $(REGISTRATIONS) $(INTEROP_SCHEMAS) $(INTEROP_REGISTRATIONS)

pkgdata_SCRIPTS += $(srcdir)/provider-register.sh

EXTRA_DIST += $(pkgdata_SCRIPTS) $(pkgdata_DATA)

# Register the provider(s) and class definition(s) to the current CIM server/CIMOM
postinstall:
	sh provider-register.sh -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)
	sh provider-register.sh -t @CIMSERVER@ -n "root/PG_InterOp" -r $(INTEROP_REGISTRATIONS) -m $(INTEROP_SCHEMAS)

preuninstall:
	sh provider-register.sh -d -t @CIMSERVER@ -r $(REGISTRATIONS) -m $(SCHEMAS)
	sh provider-register.sh -d -t @CIMSERVER@ -n "root/PG_InterOp" -r $(INTEROP_REGISTRATIONS) -m $(INTEROP_SCHEMAS)

dist-hook:
	test -d "$(distdir)" &&	rm -rf `find $(distdir) -type d -name CVS`

