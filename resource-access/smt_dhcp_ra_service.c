///===================================================================
/// smt_dhcp_ra_service.c
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
///====================================================================


#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sblim/smt_libra_conf.h>
#include <sblim/smt_libra_rastr.h>
#include <sblim/smt_libra_execscripts.h>

#include "sblim-dhcp.h"
#include "smt_dhcp_ra_service.h"
#include "smt_dhcp_ra_scripts.h"

static bool get_provider_conf(struct conf** provider_conf);

/** Method invoked to start the service */
int start_service() {
	struct conf* provider_conf = NULL;

	if ( !get_provider_conf(&provider_conf) ) {
		return -ENOENT;
	}
	
	char *daemon = get_conf(provider_conf,DEFAULT_INITSCRIPT);
	if (!daemon) {
		daemon = strdup(DEFAULT_INITSCRIPT);
	}

	return execScript1(daemon,"start");
}

/** Method to stop the service */
int stop_service() {
	struct conf* provider_conf = NULL;

	if ( !get_provider_conf(&provider_conf) ) {
		return -ENOENT;
	}
	
	char *daemon = get_conf(provider_conf,DEFAULT_INITSCRIPT);
	if (!daemon) {
		daemon = strdup(DEFAULT_INITSCRIPT);
	}

	return execScript1(daemon,"stop");
}

/** Mehthod to get the status of the service */
int status_service() {
	struct conf* provider_conf = NULL;
	char* filename = "smt_dhcp_ra_status.sh";
	char* ptr = NULL;

	if ( !get_provider_conf(&provider_conf) ) {
		return -ENOENT;
	}
	
	char* scriptdir = get_conf(provider_conf, SCRIPTDIR);
	if (!scriptdir) {
		scriptdir = strdup(DEFAULT_SCRIPTDIR);
	}

	char* script = (char *)malloc(strlen(scriptdir)+strlen(filename)+2);
	if (!script) return -ENOMEM;
	
	strncpy(script,scriptdir,strlen(scriptdir)+1);
	free(scriptdir);

	ptr = strchr(script,'\n');
	if (ptr)
		*ptr = '/';
	else
		strcat(script,"/");
	strcat(script,filename);

	return execScript(script);
}

///
/// Function to get the provider configuration file
/// 
/// @param a structure which includes the data from the provider configuration file
/// 
/// @return <code>true</code> if the function completed successful, otherwise 
/// <code>false</code>. 
///
static bool get_provider_conf(struct conf** provider_conf) {
	if ( !(*provider_conf) ) {
		/// No alternative given here, the PROVIDER_CONFFILE is the 
		/// smt_dhcp_ra_support.conf file which we installed. Its location 
		/// is fixed
		*provider_conf = read_conf(PROVIDER_CONFFILE, PROVIDER_CONFFILE);
	}
	
	if ( !(*provider_conf) ) 
		return false;
	
	return true;
}
