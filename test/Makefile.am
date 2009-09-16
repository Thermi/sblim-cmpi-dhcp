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
# Contributors : Ashoka Rao S <ashoka.rao (at) in.ibm.com>
#                Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
# ==================================================================


# docdir defines where the documentation goes
docdir = $(datadir)/doc/$(PACKAGE)-$(VERSION)


######################################################################################
# Automake instructions for ./test subdir
EXTRA_DIST = README.test $(testsuite_SCRIPTS) $(testsuiteconf_DATA) system cim dhcpd.conf

doc_DATA = README.test

testsuitedir=@TESTSUITEDIR@
testsuitesystemdir=@TESTSUITEDIR@/system/linux
testsuitecimdir=@TESTSUITEDIR@/cim
testsuitexmldir=@TESTSUITEDIR@/xml
testsuiteconfdir=@TESTSUITEDIR@/

testsuite_SCRIPTS=test-cmpi-dhcp.sh

testsuitecim_DATA=\
	cim/Linux_DHCPElementConformsToProfile.cim \
    cim/Linux_DHCPRegisteredProfile.cim \
    cim/Linux_DHCPGroup.cim \
    cim/Linux_DHCPHost.cim \
    cim/Linux_DHCPPool.cim \
    cim/Linux_DHCPSubnet.cim \
    cim/Linux_DHCPSharednet.cim \
    cim/Linux_DHCPGlobal.cim \
    cim/Linux_DHCPOptions.cim \
    cim/Linux_DHCPParams.cim \
    cim/Linux_DHCPService.cim \
    cim/Linux_DHCPServiceConfiguration.cim \
    cim/Linux_DHCPGlobalForService.cim \
    cim/Linux_DHCPGroupsForEntity.cim \
    cim/Linux_DHCPHostsForEntity.cim \
    cim/Linux_DHCPOptionsForEntity.cim \
    cim/Linux_DHCPParamsForEntity.cim \
    cim/Linux_DHCPPoolsForEntity.cim \
    cim/Linux_DHCPServiceConfigurationForService.cim \
    cim/Linux_DHCPSharednetsForEntity.cim \
    cim/Linux_DHCPSubnetsForEntity.cim

testsuitesystem_DATA=\
	system/linux/Linux_DHCPElementConformsToProfile.system \
    system/linux/Linux_DHCPGlobal.system \
    system/linux/Linux_DHCPGlobalForService.system \
    system/linux/Linux_DHCPGroup.system \
    system/linux/Linux_DHCPGroupsForEntity.system \
    system/linux/Linux_DHCPHost.system \
    system/linux/Linux_DHCPHostsForEntity.system \
    system/linux/Linux_DHCPOptions.system \
    system/linux/Linux_DHCPOptionsForEntity.system \
    system/linux/Linux_DHCPParams.system \
    system/linux/Linux_DHCPParamsForEntity.system \
    system/linux/Linux_DHCPPool.system \
    system/linux/Linux_DHCPPoolsForEntity.system \
    system/linux/Linux_DHCPRegisteredProfile.system \
    system/linux/Linux_DHCPService.system \
    system/linux/Linux_DHCPServiceConfiguration.system \
    system/linux/Linux_DHCPServiceConfigurationForService.system \
    system/linux/Linux_DHCPSharednet.system \
    system/linux/Linux_DHCPSharednetsForEntity.system \
    system/linux/Linux_DHCPSubnetsForEntity.system \
    system/linux/Linux_DHCPSubnet.system

testsuiteconf_DATA=\
    dhcpd.conf 
