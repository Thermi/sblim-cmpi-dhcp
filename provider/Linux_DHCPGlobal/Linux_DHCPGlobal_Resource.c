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
/// Contributors:
/// ============================================================================

#include "Linux_DHCPGlobal_Resource.h"
#include "sblim-dhcp.h"

#include <string.h>
#include <stdlib.h>

/** Include the required CMPI data types, function headers, and macros. */
#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

/// ----------------------------------------------------------------------------

/** Set supported methods accordingly */
bool Global_isEnumerateInstanceNamesSupported() { return true; };
bool Global_isEnumerateInstancesSupported()     { return true; };
bool Global_isGetSupported()                    { return true; };
bool Global_isCreateSupported()                 { return false; };
bool Global_isModifySupported()                 { return false; };
bool Global_isDeleteSupported()                 { return false; };

/// ----------------------------------------------------------------------------

/** Get a handle to the list of all system resources for this class. */
_RA_STATUS Linux_DHCPGlobal_getResources( _RESOURCES** resources  ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    (*resources) = (_RESOURCES *)malloc(sizeof(_RESOURCES));
    memset((*resources), '\0', sizeof(_RESOURCES));

    if( (*resources) == NULL) {
        setRaStatus( &ra_status, RA_RC_FAILED, DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
        return ra_status;
    }

    (*resources)->Array = ra_getAllEntity(GLOBALF, NULL, &ra_status);

    if( (*resources)->Array == NULL) {
        setRaStatus( &ra_status, RA_RC_FAILED, ENTITY_NOT_FOUND, _("Entity Not Found") );
        return ra_status;
    }

    (*resources)->current = 0;
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Iterator to get the next resource from the resources list. */
_RA_STATUS Linux_DHCPGlobal_getNextResource( _RESOURCES* resources, _RESOURCE** resource ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    _RESOURCE * temp;
    
    if(resources->Array[resources->current] != NULL)
    {
	temp = (_RESOURCE *)malloc(sizeof(_RESOURCE));
	memset(temp, '\0', sizeof(_RESOURCE));

	if( temp == NULL) {
                setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                return ra_status;
        }

	temp->Entity = resources->Array[resources->current++];
	temp->InstanceID = ra_instanceId(temp->Entity, _CLASSNAME);

	(*resource) = temp;
    }else{

	(*resource) = NULL;
    }
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Get the specific resource that matches the CMPI object path. */
_RA_STATUS Linux_DHCPGlobal_getResourceForObjectPath( _RESOURCES* resources, _RESOURCE** resource, const CMPIObjectPath* objectpath ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus cmpi_status = {CMPI_RC_OK, NULL};
    CMPIData cmpi_info;
    NODE ** itr;
    int key = 0;
    int index = 0;
    if(CMIsNullObject(objectpath))
    {
	 setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
	return ra_status;
    }

    cmpi_info = CMGetKey(objectpath, "InstanceID", &cmpi_status);
    if((cmpi_status.rc != CMPI_RC_OK) || CMIsNullValue(cmpi_info)){
	setRaStatus( &ra_status, RA_RC_FAILED, FAILED_TO_FETCH_KEY_ELEMENT_DATA, _("Failed to fetch the key element data") );
	return ra_status;
    }

    const char* cmpi_name = CMGetCharsPtr(cmpi_info.value.string, NULL);
    key = ra_getKeyFromInstance((char*) cmpi_name);

    if(cmpi_name == NULL){
	setRaStatus( &ra_status, RA_RC_FAILED,  CMPI_INSTANCE_NAME_IS_NULL, _("Cmpi instance name is NULL") );
	return ra_status;
    }
    
    for(itr = resources->Array, index = 0; *itr != NULL; index++, itr++){
	if(key == (*itr)->obID){
	    (*resource) = (_RESOURCE *)malloc(sizeof(_RESOURCE));
	    memset((*resource), '\0', sizeof(_RESOURCE));

            if( (*resource) == NULL) {
                 setRaStatus( &ra_status, RA_RC_FAILED, DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                 return ra_status;
            }

	    (*resource)->Entity = resources->Array[index];
	    (*resource)->InstanceID = ra_instanceId(resources->Array[index], _CLASSNAME);
	}
    }
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Free/deallocate/cleanup the resource after use. */
_RA_STATUS Linux_DHCPGlobal_freeResource( _RESOURCE* resource ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    if(resource) {
	if(resource->InstanceID) {
		free(resource->InstanceID);
                resource->InstanceID = NULL;
        }
	free(resource);
        resource = NULL;
    }
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Free/deallocate/cleanup the resources list after use. */
_RA_STATUS Linux_DHCPGlobal_freeResources( _RESOURCES* resources ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    if(resources) {
	    if(resources->Array) {
		free( resources->Array);
                resources->Array = NULL;
            }
	free( resources);
        resources = NULL;
     }
    return ra_status;
}

/// ---------------------------------------------------------------------------- 

/** Set the property values of a CMPI instance from a specific resource. */
_RA_STATUS Linux_DHCPGlobal_setInstanceFromResource( _RESOURCE* resource, const CMPIInstance* instance, const CMPIBroker* broker ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

    CMSetProperty(instance, "InstanceID", (CMPIValue *)resource->InstanceID, CMPI_chars);
    CMSetProperty(instance, "ElementName", (CMPIValue *)"Global", CMPI_chars);
    
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Delete the specified resource from the system. */
_RA_STATUS Linux_DHCPGlobal_deleteResource( _RESOURCES* resources, _RESOURCE* resource, const CMPIBroker* broker ) {

    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Modify the specified resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPGlobal_setResourceFromInstance( _RESOURCE** resource, const CMPIInstance* instance, const char** properties, const CMPIBroker* broker ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Create a new resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPGlobal_createResourceFromInstance( _RESOURCES* resources, _RESOURCE** resource, const CMPIInstance* instance, const CMPIBroker* broker ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus cmpi_status = { CMPI_RC_OK, NULL };
    CMPIData cmpi_info;
    NODE *pnode = NULL, *newnode = NULL;
    int level = 0;
    unsigned long long pid;
    if (CMIsNullObject(instance)) {
	setRaStatus(&ra_status, RA_RC_FAILED, INSTANCE_ID_IS_NULL,
		    _("Instance is NULL"));
	return ra_status;
    }

    cmpi_info = CMGetProperty(instance, "ParentID", &cmpi_status);
    if ((cmpi_status.rc != CMPI_RC_OK) || CMIsNullValue(cmpi_info)) {
	setRaStatus(&ra_status, RA_RC_FAILED,
		    PARENTID_NOT_SPECIFIED_OR_NOT_PROPER,
		    _("ParentID not specified properly or not provided"));
	return ra_status;
    }

    const char *cmpi_name = CMGetCharsPtr(cmpi_info.value.string, NULL);
    level = ra_findLevel(cmpi_name);
    pid = ra_getKeyFromInstance((char *) cmpi_name);
    pnode = ra_getEntity(pid, NULL, &ra_status);

    if (pnode == NULL) {
	setRaStatus(&ra_status, RA_RC_FAILED, ENTITY_NOT_FOUND,
		    _("Entity Not Found"));
	return ra_status;
    }

    if (newnode == NULL) {
	setRaStatus(&ra_status, RA_RC_FAILED, FAILED_CREATING_A_NODE,
		    _("Failed creating a Node"));
	return ra_status;
    }

    ra_updateDhcpdFile();

    (*resource) = (_RESOURCE *) malloc(sizeof(_RESOURCE));
    memset((*resource), '\0', sizeof(_RESOURCE));

    if ((*resource) == NULL) {
	setRaStatus(&ra_status, RA_RC_FAILED,
		    DYNAMIC_MEMORY_ALLOCATION_FAILED,
		    _("Dynamic Memory Allocation Failed"));
	return ra_status;
    }

    (*resource)->Entity = newnode;
    (*resource)->InstanceID = ra_instanceId(newnode, _CLASSNAME);


    return ra_status;
}

_RA_STATUS Linux_DHCPGlobal_InstanceProviderInitialize(_RA_STATUS* ra_status){
    ra_Initialize( ra_status );
    return (*ra_status);
}

_RA_STATUS Linux_DHCPGlobal_InstanceProviderCleanUp(bool term){
    _RA_STATUS ra_status ={RA_RC_OK, 0, NULL};
    if(term) ra_CleanUp();
    return ra_status;
}
