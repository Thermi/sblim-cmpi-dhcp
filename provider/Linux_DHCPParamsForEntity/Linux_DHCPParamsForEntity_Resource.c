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

#include "Linux_DHCPParamsForEntity_Resource.h"

#include <string.h>
#include <stdlib.h>

/** Include the required CMPI data types, function headers, and macros. */
#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

/** Include the DHCP API. */
#include "sblim-dhcp.h"


int Linux_DHCPParamsForEntity_isAssociated ( CMPIObjectPath * lhs, CMPIObjectPath * rhs)
{
    CMPIStatus cmpi_status = {CMPI_RC_OK, NULL};
    char *lparent, *rparent, *lchild , *rchild;
    const char* cmpi_name = NULL;

    CMPIData cmpiInfo = CMGetKey(lhs, "InstanceID", &cmpi_status);
    if ((cmpi_status.rc != CMPI_RC_OK) || CMIsNullObject(lhs)) {
	return 0;
    }

    cmpi_name = CMGetCharsPtr(cmpiInfo.value.string, NULL);
    lchild = ra_tokenize((char*) cmpi_name, 1);
    lparent = ra_tokenize((char *)cmpi_name, 2);
    
    cmpiInfo = CMGetKey(rhs, "InstanceID", &cmpi_status);
    if ((cmpi_status.rc != CMPI_RC_OK) || CMIsNullObject(rhs)) {
	return 0;
    }

    cmpi_name = CMGetCharsPtr(cmpiInfo.value.string, NULL);
    rchild = ra_tokenize((char*) cmpi_name, 1);
    rparent = ra_tokenize((char *)cmpi_name, 2);

    if(strcasecmp(lparent, rchild) == 0 ) {
        free(lparent);free(lchild);
	free(rparent);free(rchild);
	return 1;
    }
    else if (strcasecmp(rparent, lchild) == 0) {
        free(lparent);free(lchild);
	free(rparent);free(rchild);
	return 2;
    }
    else {
        free(lparent);free(lchild);
	free(rparent);free(rchild);
	return 0;
    }
}

/// ----------------------------------------------------------------------------
/** Get a handle to the list of all system resources for this class. */
_RA_STATUS Linux_DHCPParamsForEntity_getResources( const CMPIBroker * broker, const CMPIContext * context, const CMPIObjectPath * reference, _RESOURCES ** resources)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus status = {CMPI_RC_OK, NULL};
    LIST * current;
    int i;
    _RESOURCE * resource;

    (*resources) = (_RESOURCES *)malloc(sizeof(_RESOURCES));
    memset((*resources), '\0', sizeof(_RESOURCES));
    if ( (*resources) == NULL ) {
                setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                return ra_status;
    }

    (*resources)->list = (LIST *)malloc(sizeof(LIST));
    memset((*resources)->list, '\0', sizeof(LIST));
    if ( (*resources)->list == NULL ) {
                setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                return ra_status;
    }

    current = (*resources)->list;

    const char * namespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL);
    CMPIObjectPath * objectpath = CMNewObjectPath(broker, namespace, _LHSCLASSNAME, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath)) {
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
	return ra_status;
    }

    CMPIEnumeration * lhs_ops = CBEnumInstanceNames(broker, context, objectpath, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(lhs_ops)) {
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
	return ra_status;
    }

    objectpath = CMNewObjectPath(broker, namespace, _RHSCLASSNAME, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath)) {
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
	return ra_status;
    }

    CMPIEnumeration * rhs_ops = CBEnumInstanceNames(broker, context, objectpath, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(rhs_ops)) {
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
	return ra_status;
    }

    CMPIArray * rhs_ops_array = CMToArray(rhs_ops, NULL);
    CMPICount rhs_length = CMGetArrayCount(rhs_ops_array, NULL);

    while (CMHasNext(lhs_ops, NULL)) {
	CMPIData lhs_data = CMGetNext(lhs_ops, NULL);
	CMPIObjectPath * lhs_op = lhs_data.value.ref;
	int state = 0;

	for(i = 0; i < rhs_length; i++)
	{
	    CMPIData rhs_data = CMGetArrayElementAt(rhs_ops_array, i, NULL);
	    CMPIObjectPath * rhs_op = rhs_data.value.ref;

	    if (lhs_op && rhs_op && (state = Linux_DHCPParamsForEntity_isAssociated(lhs_op, rhs_op))) {
		resource = (_RESOURCE *)malloc(sizeof(_RESOURCE));
		memset(resource, '\0', sizeof(_RESOURCE));
                    if ( resources == NULL ) {
                        setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                        return ra_status;
                    }

		switch(state){
		    case 1:
			resource->lhs = lhs_op;
			resource->rhs = rhs_op;
			break;
		    case 2:
			resource->lhs = rhs_op;
			resource->rhs = lhs_op;
			break;
		}
		current->content = (_RESOURCE *)resource;
		current->next = (LIST *)malloc(sizeof(LIST));
		memset(current->next, '\0', sizeof(LIST));
                if ( current->next == NULL ) {
                     setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                     return ra_status;
                }

		current = current->next;
		current->content = current->next = NULL;
	    } 
	}
    }

    (*resources)->current = (*resources)->list;
    return ra_status;
}

/// ----------------------------------------------------------------------------
/** Iterator to get the next resource from the resources list. */
_RA_STATUS Linux_DHCPParamsForEntity_getNextResource( _RESOURCES * resources, _RESOURCE ** resource)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    LIST * list = resources->current;
    if(list->next) {
	(*resource) = (_RESOURCE *)list->content;
	resources->current = list->next;
    }
    else {
	(*resource) = NULL;
    }

    return ra_status;
}



/// ----------------------------------------------------------------------------

/** Free/deallocate/cleanup the resources list after use. */
_RA_STATUS Linux_DHCPParamsForEntity_freeResources( _RESOURCES * resources)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
   if (resources) {
	if (resources->list) { 
		ra_deleteList(resources->list);
		resources->list = NULL;
	}
        free(resources);
	resources = NULL;
    }
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Get the specific resource that matches the CMPI object path. */
_RA_STATUS Linux_DHCPParamsForEntity_getResourceForObjectPath( const CMPIBroker * broker, const  CMPIContext * ctx, _RESOURCES * resources, _RESOURCE ** resource, const CMPIObjectPath * objectpath)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus cmpi_status = {CMPI_RC_OK, NULL};
    CMPIData cmpiInfo;
    LIST * itrl = NULL;
    char * src, * dest;

    if(CMIsNullObject(objectpath)){
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
	return ra_status;
    }

    cmpiInfo = CMGetKey(objectpath, _LHSPROPERTYNAME, &cmpi_status);
    if(cmpi_status.rc != CMPI_RC_OK || CMIsNullValue(cmpiInfo)){
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );	
	return ra_status;
    }

    cmpiInfo = CMGetKey(cmpiInfo.value.ref, "InstanceID", &cmpi_status); 
    src = ra_tokenize((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL), 1);
    for(itrl = resources->list; itrl->next != NULL; itrl = itrl->next){
	cmpiInfo = CMGetKey( ((_RESOURCE*)itrl->content)->lhs, "InstanceID", &cmpi_status);
	if (cmpi_status.rc != CMPI_RC_OK) {
            setRaStatus( &ra_status, RA_RC_FAILED, INVALID_INSTANCE_ID, _("Invalid instance ID or InstanceID not found") );
	    return ra_status;
	}

	dest = ra_tokenize((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL), 1);
 
	if(strcasecmp(src, dest) == 0) {
	    (*resource) = (_RESOURCE*)itrl->content;
	    ra_status.rc = RA_RC_OK;
	    return ra_status;
	}
	else {
	    ra_status.rc = RA_RC_FAILED;
	    (*resource) = NULL;
	}
    }
    return ra_status;
}

/// ----------------------------------------------------------------------------
/** Free/deallocate/cleanup the resource after use. */
_RA_STATUS Linux_DHCPParamsForEntity_freeResource( _RESOURCE * resource)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}

/// ---------------------------------------------------------------------------- 

/** Set the property values of a CMPI instance from a specific resource. */
_RA_STATUS Linux_DHCPParamsForEntity_setInstanceFromResource( _RESOURCE * resource, const CMPIInstance * instance, const CMPIBroker * broker)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    if(!resource)
	return ra_status;


    CMSetProperty(instance, _LHSPROPERTYNAME, (CMPIValue*) &resource->lhs, CMPI_ref);
    CMSetProperty(instance, _RHSPROPERTYNAME, (CMPIValue*) &resource->rhs, CMPI_ref);
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Delete the specified resource from the system. */
_RA_STATUS Linux_DHCPParamsForEntity_deleteResource( _RESOURCES * resources, _RESOURCE * resource)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Create a new resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPParamsForEntity_createResourceFromInstance( _RESOURCES * resources, _RESOURCE ** resource, const CMPIInstance * instance, const CMPIBroker * broker)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}
