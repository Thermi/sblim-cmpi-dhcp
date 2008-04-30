/// ============================================================================
/// Copyright © 2007, International Business Machines
///
/// THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
/// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
/// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
///
/// You can obtain a current copy of the Common Public License from
/// http://oss.software.ibm.com/developerworks/opensource/license-cpl.html
///
/// Authors:             Ashoka Rao S <ashoka.rao (at) in.ibm.com>
///                      Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
/// ============================================================================

#include <stdbool.h>

/** Include the DHCP API. */
#include "sblim-dhcp.h"
#include "ra-support.h"
#include "provider-support.h"

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** Name of the class implemented by this instance provider. */
#define _CLASSNAME "Linux_DHCPOptions"

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** DEFINE A HANDLE TO REPRESENT THE 'LIST' OF ALL SYSTEM RESOURCES.
   THE MAIN PROVIDER CODE DOES NOT NEED TO KNOW THE PARTICULARS OF HOW THIS
   LIST IS IMPLEMENTED - IT MAY BE AN ARRAY, LINKED LIST, FILE, ETC.
   THIS HANDLE IS PASSED INTO THE APPROPRIATE RESOURCE ACCESS METHODS WHEN
   ACCESSING/ITERATING/ADDING/REMOVING RESOURCES INSTANCES. */
typedef struct {
    int current ;
    NODE ** Array;
} _RESOURCES;

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** DEFINE A HANDLE TO BE USED FOR EACH INSTANCE OF A SYSTEM RESOURCE.
   THE MAIN PROVIDER CODE DOES NOT NEED TO KNOW THE PARTICULARS OF HOW EACH
   RESOURCE IS REPRESENTED, BUT TYPICALLY IT IS A STRUCTURE MIRRORING THE
   PROPERTIES IN THE CIM CLASS. THIS HANDLE IS PASSED BETWEEN THE RESOURCE
   ACCESS METHODS WHEN MANIPULATING SPECIFIC RESOURCE INSTANCES. */
typedef struct {
    NODE * Entity;
    char * InstanceID;
} _RESOURCE;

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** DEFINE A UNION FOR THE RESULT OF THE EXTRINSIC METHODS */
typedef union {
	unsigned int uint32;
} _METHOD_RETURN_VALUE;

/** NOTHING BELOW THIS LINE SHOULD NEED TO BE CHANGED. */

/** Include the required CMPI data types. */
#include <cmpidt.h>


bool Option_isEnumerateInstanceNamesSupported();
bool Option_isEnumerateInstancesSupported();
bool Option_isGetSupported();
bool Option_isCreateSupported( );
bool Option_isModifySupported( );
bool Option_isDeleteSupported( );

/** Get a handle to the list of all system resources for this class. */
_RA_STATUS Linux_DHCPOptions_getResources( _RESOURCES** resources );

/** Iterator to get the next resource from the resources list. */
_RA_STATUS Linux_DHCPOptions_getNextResource( _RESOURCES* resources, _RESOURCE** resource );

/** Get the specific resource that matches the CMPI object path. */
_RA_STATUS Linux_DHCPOptions_getResourceForObjectPath( _RESOURCES* resources, _RESOURCE** resource, const CMPIObjectPath* objectpath );

/** Free/deallocate/cleanup a resource after use. */
_RA_STATUS Linux_DHCPOptions_freeResource( _RESOURCE* resource );

/** Free/deallocate/cleanup the resources list after use. */
_RA_STATUS Linux_DHCPOptions_freeResources( _RESOURCES* resources );

/** Set the property values of a CMPI instance from a specific resource. */
_RA_STATUS Linux_DHCPOptions_setInstanceFromResource( _RESOURCE* resource, const CMPIInstance* instance, const CMPIBroker* broker );

/** Create a new resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPOptions_createResourceFromInstance( _RESOURCES* resources, _RESOURCE** resource, const CMPIInstance* instance, const CMPIBroker* broker );

/** Delete the specified resource from the system. */
_RA_STATUS Linux_DHCPOptions_deleteResource( _RESOURCES* resources, _RESOURCE* resource, const CMPIBroker* broker);

/** Modify the specified resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPOptions_setResourceFromInstance( _RESOURCE** resource, const CMPIInstance* instance, const char** properties, const CMPIBroker* broker );

/** Initialization method for Instance Provider */
_RA_STATUS Linux_DHCPOptions_InstanceProviderInitialize(_RA_STATUS* );

/** CleanUp method for Instance Provider */
_RA_STATUS Linux_DHCPOptions_InstanceProviderCleanUp(bool terminate);

///----------------------------------------------------------------------------
/** Build object path method for Instance Provider */
_RA_STATUS Linux_DHCPOptions_BuildObjectPath(CMPIObjectPath* , CMPIInstance* , char* , _RESOURCE* );

