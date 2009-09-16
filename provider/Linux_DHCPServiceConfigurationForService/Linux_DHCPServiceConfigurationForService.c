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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Include the required CMPI data types, function headers, and macros */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

/** Include our macros. */
#include "sblim-dhcp.h"

#include "Linux_DHCPServiceConfigurationForService_Resource.h"

/// ----------------------------------------------------------------------------
/// COMMON GLOBAL VARIABLES
/// ----------------------------------------------------------------------------

/** Handle to the CIM broker. Initialized when the provider lib is loaded. */
static const CMPIBroker *_BROKER;

/// ============================================================================
/// CMPI INSTANCE PROVIDER FUNCTION TABLE
/// ============================================================================

/// ----------------------------------------------------------------------------
/// Info for the class supported by the instance provider
/// ----------------------------------------------------------------------------

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** Name of the class implemented by this instance provider. */
static char * _CLASSNAME = _ASSOCCLASS;

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** NULL terminated list of key properties of this class. */
const static char * _KEYNAMES[] = {_LHSPROPERTYNAME, _RHSPROPERTYNAME, NULL};

/// ----------------------------------------------------------------------------
/// EnumInstanceNames()
/// Return a list of all the instances names (return their object paths only).
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_EnumInstanceNames(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference) 	/** [in] Contains target namespace and classname. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};	/** Return status of CIM operations. */
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    _RESOURCES * resources = NULL;			/** Handle to the list of system resources. */
    _RESOURCE * resource = NULL;			/** Handle to each system resource. */
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};


    const char * lnamespace =  CMGetCharsPtr(CMGetNameSpace(reference, &status), NULL); /** Target namespace. */
    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPServiceConfigurationForService_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	/** Create a new CMPIObjectPath to store this resource. */
	op = CMNewObjectPath( _BROKER, lnamespace, _CLASSNAME, &status);
	if( CMIsNullObject(op) ) { 
            build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
            goto clean_on_error; 
        }

	/** Create a new CMPIInstance to store this resource. */
	instance = CMNewInstance( _BROKER, op, &status);
	if( CMIsNullObject(instance) ) {
            build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
            goto clean_on_error; 
        }

	/** Set the instance property values from the resource data. */
	ra_status = Linux_DHCPServiceConfigurationForService_setInstanceFromResource(resource, instance, _BROKER);
	if (ra_status.rc != RA_RC_OK) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status );
            goto clean_on_error; 
        }

	/** Return the CMPIObjectPath for this instance. */
	CMPIObjectPath * objectpath = CMGetObjectPath(instance, &status);
	if ((status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath)) {
            setRaStatus( &ra_status, RA_RC_FAILED, OBJECT_PATH_IS_NULL, _("Object Path is NULL") );
	    build_ra_error_msg (_BROKER, &status, CMPI_RC_ERR_FAILED, _("Cannot get CMPIObjectPath for instance"), ra_status);
	    goto clean_on_error;
	}
	
	CMSetNameSpace(objectpath, lnamespace); /** Note - CMGetObjectPath() does not preserve the namespace! */
	CMReturnObjectPath(results, objectpath);
	ra_status = Linux_DHCPServiceConfigurationForService_getNextResource( resources, &resource);
    }

    if ( ra_status.rc != RA_RC_OK ) {
        setRaStatus( &ra_status, RA_RC_FAILED, FAILED_TO_GET_SYSTEM_RESOURCE, _("Failed to get resource data") );
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    }
    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }
   
    /**  Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }
    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// EnumInstances()
/// Return a list of all the instances (return all the instance data).
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_EnumInstances(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference,	/** [in] Contains target namespace and classname. */
	const char ** properties)		/** [in] List of desired properties (NULL=all). */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};	/** Return status of CIM operations. */
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    _RESOURCES * resources = NULL;		/** Handle to the list of system resources. */
    _RESOURCE * resource = NULL;		/** Handle to each system resource. */
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    const char * lnamespace =  CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL); /** Target namespace. */
    
    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources); 
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPServiceConfigurationForService_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	/** Create a new CMPIObjectPath to store this resource. */
	op = CMNewObjectPath( _BROKER, lnamespace, _CLASSNAME, &status);
	if( CMIsNullObject(op) ) { 
            build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
            goto clean_on_error; 
        }

	/** Create a new CMPIInstance to store this resource. */
	instance = CMNewInstance( _BROKER, op, &status);
	if( CMIsNullObject(instance) ) {
            setRaStatus( &ra_status, RA_RC_FAILED, INSTANCE_ID_IS_NULL, _("Instance is NULL") );
	    build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Create CMPIInstance failed.") , ra_status); 
	    goto clean_on_error;
	}

	/** Setup a filter to only return the desired properties. */
	status = CMSetPropertyFilter(instance, properties, _KEYNAMES);
	if (status.rc != CMPI_RC_OK) {	
	    build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Cannot set property filter"), ra_status);
	    goto clean_on_error;
	}

	/** Set the instance property values from the resource data. */
	ra_status = Linux_DHCPServiceConfigurationForService_setInstanceFromResource(resource, instance, _BROKER);
	if( ra_status.rc != RA_RC_OK ) {
	    build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
	    goto clean_on_error;
	}

	/** Return the CMPI Instance for this instance */
	CMReturnInstance(results, instance);

	ra_status = Linux_DHCPServiceConfigurationForService_getNextResource( resources, &resource);
    }

    if ( ra_status.rc != RA_RC_OK ) {
        setRaStatus( &ra_status, RA_RC_FAILED, FAILED_TO_GET_SYSTEM_RESOURCE, _("Failed to get resource data") );
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// GetInstance()
/// Return the instance data for the specified instance only.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_GetInstance(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference,	/** [in] Contains the target namespace, classname and object path. */
	const char ** properties)		/** [in] List of desired properties (NULL=all). */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};	/** Return status of CIM operations. */
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    _RESOURCE * resource = NULL;			/** Handle to the system resource. */
    _RESOURCES * resources = NULL;			/** Handle to the system resource. */
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    const char * lnamespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL); /** Target namespace. */


    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
  

     if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    } 

    
    /** Get the target resource. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResourceForObjectPath(_BROKER, context, resources, &resource, reference);
    if ( ra_status.rc != RA_RC_OK) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    } else if ( !resource ) {
        setRaStatus( &ra_status, RA_RC_FAILED, INSTANCE_NOT_FOUND, _("Target instance not found") );
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Target instance not found"), ra_status);
	goto clean_on_error;
    }

    /** Create a new CMPIObjectPath to store this resource. */
    op = CMNewObjectPath( _BROKER, lnamespace, _CLASSNAME, &status);
    if( CMIsNullObject(op) || (status.rc != CMPI_RC_OK)) { 
	build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
	goto clean_on_error; 
    }

    /** Create a new CMPIInstance to store this resource. */
    instance = CMNewInstance( _BROKER, op, &status);
    if( CMIsNullObject(instance) ) {
        setRaStatus( &ra_status, RA_RC_FAILED, INSTANCE_ID_IS_NULL, _("Instance is NULL") );
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Create CMPIInstance failed.") , ra_status); 
	goto clean_on_error;
    }

    /** Setup a filter to only return the desired properties. */
    status = CMSetPropertyFilter(instance, properties, _KEYNAMES);
    if (status.rc != CMPI_RC_OK) {
        setRaStatus( &ra_status, RA_RC_FAILED, CANNOT_SET_PROPERTY_FILTER, _("cannot set property filter") );
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Cannot set property filter"), ra_status);
	goto clean_on_error;
    }

    /** Set the instance property values from the resource data. */
    ra_status = Linux_DHCPServiceConfigurationForService_setInstanceFromResource(resource, instance, _BROKER);
    if( ra_status.rc != RA_RC_OK ) {
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }
    /** Return the CMPI Instance for this instance */
    CMReturnInstance(results, instance);

    CMReturnDone(results);
    goto exit;

clean_on_error:
  free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// ModifyInstance()
/// Save modified instance data for the specified instance.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_ModifyInstance(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference,	/** [in] Contains the target namespace, classname and object path. */
	const CMPIInstance * newinstance,	/** [in] Contains the new instance data. */
	const char** properties)	
{
	CMPIStatus status = {CMPI_RC_OK, NULL}; /** Return status of CIM operations. */
	CMReturnDone(results);
	build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );

	return status;
}

/// ----------------------------------------------------------------------------
/// CreateInstance()
/// Create a new instance from the specified instance data.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_CreateInstance(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,	 	/** [out] Results of this operation. */
	const CMPIObjectPath * reference,	/// [in] Contains the target namespace, classname & objectPath
	const CMPIInstance * newinstance)	/** [in] Contains the new instance data. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};	/** Return status of CIM operations. */
    _RESOURCES * resources = NULL;		/** Handle to the list of system resources. */
    _RESOURCE * resource = NULL;		/** Handle to the system resource. */
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    const char * lnamespace =  CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL); /** Target namespace. */

    build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
    goto exit;

    /** WORKAROUND FOR PEGASUS BUG?! reference does not contain object path, only namespace & classname. */
    reference = CMGetObjectPath(newinstance, NULL);

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Get the target resource. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResourceForObjectPath(_BROKER, context, NULL, &resource, reference);
    if ( ra_status.rc != RA_RC_OK) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    } else if ( !resource ) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Target instance not found"), ra_status);
	goto clean_on_error;
    }

    /** Set the instance property values from the resource data. */
    ra_status = Linux_DHCPServiceConfigurationForService_createResourceFromInstance(resources, &resource, newinstance, _BROKER);
    if( ra_status.rc != RA_RC_OK ) {
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to create resource data from instance"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    /** Return the object path for the newly created instance. */
    CMPIObjectPath * objectpath = CMGetObjectPath(newinstance, NULL);
    CMSetNameSpace(objectpath, lnamespace);
    CMReturnObjectPath(results, objectpath);
    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// DeleteInstance()
/// Delete or remove the specified instance from the system.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_DeleteInstance(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference)	/** [in] Contains the target namespace, classname and object path. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};	/** Return status of CIM operations. */
    _RESOURCES * resources = NULL;		/** Handle to the list of system resources. */
    _RESOURCE * resource = NULL;		/** Handle to the system resource. */
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
///    char * lnamespace = CMGetCharPtr(CMGetNameSpace(reference, NULL)); /** Target namespace. */

    build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
    goto exit;

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Get the target resource. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResourceForObjectPath(_BROKER, context, NULL, &resource, reference);
    if ( ra_status.rc != RA_RC_OK) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    } else if ( !resource ) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Target instance not found"), ra_status);
	goto clean_on_error;
    }

    /** Set the instance property values from the resource data. */
    ra_status = Linux_DHCPServiceConfigurationForService_deleteResource(resources, resource);
    if( ra_status.rc != RA_RC_OK ) {
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to delete resource"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// ExecQuery()
/// Return a list of all the instances that satisfy the specified query filter.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_ExecQuery(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference,	/** [in] Contains the target namespace and classname. */
	const char * language,			/** [in] Name of the query language. */
	const char * query)			/** [in] Text of the query written in the query language. */
{
	CMPIStatus status = {CMPI_RC_OK, NULL}; /** Return status of CIM operations. */

	CMReturnDone(results);
	return status;
}

/// ----------------------------------------------------------------------------
/// Initialize()
/// Perform any necessary initialization immediately after this provider is
/// first loaded.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPServiceConfigurationForService_Initialize(
	CMPIInstanceMI * self,		/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context)		/** [in] Additional context info, if any. */
{
	CMPIStatus status = {CMPI_RC_OK, NULL};
	return status;
}

/// ----------------------------------------------------------------------------
/// Cleanup()
/// Perform any necessary cleanup immediately before this provider is unloaded.
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPServiceConfigurationForService_Cleanup(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	CMPIBoolean terminating)
{
	CMPIStatus status = { CMPI_RC_OK, NULL };	/** Return status of CIM operations. */

	return status;
}

/// ============================================================================
/// CMPI ASSOCIATION PROVIDER FUNCTION TABLE
/// ============================================================================

/// ----------------------------------------------------------------------------
/// AssociationInitialize()
/// Perform any necessary initialization immediately after this provider is
/// first loaded.
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPServiceConfigurationForService_AssociationInitialize(
		CMPIAssociationMI * self,	/** [in] Handle to this provider (i.e. 'self'). */
		const CMPIContext * context)		/** [in] Additional context info, if any. */
{
	CMPIStatus status = {CMPI_RC_OK, NULL};
         _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
        ra_Initialize(&ra_status);
	return status;
}


/// ----------------------------------------------------------------------------
/// AssociationCleanup()
/// Perform any necessary cleanup immediately before this provider is unloaded.
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPServiceConfigurationForService_AssociationCleanup(
	CMPIAssociationMI * self,	/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	CMPIBoolean terminating)
{
	CMPIStatus status = { CMPI_RC_OK, NULL };	/** Return status of CIM operations. */

	return status;
}


/// ----------------------------------------------------------------------------
/// AssociatorNames()
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPServiceConfigurationForService_AssociatorNames(
	CMPIAssociationMI * self,	    /** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,	    /** [in] Additional context info, if any. */
	const CMPIResult * results,	    /** [out] Results of this operation. */
	const CMPIObjectPath * reference,   /** [in] Contains source namespace, classname and object path. */
	const char * assocClass,
	const char * resultClass,
	const char * role,
	const char * resultRole)
{
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus status = { CMPI_RC_OK, NULL };				/** Return status of CIM operations. */
    CMPIData cmpiInfo;
    _RESOURCE * resource;
    _RESOURCES * resources;
    bool want_lhs = false;
    int srcId = 0;
    const char* srcName = NULL;

    const char *srcClass = CMGetCharsPtr(CMGetClassName(reference, &status), NULL);

    //** Verify if both the associationClass name and ResultClass name have been supplied, else throw an error. */
    if( assocClass == NULL || resultClass == NULL)  {
         build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Both AssociationClass and ResultClass names need to be provided") );
         goto exit;
    }

    if (!strcmp((char*) srcClass, "Linux_DHCPServiceConfiguration"))
	cmpiInfo = CMGetKey(reference, "Name", NULL);
    else
        cmpiInfo = CMGetKey(reference, "SystemName", NULL);

    /** Determine the target class from the source class. */
    if (strcmp(srcClass, _LHSCLASSNAME) == 0) {
	want_lhs = false;
        srcId = ra_getKeyFromInstance( (char*) CMGetCharsPtr(cmpiInfo.value.string, NULL));
   
    } else if (strcmp((char*) srcClass, _RHSCLASSNAME) == 0) {
	want_lhs = true;
        srcName = CMGetCharsPtr(cmpiInfo.value.string, NULL);
    }

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPServiceConfigurationForService_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	CMPIObjectPath * curr = want_lhs ? resource->rhs : resource->lhs;

        if(!strcmp( (char*) CMGetCharsPtr(CMGetClassName(curr, &status), NULL), "Linux_DHCPServiceConfiguration")) {
	int destId = ra_getKeyFromInstance((char*) CMGetCharsPtr(cmpiInfo.value.string, NULL));

		if(srcId == destId){
	    	curr = want_lhs ? resource->lhs : resource->rhs;
	    	CMReturnObjectPath(results, curr);
	    	break;
		}
        }

        else {
	   curr = want_lhs ? resource->lhs : resource->rhs;
           CMReturnObjectPath(results, curr);
           break;
        }
	ra_status = Linux_DHCPServiceConfigurationForService_getNextResource( resources, &resource);
	if ( ra_status.rc != RA_RC_OK ) {
	    build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	    goto clean_on_error;
	}
    }


    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}


/// ----------------------------------------------------------------------------
/// Associators()
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPServiceConfigurationForService_Associators(
	CMPIAssociationMI * self,	/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference,	/** [in] Contains the source namespace, classname and object path. */
	const char *assocClass,
	const char *resultClass,
	const char *role,
	const char *resultRole,
	const char ** properties)		/** [in] List of desired properties (NULL=all). */
{
    _RA_STATUS ra_status = {RA_RC_OK , 0, NULL};
    CMPIStatus status = { CMPI_RC_OK, NULL };    /** Return status of CIM operations. */
    CMPIData cmpiInfo;
    _RESOURCE * resource;
    _RESOURCES * resources;
    bool want_lhs = false;
    int srcId = 0;
    char* srcName = NULL;

    const char *srcClass =  CMGetCharsPtr(CMGetClassName(reference, &status), NULL); 

    //** Verify if both the associationClass name and ResultClass name have been supplied, else throw an error. */
    if( assocClass == NULL || resultClass == NULL)  {
         build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Both AssociationClass and ResultClass names need to be provided") );
         goto exit;
    }

    if( !strcmp( srcClass, "Linux_DHCPServiceConfiguration"))
        cmpiInfo = CMGetKey(reference, "Name", NULL);
    else 
        cmpiInfo = CMGetKey(reference, "SystemName", NULL);

    if (strcmp(srcClass, _LHSCLASSNAME) == 0) {
	want_lhs = false;
        srcId = ra_getKeyFromInstance((char*) CMGetCharsPtr(cmpiInfo.value.string, NULL));
    } else if (strcmp(srcClass, _RHSCLASSNAME) == 0) {
	want_lhs = true;
        srcName = (char*) CMGetCharsPtr(cmpiInfo.value.string, NULL);
    }

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPServiceConfigurationForService_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	CMPIObjectPath * curr = want_lhs ? resource->rhs : resource->lhs;
        if(!strcmp( (char*) CMGetCharsPtr(CMGetClassName(curr, &status), NULL), "Linux_DHCPServiceConfiguration")) {
                int destId = ra_getKeyFromInstance((char *) CMGetCharsPtr(cmpiInfo.value.string, NULL));
                if(srcId == destId) {
                        curr = want_lhs ? resource->lhs : resource->rhs;
                        CMPIInstance * inst = CBGetInstance(_BROKER, context, curr, NULL, &status);
                        if ((CMIsNullObject(inst)) || (status.rc != CMPI_RC_OK))
                                {
                                        goto clean_on_error;
                                }
                        CMReturnInstance(results, inst);
                        break;
                }
        }
        else {
                curr = want_lhs ? resource->lhs : resource->rhs;
                        CMPIInstance * inst = CBGetInstance(_BROKER, context, curr, NULL, &status);
                        if ((CMIsNullObject(inst)) || (status.rc != CMPI_RC_OK))
                                {
                                        goto clean_on_error;
                                }
                        CMReturnInstance(results, inst);
                        break;
        }
        ra_status = Linux_DHCPServiceConfigurationForService_getNextResource( resources, &resource);
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
            goto clean_on_error;
        }
    }


    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// ReferenceNames()
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPServiceConfigurationForService_ReferenceNames(
	CMPIAssociationMI * self,	    /** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,	    /** [in] Additional context info, if any. */
	const CMPIResult * results,	    /** [out] Results of this operation. */
	const CMPIObjectPath * reference,   ///** [in] Contains the source namespace, classname and object path.
	const char *resultClass, 
	const char *role)
{
    _RA_STATUS ra_status = { RA_RC_OK, 0, NULL};
    CMPIStatus status = { CMPI_RC_OK, NULL };    /** Return status of CIM operations. */
    CMPIData cmpiInfo;
    _RESOURCE * resource;
    _RESOURCES * resources;
    bool check_lhs = false;
    int srcId = 0;
    char* srcName = NULL;
    const char *namespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL);
    const char *srcClass =  CMGetCharsPtr(CMGetClassName(reference, &status), NULL);
    if( !strcmp(srcClass, "Linux_DHCPServiceConfiguration"))
        cmpiInfo = CMGetKey(reference, "Name", NULL);
    else
        cmpiInfo = CMGetKey(reference, "SystemName", NULL);

    if (strcmp((char*) srcClass, _LHSCLASSNAME) == 0) {
        check_lhs = true;
        srcId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));
    } else if (strcmp(srcClass, _RHSCLASSNAME) == 0) {
        check_lhs = false;
        srcName = (char *) CMGetCharsPtr(cmpiInfo.value.string, NULL);
    }

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPServiceConfigurationForService_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	CMPIObjectPath * curr = check_lhs ? resource->lhs : resource->rhs;
        if(!strcmp( (char*) CMGetCharsPtr(CMGetClassName(curr, &status), NULL), "Linux_DHCPServiceConfiguration")) {
                int destId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));

                if(destId == srcId) {
                        CMPIObjectPath * assocOp = CMNewObjectPath(_BROKER, namespace, _ASSOCCLASS, &status);
                         if (CMIsNullObject(assocOp) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIObjectPath failed."));
                                goto clean_on_error;
                        }

                        CMPIInstance * assocInst = CMNewInstance(_BROKER, assocOp,&status);
                        if (CMIsNullObject(assocInst) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIInstance failed."));
                                goto clean_on_error;
                        }

                        ra_status = Linux_DHCPServiceConfigurationForService_setInstanceFromResource(resource, assocInst, _BROKER);
                        if (ra_status.rc != RA_RC_OK) {
                                build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
                                goto clean_on_error;
                        }
                        assocOp = CMGetObjectPath(assocInst, NULL);
                        CMSetNameSpace(assocOp, namespace);
                        CMReturnObjectPath(results, assocOp);
                        break;
                }
        } else {
                 CMPIObjectPath * assocOp = CMNewObjectPath(_BROKER, namespace, _ASSOCCLASS, &status);
                         if (CMIsNullObject(assocOp) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIObjectPath failed."));
                                goto clean_on_error;
                        }

                        CMPIInstance * assocInst = CMNewInstance(_BROKER, assocOp,&status);
                        if (CMIsNullObject(assocInst) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIInstance failed."));
                                goto clean_on_error;
                        }

                        ra_status = Linux_DHCPServiceConfigurationForService_setInstanceFromResource(resource, assocInst, _BROKER);
                        if (ra_status.rc != RA_RC_OK) {
                                build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
                                goto clean_on_error;
                        }
                        assocOp = CMGetObjectPath(assocInst, NULL);
                        CMSetNameSpace(assocOp, namespace);
                        CMReturnObjectPath(results, assocOp);
                        break;

        }

        ra_status = Linux_DHCPServiceConfigurationForService_getNextResource( resources, &resource);
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
            goto clean_on_error;
        }
    }


    /** Free system resource */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// References()
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPServiceConfigurationForService_References(
	CMPIAssociationMI * self,	    /** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,	    /** [in] Additional context info, if any. */
	const CMPIResult * results,	    /** [out] Results of this operation. */
	const CMPIObjectPath * reference,   /// [in] Contains the namespace, classname and desired object path.
	const char *resultClass,
	const char *role,
	const char **properties)	    /** [in] List of desired properties (NULL=all). */
{
    _RA_STATUS ra_status = { RA_RC_OK, 0, NULL};
    CMPIStatus status = { CMPI_RC_OK, NULL };    /** Return status of CIM operations. */
    CMPIData cmpiInfo;
    _RESOURCE * resource;
    _RESOURCES * resources;
    bool check_lhs = false;
    int srcId =0;
    char* srcName = NULL;
    const char *namespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL);
    const char *srcClass = CMGetCharsPtr(CMGetClassName(reference, &status), NULL);
    if( !strcmp((char*)srcClass, "Linux_DHCPServiceConfiguration"))
        cmpiInfo = CMGetKey(reference, "Name", NULL);
    else
        cmpiInfo = CMGetKey(reference, "SystemName", NULL);

    if (strcmp((char*) srcClass, _LHSCLASSNAME) == 0) {
        check_lhs = true;
        srcId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));
    } else if (strcmp(srcClass, _RHSCLASSNAME) == 0) {
        check_lhs = false;
        srcName = (char *)CMGetCharsPtr(cmpiInfo.value.string, NULL);
    }

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPServiceConfigurationForService_getResources(_BROKER, context, reference, &resources);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }


    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPServiceConfigurationForService_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	CMPIObjectPath * curr = check_lhs ? resource->lhs : resource->rhs;
        if(!strcmp( (char*) CMGetCharsPtr(CMGetClassName(curr, &status), NULL), "Linux_DHCPServiceConfiguration")) {
                int destId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));
                if(destId == srcId) {
                        CMPIObjectPath * assocOp = CMNewObjectPath(_BROKER, namespace, _ASSOCCLASS, &status);
                        if (CMIsNullObject(assocOp) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIObjectPath failed."));
                                goto clean_on_error;
                        }
                        CMPIInstance * assocInst = CMNewInstance(_BROKER, assocOp,&status);
                        if (CMIsNullObject(assocInst) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIInstance failed."));
                                goto clean_on_error;
                        }
                        ra_status = Linux_DHCPServiceConfigurationForService_setInstanceFromResource(resource, assocInst, _BROKER);
                        if (ra_status.rc != RA_RC_OK) {
                                build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
                                goto clean_on_error;
                        }
                        CMReturnInstance(results, assocInst);
                        break;
                }
        } else {
                CMPIObjectPath * assocOp = CMNewObjectPath(_BROKER, namespace, _ASSOCCLASS, &status);
                        if (CMIsNullObject(assocOp) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIObjectPath failed."));
                                goto clean_on_error;
                        }
                        CMPIInstance * assocInst = CMNewInstance(_BROKER, assocOp,&status);
                        if (CMIsNullObject(assocInst) || status.rc != CMPI_RC_OK) {
                                CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERROR, _("Create CMPIInstance failed."));
                                goto clean_on_error;
                        }
                        ra_status = Linux_DHCPServiceConfigurationForService_setInstanceFromResource(resource, assocInst, _BROKER);
                        if (ra_status.rc != RA_RC_OK) {
                                build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
                                goto clean_on_error;
                        }
                        CMReturnInstance(results, assocInst);
                        break;

        }

        ra_status = Linux_DHCPServiceConfigurationForService_getNextResource( resources, &resource);
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
            goto clean_on_error;
        }
    }

    /** Free system resource */
   ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPServiceConfigurationForService_freeResource( resource );
    ra_status = Linux_DHCPServiceConfigurationForService_freeResources( resources );

exit:
    return status;
}


/// ============================================================================
/// CMPI PROVIDER FUNCTION TABLE SETUP
/// ============================================================================
CMInstanceMIStub(Linux_DHCPServiceConfigurationForService_, Linux_DHCPServiceConfigurationForServiceProvider, _BROKER, Linux_DHCPServiceConfigurationForService_Initialize(&mi, ctx));
CMAssociationMIStub(Linux_DHCPServiceConfigurationForService_, Linux_DHCPServiceConfigurationForServiceProvider, _BROKER, Linux_DHCPServiceConfigurationForService_AssociationInitialize(&mi, ctx));
