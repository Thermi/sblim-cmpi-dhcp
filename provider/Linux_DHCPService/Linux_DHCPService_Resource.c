// ============================================================================
// Copyright © 2007, International Business Machines
//
// THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
//
// You can obtain a current copy of the Common Public License from
// http://oss.software.ibm.com/developerworks/opensource/license-cpl.html
//
// Authors:             Ashoka Rao S <ashoka.rao (at) in.ibm.com>
//                      Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
// ============================================================================

#include "Linux_DHCPService_Resource.h"

#include <string.h>
#include <stdlib.h>

/* Include the required CMPI data types, function headers, and macros. */
#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

// ----------------------------------------------------------------------------

/* Set supported methods accordingly */
bool isEnumerateInstanceNamesSupported() { return true; };
bool isEnumerateInstancesSupported()     { return true; };
bool isGetSupported()                    { return true; };
bool isCreateSupported()                 { return false; };
bool isModifySupported()                 { return false; };
bool isDeleteSupported()                 { return false; };

// ----------------------------------------------------------------------------

/* Get a handle to the list of all system resources for this class. */
_RA_STATUS Linux_DHCPService_getResources( _RESOURCES** resources  ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    
    (*resources) = (_RESOURCES *)malloc(sizeof(_RESOURCES));

    if( (*resources) == NULL) {
        setRaStatus( &ra_status, RA_RC_FAILED, DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
        return ra_status;
    }

    (*resources)->Array = ra_getAllEntity(SERVICEF, NULL, &ra_status);

    if( (*resources)->Array == NULL) {
        setRaStatus( &ra_status, RA_RC_FAILED, ENTITY_NOT_FOUND, _("Entity Not Found") );
        return ra_status;
    }

    (*resources)->current = 0;

    return ra_status;
}

// ----------------------------------------------------------------------------

/* Iterator to get the next resource from the resources list. */
_RA_STATUS Linux_DHCPService_getNextResource( _RESOURCES* resources, _RESOURCE** resource ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    
    _RESOURCE *temp;

    if(resources->Array[resources->current] != NULL) 
    {
	temp = (_RESOURCE*)malloc(sizeof(_RESOURCE));

        if( temp == NULL) {
                setRaStatus( &ra_status, RA_RC_FAILED, DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
                return ra_status;
        }

        temp->Entity = resources->Array[resources->current++];
        temp->InstanceID = ra_instanceId(temp->Entity, _CLASSNAME);
    
        (*resource) = temp;
     } else {
    
        (*resource) = NULL;
	//setRaStatus( &ra_status, RA_RC_FAILED,  DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Failed to get list of system resources") );
        //return ra_status;

    }

    return ra_status;
}

// ----------------------------------------------------------------------------

/* Get the specific resource that matches the CMPI object path. */
_RA_STATUS Linux_DHCPService_getResourceForObjectPath( _RESOURCES* resources, _RESOURCE** resource, const CMPIObjectPath* objectpath ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus cmpi_status = {CMPI_RC_OK, NULL};
    CMPIData cmpi_info;
    NODE** itr;
    const char* cmpi_name;
    int index = 0;


    if(CMIsNullObject(objectpath))  //Verify if the ObjectPath received is NULL
    {
	setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
 	return ra_status;
    }

    cmpi_info = CMGetKey(objectpath, "SystemName", &cmpi_status);
    if((cmpi_status.rc != CMPI_RC_OK) || CMIsNullValue(cmpi_info)){
	  setRaStatus( &ra_status, RA_RC_FAILED, FAILED_TO_FETCH_KEY_ELEMENT_DATA, _("Failed to fetch the key element data") );	
         return ra_status;
    }

    cmpi_name =  CMGetCharsPtr(cmpi_info.value.string, NULL);

    if(cmpi_name == NULL){  //No key value found
        setRaStatus( &ra_status, RA_RC_FAILED,  CMPI_INSTANCE_NAME_IS_NULL, _("Cmpi instance name is NULL") ); 
	return ra_status;
    }
    
    for(itr = resources->Array, index = 0; *itr != NULL; index++, itr++) {

	if( !strcmp((char*) cmpi_name, (*itr)->obValue)) {
                (*resource) = (_RESOURCE *)malloc(sizeof(_RESOURCE));

		 if( (*resource) == NULL) {
	                 setRaStatus( &ra_status, RA_RC_FAILED, DYNAMIC_MEMORY_ALLOCATION_FAILED, _("Dynamic Memory Allocation Failed") );
        	         return ra_status;
            	}

		(*resource)->Entity = resources->Array[index];
                //(*resource)->InstanceID = ra_instanceId(resources->Array[index], _CLASSNAME);
         }
    }

    return ra_status;
}

// ---------------------------------------------------------------------------- 

/* Get an object path from a plain CMPI instance. This has to include to create the key attributes properly.*/
_RA_STATUS Linux_DHCPService_getObjectPathForInstance( CMPIObjectPath **objectpath, const CMPIInstance *instance ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    
#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Getting object path from instance\n",__FUNCTION__);
#endif //DEBUG

    /*if (!instance || CMIsNullObject(instance)) {
        setRaStatus(&ra_status, RA_RC_FAILED, MSGID_INVALID_CMPI_INSTANCE, _("The submitted CMPI instance was not initialized properly."));
#ifdef DEBUG
        fprintf(stderr,"\t -%s -- Setting instance from resource - completed with error\n",__FUNCTION__);
#endif //DEBUG
        return ra_status;
    }*/

    //TODO - Include logic code here (if not required, remove this tag)
    
#ifdef DEBUG
        fprintf(stderr,"\t -%s -- Getting object path from instance - completed\n",__FUNCTION__);
#endif //DEBUG
    return ra_status;
}

// ---------------------------------------------------------------------------- 

/* Set the property values of a CMPI instance from a specific resource. */
_RA_STATUS Linux_DHCPService_setInstanceFromResource( _RESOURCE* resource, const CMPIInstance* instance, const CMPIBroker* broker ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    char* parentID;
    NODE * service_node = resource->Entity;    

    parentID = ra_instanceId(resource->Entity->parent, _CLASSNAME);

    CMSetProperty(instance, "SystemName", (CMPIValue *)service_node->obValue, CMPI_chars);
    CMSetProperty(instance, "Name", (CMPIValue *)service_node->obName, CMPI_chars);
    CMSetProperty(instance, "CreationClassName", (CMPIValue *)"Linux_DHCPService", CMPI_chars);
    CMSetProperty(instance, "SystemCreationClassName", (CMPIValue *)"Linux_DHCPService", CMPI_chars);
    CMSetProperty(instance, "ParentID", (CMPIValue *)parentID, CMPI_chars);
     
    return ra_status;
}

// ----------------------------------------------------------------------------

/* Free/deallocate/cleanup the resource after use. */
_RA_STATUS Linux_DHCPService_freeResource( _RESOURCE* resource ) {
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

// ----------------------------------------------------------------------------

/* Free/deallocate/cleanup the resources list after use. */
_RA_STATUS Linux_DHCPService_freeResources( _RESOURCES* resources ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
   if(resources) {
	if(resources->Array) {
		free(resources->Array);
		resources->Array = NULL;
	}
	free(resources);
	resources = NULL;
    }
    return ra_status;
}

// ----------------------------------------------------------------------------

/* Delete the specified resource from the system. */
_RA_STATUS Linux_DHCPService_deleteResource( _RESOURCES* resources, _RESOURCE* resource, const CMPIBroker* broker ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

   if(resource)
	ra_deleteNode(resource->Entity);
    return ra_status;
}

// ----------------------------------------------------------------------------

/* Modify the specified resource using the property values of a CMPI instance. */
_RA_STATUS Linux_DHCPService_setResourceFromInstance( _RESOURCE** resource, const CMPIInstance* instance, const char** properties, const CMPIBroker* broker ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Setting resource from CMPI instance\n",__FUNCTION__);
#endif //DEBUG

    //TODO - Include logic code here (if not required, remove this tag)

#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Setting resource from CMPI instance - completed\n",__FUNCTION__);
#endif //DEBUG
    return ra_status;
}

// ----------------------------------------------------------------------------

/* Create a new resource using the property values of a CMPI instance. */
/* Not supported for this. */
_RA_STATUS Linux_DHCPService_createResourceFromInstance( _RESOURCES* resources, _RESOURCE** resource, const CMPIInstance* instance, const CMPIBroker* broker ) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus cmpi_status = {CMPI_RC_OK, NULL};
    CMPIData cmpi_info;
    NODE * pnode, *newnode;
    const char* cmpi_name;
    int pid;

#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Creating resource from CMPI instance\n",__FUNCTION__);
#endif //DEBUG

    if(CMIsNullObject(instance)) {

	return ra_status;
    }
 
    cmpi_info = CMGetProperty(instance, "ParentID", &cmpi_status);
    if((cmpi_status.rc != CMPI_RC_OK) || CMIsNullValue(cmpi_info)) 
	return ra_status;
  
    cmpi_name =  CMGetCharsPtr(cmpi_info.value.string, NULL);
    pid = ra_getKeyFromInstance((char*) cmpi_name);
    pnode = ra_getEntity(pid, NULL, &ra_status);
    
    newnode = ra_createService( 1);
    ra_dropChild(pnode, newnode);
    //printf("Parent = %x, Child = %x\n", pnode->obID, newnode->obID);

    (*resource) = (_RESOURCE *)malloc(sizeof(_RESOURCE));
    (*resource)->Entity = newnode;
    (*resource)->InstanceID = ra_instanceId(newnode, _CLASSNAME);


#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Creating resource from CMPI instance - completed\n",__FUNCTION__);
#endif //DEBUG
    return ra_status;
}

// ----------------------------------------------------------------------------

/* Initialization method for Instance Provider */
_RA_STATUS Linux_DHCPService_InstanceProviderInitialize() {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Initialization of instance provider\n",__FUNCTION__);
#endif //DEBUG

    //TODO - Include logic code here (if not required, remove this tag)

#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Initialization of instance provider - completed\n",__FUNCTION__);
#endif //DEBUG
    return ra_status;
}

// ----------------------------------------------------------------------------

/* CleanUp method for Instance Provider */
_RA_STATUS Linux_DHCPService_InstanceProviderCleanUp(bool terminate) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Cleanup of instance provider\n",__FUNCTION__);
#endif //DEBUG

    //TODO - Include logic code here (if not required, remove this tag)

#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Cleanup of instance provider - completed\n",__FUNCTION__);
#endif //DEBUG
    return ra_status;
}

// ----------------------------------------------------------------------------
// Method Provider

/* Extrinsic Method - StartService */
_RA_STATUS Linux_DHCPService_Method_StartService( unsigned int* methodResult, _RESOURCE* resource) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    *methodResult = 0;


//#ifdef DEBUG
//    fprintf(stderr,"\t -%s -- Processing extrinsic method for start service\n",__FUNCTION__);
//#endif //DEBUG

  start_service();    
  //TODO - Include logic code here (if not required, remove this tag)
    
//#ifdef DEBUG
//    fprintf(stderr,"\t -%s -- Processing extrinsic method for start service - completed\n",__FUNCTION__);
//#endif //DEBUG
    
    return ra_status;
}

// ----------------------------------------------------------------------------

/* Extrinsic Method - StopService */
_RA_STATUS Linux_DHCPService_Method_StopService( unsigned int* methodResult, _RESOURCE* resource) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    *methodResult = 0;
    
//#ifdef DEBUG
//    fprintf(stderr,"\t -%s -- Processing extrinsic method for stop service\n",__FUNCTION__);
//#endif //DEBUG

    stop_service();
    //TODO - Include logic code here (if not required, remove this tag)
    
//#ifdef DEBUG
//    fprintf(stderr,"\t -%s -- Processing extrinsic method for stop service - completed\n",__FUNCTION__);
//#endif //DEBUG

    return ra_status;
}

// ----------------------------------------------------------------------------

/* Initialization method for Method Provider */
_RA_STATUS Linux_DHCPService_MethodProviderInitialize(_RA_STATUS *ra_status) {
    ra_Initialize( ra_status );
#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Initialization of method provider\n",__FUNCTION__);
#endif //DEBUG
    
    //TODO - Include logic code here (if not required, remove this tag)

#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Initialization of method provider - completed\n",__FUNCTION__);
#endif //DEBUG
    return (*ra_status);
}

// ----------------------------------------------------------------------------

/* CleanUp method for Method Provider */
_RA_STATUS Linux_DHCPService_MethodProviderCleanUp(bool terminate) {
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
	ra_CleanUp();
#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Cleanup of method provider\n",__FUNCTION__);
#endif //DEBUG

    //TODO - Include logic code here (if not required, remove this tag)

#ifdef DEBUG
    fprintf(stderr,"\t -%s -- Cleanup of method provider - completed\n",__FUNCTION__);
#endif //DEBUG
    return ra_status;
}
