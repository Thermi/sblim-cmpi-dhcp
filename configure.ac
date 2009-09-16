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
# ==================================================================
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(SBLIM CMPI Provider for WBEM-SMT DHCP task, 1.0, sblim-devel@lists.sourceforge.net, sblim-cmpi-dhcp)
AC_CONFIG_SRCDIR([.])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([tar-pax])

AC_CANONICAL_HOST
case $host_cpu in
     i*86) HW=INTEL;; 
     s390*) HW=S390;; 
     ppc*) HW=PPC;; 
     x86_64) HW=X86_64;; 
     ia64) HW=IA64;; 
     **) HW=GENERIC;;
esac

AC_ARG_VAR([TESTSUITEDIR],[the directory where the SBLIM testsuite resides.])
AC_ARG_VAR([PROVIDERDIR],[the directory where the CMPI providers will be installed.])
AC_ARG_VAR([CIMSERVER],[the target CIM server (pegasus|sfcb|openwbem).])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

# gettext
AM_ICONV
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.14])

# Check for installed CIM Server (defined in local m4 macro)
CHECK_CIMSERVER

# Check for required CIM Server provider directory (defined in local m4 macro)
CHECK_CIMSERVER_PROVIDERDIR

# Check for required CMPI header files (defined in local m4 macro)
CHECK_CMPI_HEADER

# Check for SBLIM TestSuite support (defined in local m4 macro)
CHECK_SBLIM_TESTSUITE

# Check for required WBEM-SMT Resource Access header files (defined in local m4 macro)
CHECK_SBLIM_LIBRA_HEADER

# Check for required libraries
AC_CHECK_LIB(RaTools, main, [], [AC_MSG_ERROR(You have to install libRaTools first)])

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h string.h errno.h stdbool.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STAT
AC_CHECK_FUNCS([memmove select strchr strdup])

AM_CONDITIONAL([TESTSUITE_SUPPORT], test "$TESTSUITEDIR")

AC_CONFIG_FILES(Makefile sblim-cmpi-dhcp.spec)
AC_CONFIG_FILES(po/Makefile.in)
AC_CONFIG_FILES(man/Makefile man/smt_dhcp_ra_support.conf.5.pre)
AC_CONFIG_FILES(include/sblim-dhcp.h.pre)
AC_CONFIG_FILES(hashing/Makefile)    
AC_CONFIG_FILES(resource-access/Makefile resource-access/smt_dhcp_ra_support.conf.pre)
AC_CONFIG_FILES(provider/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPRegisteredProfile/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPService/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPGroup/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPPool/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPSubnet/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPSharednet/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPHost/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPGlobal/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPOptions/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPParams/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPGroupsForEntity/Makefile)    
AC_CONFIG_FILES(provider/Linux_DHCPHostsForEntity/Makefile)       
AC_CONFIG_FILES(provider/Linux_DHCPPoolsForEntity/Makefile)       
AC_CONFIG_FILES(provider/Linux_DHCPSubnetsForEntity/Makefile)       
AC_CONFIG_FILES(provider/Linux_DHCPSharednetsForEntity/Makefile)       
AC_CONFIG_FILES(provider/Linux_DHCPOptionsForEntity/Makefile)       
AC_CONFIG_FILES(provider/Linux_DHCPParamsForEntity/Makefile)       
AC_CONFIG_FILES(provider/Linux_DHCPGlobalForService/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPServiceConfiguration/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPServiceConfigurationForService/Makefile)
AC_CONFIG_FILES(provider/Linux_DHCPElementConformsToProfile/Makefile)
AC_CONFIG_FILES(test/Makefile)
AC_OUTPUT
echo

# Display configuration options
echo "-------------------------------------------------------"
echo "Configuration for $PACKAGE complete."
echo ""
echo "The following configuration options have been selected:"
echo "CIMSERVER: " $CIMSERVER
echo "PROVIDERDIR: " $PROVIDERDIR
echo "CPPFLAGS:" $CPPFLAGS
echo "LDFLAGS:" $LDFLAGS
echo
if test $TESTSUITEDIR; then
  echo "Test Suite Support: enabled"
  echo "TESTSUITEDIR = $TESTSUITEDIR"
else
  echo "Test Suite Support: disabled"
fi
echo "-------------------------------------------------------"
echo
echo "You may now run make"
echo
