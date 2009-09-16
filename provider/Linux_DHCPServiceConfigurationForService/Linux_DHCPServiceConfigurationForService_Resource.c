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

#include "Linux_DHCPServiceConfigurationForService_Resource.h"

#include <string.h>
#include <stdlib.h>

/** Include the required CMPI data types, function headers, and macros. */
#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

/** Include the DHCP API. */
#include "sblim-dhcp.h"


bool Linux_DHCPServiceConfigurationForService_isAssociated ( CMPIObjectPath * lhs, CMPIObjectPath * rhs)
{
  
    CMPIStatus cmpi_status = {CMPI_RC_OK, NULL};
    const char * cmpi_name1 = NULL, * cmpi_name2 = NULL;
    
    if (dhcp_conf_tree == NULL)
    ///	printf("dhcp_conf_tree = NULL\n");
       return false;

    cmpi_name1 =  CMGetCharsPtr(CMGetClassName(lhs, &cmpi_status), NULL);
    cmpi_name2 =  CMGetCharsPtr(CMGetClassName(rhs, &cmpi_status), NULL);

     if((strcasecmp((char*) cmpi_name1, "Linux_DHCPServiceConfiguration") == 0) && (strcasecmp((char*) cmpi_name2, "Linux_DHCPService") == 0)){
        return true;
    }
    else {
        return false;
    }
}

/// ----------------------------------------------------------------------------
/** Get a handle to the list of all system resources for this class. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_getResources( const CMPIBroker * broker, const CMPIContext * context, const CMPIObjectPath * reference, _RESOURCES ** resources)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus status = {CMPI_RC_OK, NULL};
    LIST * current;
    int i;
    _RESOURCE * resource;


    (*resources) = (_RESOURCES *)malloc(sizeof(_RESOURCES));
    //memset((*resources), '\0', sizeof(_RESOURCES));
    if ( (*resources) == NULL ) {
                setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                return ra_status;
    }

    (*resources)->list = (LIST *)malloc(sizeof(LIST));
    //memset((*resources), '\0', sizeof(LIST));
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

        
	for(i = 0; i < rhs_length; i++)
	{   
            CMPIData rhs_data = CMGetArrayElementAt(rhs_ops_array, i, NULL);
	    CMPIObjectPath * rhs_op = rhs_data.value.ref;
	    if (lhs_op && rhs_op && Linux_DHCPServiceConfigurationForService_isAssociated(lhs_op, rhs_op)) {
		
                resource = (_RESOURCE *)malloc(sizeof(_RESOURCE));
		//memset(resource, '\0', sizeof(_RESOURCE));
                    if ( resource == NULL ) {
                        setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                        return ra_status;
                    }

		resource->lhs = lhs_op;
		resource->rhs = rhs_op;
		current->content = (_RESOURCE *)resource;
		current->next = (LIST *)malloc(sizeof(LIST));
		//memset(current->next, '\0', sizeof(LIST));
                if ( current->next == NULL ) {
                     setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                     return ra_status;
                }

		current = current->next;
		current->content = current->next = NULL;
		break;
	    } 
	}
    }
    

    (*resources)->current = (*resources)->list;
   
    return ra_status;
}

/// ----------------------------------------------------------------------------
/** Iterator to get the next resource from the resources list. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_getNextResource( _RESOURCES * resources, _RESOURCE ** resource)
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


_RA_STATUS Linux_DHCPServiceConfigurationForService_getResourceForObjectPath( const CMPIBroker * broker, const  CMPIContext * ctx, _RESOURCES * resources, _RESOURCE ** resource, const CMPIObjectPath * objectpath)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus cmpi_status = {CMPI_RC_OK, NULL};
    CMPIData cmpiInfo1, cmpiInfo2;
    CMPIObjectPath * src, *dest;
    CMPIInstance * lhs_inst, *rhs_inst;
     
     if(CMIsNullObject(objectpath)){
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
         return ra_status;
      }
    
    *resource = (_RESOURCE*) malloc(sizeof(_RESOURCE));
    //memset((*resource), '\0', sizeof(_RESOURCE));
    if ( resource == NULL ) {
         setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
         return ra_status;
    }

   cmpiInfo1 = CMGetKey(objectpath, _RHSPROPERTYNAME, &cmpi_status);
    if(cmpi_status.rc != CMPI_RC_OK || CMIsNullValue(cmpiInfo1)){
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
        return ra_status;
    }

   cmpiInfo2 = CMGetKey(objectpath, _LHSPROPERTYNAME, &cmpi_status);
    if(cmpi_status.rc != CMPI_RC_OK || CMIsNullValue(cmpiInfo2)){
        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
        return ra_status;
    }

    src = cmpiInfo1.value.ref;
    dest = cmpiInfo2.value.ref;

        lhs_inst = CBGetInstance(broker, ctx, dest, NULL, &cmpi_status);
        if ((cmpi_status.rc != CMPI_RC_OK) || CMIsNullObject(lhs_inst))  { 
	        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
		 return ra_status;
	}
         
        rhs_inst = CBGetInstance(broker, ctx, src, NULL, &cmpi_status);
        if ((cmpi_status.rc != CMPI_RC_OK) || CMIsNullObject(rhs_inst)) { 
	        setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
		return ra_status;
	}

        dest = CMGetObjectPath(lhs_inst, NULL);
        src = CMGetObjectPath(rhs_inst, NULL);

        if (Linux_DHCPServiceConfigurationForService_isAssociated(dest, src))
        {
                _RESOURCE * this = (_RESOURCE*) malloc(sizeof(_RESOURCE));
		//memset(this, '\0', sizeof(_RESOURCE));
		    if ( this == NULL ) {
		         setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
		         return ra_status;
		    }

                this->lhs = dest;
                this->rhs = src;

                *resource = this;
                return ra_status;
        }

        else {
		ra_status.rc = RA_RC_FAILED;
                return ra_status;
        }
}
              

/// ----------------------------------------------------------------------------

/** Free/deallocate/cleanup the resources list after use. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_freeResources( _RESOURCES * resources)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    if(resources) {
	if(resources->list) {
	    ra_deleteList(resources->list);
	    resources->list = NULL;
	}
  	free(resources);
	resources = NULL;
    }
    return ra_status;
}

/// ----------------------------------------------------------------------------
/** Free/deallocate/cleanup the resource after use. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_freeResource( _RESOURCE * resource)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}

/// ---------------------------------------------------------------------------- 

/** Set the property values of a CMPI instance from a specific resource. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_setInstanceFromResource( _RESOURCE * resource, const CMPIInstance * instance, const CMPIBroker * broker)
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
_RA_STATUS Linux_DHCPServiceConfigurationForService_deleteResource( _RESOURCES * resources, _RESOURCE * resource)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}

/// ----------------------------------------------------------------------------

/** Create a new resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPServiceConfigurationForService_createResourceFromInstance( _RESOURCES * resources, _RESOURCE ** resource, const CMPIInstance * instance, const CMPIBroker * broker)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    return ra_status;
}
