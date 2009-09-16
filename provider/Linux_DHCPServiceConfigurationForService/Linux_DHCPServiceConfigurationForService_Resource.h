/// ============================================================================
/// Copyright © 2007, International Business Machines
///
/// THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
/// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
/// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
///
/// You can obtain a current copy of the Eclipse Public License from
/// http://www.opensource.org/licenses/eclipse-1.0.php
///
/// Authors:             Ashoka Rao S <ashoka.rao (at) in.ibm.com>
///                      Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
/// ============================================================================

#include <stdbool.h>

/** Include the required CMPI data types. */
#include <cmpidt.h>

/** Include the DHCP API. */
#include "sblim-dhcp.h"
#include "ra-support.h"
#include "provider-support.h"

/// ----------------------------------------------------------------------------
/// Info for the class supported by the association provider
/// ----------------------------------------------------------------------------

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** Name of the left and right hand side classes of this association. */
#define _ASSOCCLASS "Linux_DHCPServiceConfigurationForService"
#define _LHSCLASSNAME "Linux_DHCPServiceConfiguration"
#define _RHSCLASSNAME "Linux_DHCPService"
#define _LHSPROPERTYNAME "Configuration"
#define _RHSPROPERTYNAME "Element"
#define _LHSKEYNAME "Name"
#define _RHSKEYNAME "Name"
///#define _RHSKEYNAME "CreationClassName,Name,SystemCreationClassName,SystemName"
///#define _RHSKEYNAME "InstanceId"

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** DEFINE A HANDLE TO REPRESENT THE 'LIST' OF ALL SYSTEM RESOURCES.
   THE MAIN PROVIDER CODE DOES NOT NEED TO KNOW THE PARTICULARS OF HOW THIS
   LIST IS IMPLEMENTED - IT MAY BE AN ARRAY, LINKED LIST, FILE, ETC.
   THIS HANDLE IS PASSED INTO THE APPROPRIATE RESOURCE ACCESS METHODS WHEN
   ACCESSING/ITERATING/ADDING/REMOVING RESOURCES INSTANCES. */
typedef struct
{
    LIST * list;
    LIST * current;
} _RESOURCES;

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** DEFINE A HANDLE TO BE USED FOR EACH INSTANCE OF A SYSTEM RESOURCE.
   THE MAIN PROVIDER CODE DOES NOT NEED TO KNOW THE PARTICULARS OF HOW EACH
   RESOURCE IS REPRESENTED, BUT TYPICALLY IT IS A STRUCTURE MIRRORING THE
   PROPERTIES IN THE CIM CLASS. THIS HANDLE IS PASSED BETWEEN THE RESOURCE
   ACCESS METHODS WHEN MANIPULATING SPECIFIC RESOURCE INSTANCES. */
typedef struct
{
	CMPIObjectPath * lhs;
	CMPIObjectPath * rhs;
} _RESOURCE;

/** NOTHING BELOW THIS LINE SHOULD NEED TO BE CHANGED. */

/// ----------------------------------------------------------------------------
/// Generic resource access methods for CMPI providers.
/// Return value:
///	-1 = Unsupported
///	 0 = Failed
///	 1 = OK
/// ----------------------------------------------------------------------------

/** Checks whether to object pathes are associated. */
bool Linux_DHCPServiceConfigurationForService_isAssociated (CMPIObjectPath*, CMPIObjectPath*);

/** Get a handle to the list of all system resources for this class. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_getResources( const CMPIBroker *, const CMPIContext*, const CMPIObjectPath*, _RESOURCES ** resources);

/** Free/deallocate/cleanup the resources list after use. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_freeResources( _RESOURCES * resources);

/** Iterator to get the next resource from the resources list. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_getNextResource( _RESOURCES * resources, _RESOURCE ** resource);

/** Get the specific resource that matches the CMPI object path. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_getResourceForObjectPath( const CMPIBroker *, const CMPIContext*, _RESOURCES * resources, _RESOURCE ** resource, const CMPIObjectPath * objectpath);

/** Free/deallocate/cleanup a resource after use. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_freeResource( _RESOURCE * resource);

/** Set the property values of a CMPI instance from a specific resource. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_setInstanceFromResource( _RESOURCE * resource, const CMPIInstance * instance, const CMPIBroker * broker);

/// THE FOLLOWING METHODS MAY/NOT BE SUPPORTED BY THE SYSTEM FOR THIS CLASS

/** Delete the specified resource from the system. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_deleteResource( _RESOURCES * resources, _RESOURCE * resource);

/** Create a new resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_createResourceFromInstance( _RESOURCES * resources, _RESOURCE ** resource, const CMPIInstance * instance, const CMPIBroker * broker);
