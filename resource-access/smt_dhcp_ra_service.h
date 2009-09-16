///======================================================================
/// smt_dhcp_ra_service.h
///
/// © Copyright IBM Corp. 2007
///
/// THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
/// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
/// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
///
/// You can obtain a current copy of the Eclipse Public License from
/// http://www.opensource.org/licenses/eclipse-1.0.php
///
/// Authors : Ashoka Rao.S <ashoka.rao (at) in.ibm.com>
///           Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
///
///=======================================================================

#ifndef _SMT_DHCP_RA_SERVICE_H_
#define _SMT_DHCP_RA_SERVICE_H_

#include <sblim/smt_libra_execscripts.h>

#ifdef __cplusplus
extern "C" {
#endif

int start_service();
int stop_service();
int status_service();

#ifdef __cplusplus
}
#endif

#endif


