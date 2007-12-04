/*
 * conftest.c
 *
 * © Copyright IBM Corp. 2007
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Common Public License from
 * http://www.opensource.org/licenses/cpl1.0.php
 *
 * Authors : Ashoka Rao.S <ashoka.rao (at) in.ibm.com>
 *           Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
 *
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

/* Includes */
#include "sblim-dhcp.h"
#include "smt_libra_conf.h"
#include "smt_libra_rastr.h"
//#include "smt_libra_execscripts.h"

#include "smt_dhcp_ra_service.h"
#include "smt_dhcp_ra_scripts.h"
#include "ra-support.h"
#include "provider-support.h"

const char * config_file = "./dhcpd.conf";


/* Externs */
extern NODE * parseConfigFile (FILE *, FILE *);

//extern static char *init_script();
extern int start_service();
extern int stop_service();
extern int status_service();
extern char *my_script_path(char *);

//const char * config_file = "./dhcpd.conf";


int main(){
   return 0;
}
