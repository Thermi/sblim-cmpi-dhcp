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

#include "Linux_DHCPPoolsForEntity_Resource.h"

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
CMPIStatus Linux_DHCPPoolsForEntity_EnumInstanceNames(
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

    const char * lnamespace = CMGetCharsPtr(CMGetNameSpace(reference, &status), NULL); /** Target namespace. */

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, 3);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPPoolsForEntity_getNextResource(resources, &resource);
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
	ra_status = Linux_DHCPPoolsForEntity_setInstanceFromResource(resource, instance, _BROKER);
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
	ra_status = Linux_DHCPPoolsForEntity_getNextResource( resources, &resource);
    }

    if ( ra_status.rc != RA_RC_OK ) {
        setRaStatus( &ra_status, RA_RC_FAILED, FAILED_TO_GET_SYSTEM_RESOURCE, _("Failed to get resource data") );
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /**  Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// EnumInstances()
/// Return a list of all the instances (return all the instance data).
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPPoolsForEntity_EnumInstances(
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
    const char * lnamespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL); /** Target namespace. */
    
    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, 3); 
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPPoolsForEntity_getNextResource(resources, &resource);
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
	ra_status = Linux_DHCPPoolsForEntity_setInstanceFromResource(resource, instance, _BROKER);
	if( ra_status.rc != RA_RC_OK ) {
	    build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
	    goto clean_on_error;
	}

	/** Return the CMPI Instance for this instance */
	CMReturnInstance(results, instance);

	ra_status = Linux_DHCPPoolsForEntity_getNextResource( resources, &resource);
    }

    if ( ra_status.rc != RA_RC_OK ) {
        setRaStatus( &ra_status, RA_RC_FAILED, FAILED_TO_GET_SYSTEM_RESOURCE, _("Failed to get resource data") );
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// GetInstance()
/// Return the instance data for the specified instance only.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPPoolsForEntity_GetInstance(
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
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, 3);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Get the target resource. */
    ra_status = Linux_DHCPPoolsForEntity_getResourceForObjectPath(_BROKER, context, resources, &resource, reference);
    if ( ra_status.rc != RA_RC_OK || resource == NULL) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
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
    ra_status = Linux_DHCPPoolsForEntity_setInstanceFromResource(resource, instance, _BROKER);
    if( ra_status.rc != RA_RC_OK ) {
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
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
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// ModifyInstance()
/// Save modified instance data for the specified instance.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPPoolsForEntity_ModifyInstance(
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
CMPIStatus Linux_DHCPPoolsForEntity_CreateInstance(
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
    const char * lnamespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL); /** Target namespace. */

    build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
    goto exit;

    /** WORKAROUND FOR PEGASUS BUG?! reference does not contain object path, only namespace & classname. */
    reference = CMGetObjectPath(newinstance, NULL);

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, 3);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Get the target resource. */
    ra_status = Linux_DHCPPoolsForEntity_getResourceForObjectPath(_BROKER, context, NULL, &resource, reference);
    if ( ra_status.rc != RA_RC_OK) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    } else if ( !resource ) {
        setRaStatus( &ra_status, RA_RC_FAILED, INSTANCE_NOT_FOUND, _("Target instance not found") );
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Target instance not found"), ra_status);
	goto clean_on_error;
    }

    /** Set the instance property values from the resource data. */
    ra_status = Linux_DHCPPoolsForEntity_createResourceFromInstance(resources, &resource, newinstance, _BROKER);
    if( ra_status.rc != RA_RC_OK ) {
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to create resource data from instance"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
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
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// DeleteInstance()
/// Delete or remove the specified instance from the system.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPPoolsForEntity_DeleteInstance(
	CMPIInstanceMI * self,			/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	const CMPIResult * results,		/** [out] Results of this operation. */
	const CMPIObjectPath * reference)	/** [in] Contains the target namespace, classname and object path. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};	/** Return status of CIM operations. */
    _RESOURCES * resources = NULL;		/** Handle to the list of system resources. */
    _RESOURCE * resource = NULL;		/** Handle to the system resource. */
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

    build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
    goto exit;

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, 3);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Get the target resource. */
    ra_status = Linux_DHCPPoolsForEntity_getResourceForObjectPath(_BROKER, context, NULL, &resource, reference);
    if ( ra_status.rc != RA_RC_OK) {
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	goto clean_on_error;
    } else if ( !resource ) {
        setRaStatus( &ra_status, RA_RC_FAILED, INSTANCE_NOT_FOUND, _("Target instance not found") );
	build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Target instance not found"), ra_status);
	goto clean_on_error;
    }

    /** Set the instance property values from the resource data. */
    ra_status = Linux_DHCPPoolsForEntity_deleteResource(resources, resource);
    if( ra_status.rc != RA_RC_OK ) {
	build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to delete resource"), ra_status);
	goto clean_on_error;
    }

    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// ExecQuery()
/// Return a list of all the instances that satisfy the specified query filter.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPPoolsForEntity_ExecQuery(
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
CMPIStatus Linux_DHCPPoolsForEntity_Initialize(
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
static CMPIStatus Linux_DHCPPoolsForEntity_Cleanup(
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
static CMPIStatus Linux_DHCPPoolsForEntity_AssociationInitialize(
		CMPIAssociationMI * self,	/** [in] Handle to this provider (i.e. 'self'). */
		const CMPIContext * context)		/** [in] Additional context info, if any. */
{
	CMPIStatus status = {CMPI_RC_OK, NULL};

	return status;
}


/// ----------------------------------------------------------------------------
/// AssociationCleanup()
/// Perform any necessary cleanup immediately before this provider is unloaded.
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPPoolsForEntity_AssociationCleanup(
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
static CMPIStatus Linux_DHCPPoolsForEntity_AssociatorNames(
	CMPIAssociationMI * self,	    /** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,	    /** [in] Additional context info, if any. */
	const CMPIResult * results,	    /** [out] Results of this operation. */
	const CMPIObjectPath * reference,   /** [in] Contains source namespace, classname and object path. */
	const char * assocClass,
	const char * resultClass,
	const char * role,
	const char * resultRole)
{
    int returnResult = 0, count = 0;
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    CMPIStatus status = { CMPI_RC_OK, NULL };				/** Return status of CIM operations. */
    CMPIData cmpiInfo;
    _RESOURCE * resource;
    _RESOURCES * resources;


    cmpiInfo = CMGetKey(reference, "InstanceID", NULL);
    int srcId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));

    //** Verify if both the associationClass name and ResultClass name have been supplied, else throw an error. */
    if( assocClass == NULL || resultClass == NULL)  {
         build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Both AssociationClass and ResultClass names need to be provided") );
         goto exit;
    }

    if(role == NULL){
	if(resultRole == NULL) {
	    returnResult = 3;
	} else {
	    if (strcasecmp(resultRole, "PartComponent") == 0)
		returnResult = 1;
	    if (strcasecmp(resultRole, "GroupComponent") == 0)
		returnResult = 2;
	}
    } else {
	if(resultRole == NULL){
	    if(strcasecmp(role, "GroupComponent") == 0)
		returnResult = 1;
	    if( strcasecmp(role, "PartComponent") == 0)
		returnResult = 2;
	} else {
	    if( (strcasecmp(role, "GroupComponent") == 0) || (strcasecmp(resultRole, "PartComponent") == 0))
		returnResult = 1;
	    else
		returnResult = 2;
	}
    }

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, returnResult);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPPoolsForEntity_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	const char * currClass = NULL;
	CMPIObjectPath * curr =  resource->lhs;
	cmpiInfo = CMGetKey(curr, "InstanceID", &status);
	int currId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));

	if(srcId == currId){
	    currClass = CMGetCharsPtr(CMGetClassName(resource->rhs, &status), NULL);
	    if(strcasecmp((char*) currClass, resultClass) == 0 || strcasecmp((char *)"Linux_DHCPEntity", resultClass) == 0)
		CMReturnObjectPath(results, resource->rhs);
	}

	curr =  resource->rhs;
	cmpiInfo = CMGetKey(curr, "InstanceID", NULL);
	currId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));

	if(srcId == currId){
	    currClass = CMGetCharsPtr(CMGetClassName(resource->lhs, &status), NULL);
	    if(strcasecmp((char*) currClass, resultClass) == 0 || strcasecmp((char *)"Linux_DHCPEntity", resultClass) == 0)
		CMReturnObjectPath(results, resource->lhs);
	}

	ra_status = Linux_DHCPPoolsForEntity_getNextResource( resources, &resource);
	if ( ra_status.rc != RA_RC_OK ) {
	    build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	    goto clean_on_error;
	}
    }


    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}


/// ----------------------------------------------------------------------------
/// Associators()
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPPoolsForEntity_Associators(
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

    cmpiInfo = CMGetKey(reference, "InstanceID", NULL);
    int srcId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));
    int returnResult = 0;

    //** Verify if both the associationClass name and ResultClass name have been supplied, else throw an error. */
    if( assocClass == NULL || resultClass == NULL)  {
         build_cmpi_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Both AssociationClass and ResultClass names need to be provided") );
         goto exit;
    }

    if(role == NULL){
	if(resultRole == NULL) {
	    returnResult = 3;
	} else {
	    if (strcasecmp(resultRole, "PartComponent") == 0)
		returnResult = 1;
	    if (strcasecmp(resultRole, "GroupComponent") == 0)
		returnResult = 2;
	}
    } else {
	if(resultRole == NULL){
	    if(strcasecmp(role, "GroupComponent") == 0)
		returnResult = 1;
	    if( strcasecmp(role, "PartComponent") == 0)
		returnResult = 2;
	} else {
	    if( (strcasecmp(role, "GroupComponent") == 0) || (strcasecmp(resultRole, "PartComponent") == 0))
		returnResult = 1;
	    else
		returnResult = 2;
	}
    }

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, returnResult);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPPoolsForEntity_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	const char * currClass = NULL;
	CMPIObjectPath * curr =  resource->lhs;
	cmpiInfo = CMGetKey(curr, "InstanceID", NULL);
	int currId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));

	if(srcId == currId){
	    currClass = CMGetCharsPtr(CMGetClassName(resource->rhs, &status), NULL);
	    if(strcasecmp((char*) currClass, resultClass) == 0 || strcasecmp((char *)"Linux_DHCPEntity", resultClass) == 0){
		CMPIInstance * inst = CBGetInstance(_BROKER, context, resource->rhs, NULL, &status);
		if ((CMIsNullObject(inst)) || (status.rc != CMPI_RC_OK))
		{
		    goto clean_on_error;
		}
		CMReturnInstance(results, inst);
	    }
	}

	curr =  resource->rhs;
	cmpiInfo = CMGetKey(curr, "InstanceID", NULL);
	currId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));

	if(srcId == currId){
	    currClass = CMGetCharsPtr(CMGetClassName(resource->lhs, &status), NULL);
	    if(strcasecmp((char*) currClass, resultClass) == 0 || strcasecmp((char *)"Linux_DHCPEntity", resultClass) == 0){
		CMPIInstance * inst = CBGetInstance(_BROKER, context, resource->lhs, NULL, &status);
		if ((CMIsNullObject(inst)) || (status.rc != CMPI_RC_OK))
		{
		    goto clean_on_error;
		}
		CMReturnInstance(results, inst);
	    }
	}

	ra_status = Linux_DHCPPoolsForEntity_getNextResource( resources, &resource);
	if ( ra_status.rc != RA_RC_OK ) {
	    build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	    goto clean_on_error;
	}
    }


    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// ReferenceNames()
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPPoolsForEntity_ReferenceNames(
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
    bool check_lhs;

    const char *namespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL);	    /** Target namespace. */
    const char *srcClass = CMGetCharsPtr(CMGetClassName(reference, &status), NULL);   /// Class of the source  object
    cmpiInfo = CMGetKey(reference, "InstanceID", NULL);
    int srcId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));


    if (strcmp(srcClass, _LHSCLASSNAME) == 0) 
	check_lhs = true;
    else
	check_lhs = false;

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, 3);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
         free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPPoolsForEntity_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	CMPIObjectPath * curr = check_lhs ? resource->lhs : resource->rhs;
	cmpiInfo = CMGetKey(curr, "InstanceID", NULL);
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

	    ra_status = Linux_DHCPPoolsForEntity_setInstanceFromResource(resource, assocInst, _BROKER);
	    if (ra_status.rc != RA_RC_OK) {
		build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
		goto clean_on_error;
	    }
	    assocOp = CMGetObjectPath(assocInst, NULL);
	    CMSetNameSpace(assocOp, namespace);
	    CMReturnObjectPath(results, assocOp);
	    break;
	}
	ra_status = Linux_DHCPPoolsForEntity_getNextResource( resources, &resource);
	if ( ra_status.rc != RA_RC_OK ) {
	    build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	    goto clean_on_error;
	}
    }

    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
     free_ra_status(ra_status);
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// References()
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPPoolsForEntity_References(
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
    bool check_lhs;

    const char *namespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL);	    /** Target namespace. */
    const char *srcClass = CMGetCharsPtr(CMGetClassName(reference, &status), NULL);   /// Class of the source  object
    cmpiInfo = CMGetKey(reference, "InstanceID", NULL);
    int srcId = ra_getKeyFromInstance((char *)CMGetCharsPtr(cmpiInfo.value.string, NULL));


    if (strcmp((char*) srcClass, _LHSCLASSNAME) == 0) 
	check_lhs = true;
    else
	check_lhs = false;

    /** Get a handle to the list of system resources. */
    ra_status = Linux_DHCPPoolsForEntity_getResources(_BROKER, context, reference, &resources, 3);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        free_ra_status(ra_status);
        goto exit;
    }

    /** Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPPoolsForEntity_getNextResource(resources, &resource);
    while(ra_status.rc == RA_RC_OK && resource){
	CMPIObjectPath * curr = check_lhs ? resource->lhs : resource->rhs;
	cmpiInfo = CMGetKey(curr, "InstanceID", NULL);
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

	    ra_status = Linux_DHCPPoolsForEntity_setInstanceFromResource(resource, assocInst, _BROKER);
	    if (ra_status.rc != RA_RC_OK) {
		build_ra_error_msg( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status);
		goto clean_on_error;
	    }

	    CMReturnInstance(results, assocInst);
	    break;
	}
	ra_status = Linux_DHCPPoolsForEntity_getNextResource( resources, &resource);
	if ( ra_status.rc != RA_RC_OK ) {
	    build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status);
	    goto clean_on_error;
	}
    }

    /** Free system resource */
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free system resource"), ra_status );
        goto clean_on_error;
    }

    /** Free list of system resources */
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone(results);
    goto exit;

clean_on_error:
    free_ra_status(ra_status);
    ra_status = Linux_DHCPPoolsForEntity_freeResource( resource );
    ra_status = Linux_DHCPPoolsForEntity_freeResources( resources );

exit:
    return status;
}


/// ============================================================================
/// CMPI PROVIDER FUNCTION TABLE SETUP
/// ============================================================================
CMInstanceMIStub(Linux_DHCPPoolsForEntity_, Linux_DHCPPoolsForEntityProvider, _BROKER, Linux_DHCPPoolsForEntity_Initialize(&mi, ctx));
CMAssociationMIStub(Linux_DHCPPoolsForEntity_, Linux_DHCPPoolsForEntityProvider, _BROKER, Linux_DHCPPoolsForEntity_AssociationInitialize(&mi, ctx));
