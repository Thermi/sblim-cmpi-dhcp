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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Include the required CMPI data types, function headers, and macros */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

/* The include for common MetaCluster settings */
#include "sblim-dhcp.h"

/* Include the abstract resource access functions and abstracted _RESOURCES and _RESOURCE data types. */
#include "Linux_DHCPParams_Resource.h"

#ifndef CMPI_VER_100
#define Linux_DHCPParams_ModifyInstance Linux_DHCPParams_SetInstance
#endif

// ----------------------------------------------------------------------------
// COMMON GLOBAL VARIABLES
// ----------------------------------------------------------------------------

/* Handle to the CIM broker. Initialized when the provider lib is loaded. */
static const CMPIBroker *_BROKER;


// ============================================================================
// CMPI INSTANCE PROVIDER FUNCTION TABLE
// ============================================================================

// ----------------------------------------------------------------------------
// Info for the class supported by the instance provider
// ----------------------------------------------------------------------------

/*** CUSTOMIZE FOR EACH PROVIDER ***/
/* NULL terminated list of key properties of this class. */
static const char * _KEYNAMES[] = {"InstanceID", NULL};

// ----------------------------------------------------------------------------
// EnumInstanceNames()
// Return a list of all the instances names (return their object paths only).
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_EnumInstanceNames(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference)    /* [in] Contains target namespace and classname. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    const char * namespace = CMGetCharsPtr( CMGetNameSpace( reference, &status ), NULL );
    int found = 0;

    if ( !isEnumerateInstanceNamesSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPParams_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPParams_getNextResource( resources, &resource );
    while ( ra_status.rc == RA_RC_OK && resource ) {
        /* Create a new CMPIObjectPath to store this resource. */
        op = CMNewObjectPath( _BROKER, namespace, _CLASSNAME, &status );
        if ( CMIsNullObject( op ) ) { 
            build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
            goto clean_on_error; 
        }

        /* Create a new CMPIInstance to store this resource. */
        instance = CMNewInstance( _BROKER, op, &status );
        if ( CMIsNullObject( instance ) ) {
            build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIInstance failed"));
            goto clean_on_error; 
        }

        /* Set the instance property values from the resource data. */
        ra_status = Linux_DHCPParams_setInstanceFromResource( resource, instance, _BROKER );
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status );
            goto clean_on_error; 
        }

        /* Free the resource data. */
        ra_status = Linux_DHCPParams_freeResource( resource );
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
            goto clean_on_error; 
        }

        /* Return the CMPIObjectPath for this instance. */
        CMPIObjectPath * objectpath = CMGetObjectPath( instance, &status );
        if ( (status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath) ) {
            build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get CMPIObjectPath from CMPIInstance") );
            goto clean_on_error; 
        }

        CMSetNameSpace( objectpath, namespace ); /* Note - CMGetObjectPath() does not preserve the namespace! */

        CMReturnObjectPath( results, objectpath );
        found++;
        ra_status = Linux_DHCPParams_getNextResource( resources, &resource );
    }

    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPParams_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone( results );
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPParams_freeResource( resource );
    ra_status = Linux_DHCPParams_freeResources( resources );

exit:

    return status;
}

// ----------------------------------------------------------------------------
// EnumInstances()
// Return a list of all the instances (return all the instance data).
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_EnumInstances(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains target namespace and classname. */
            const char ** properties)            /* [in] List of desired properties (NULL=all). */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    const char * namespace = CMGetCharsPtr( CMGetNameSpace( reference, NULL ), NULL );
    int found = 0;

    if ( !isEnumerateInstancesSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPParams_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPParams_getNextResource( resources, &resource );
    while ( ra_status.rc == RA_RC_OK && resource ) {
        /* Create a new CMPIObjectPath to store this resource. */
        op = CMNewObjectPath( _BROKER, namespace, _CLASSNAME, &status );
        if( CMIsNullObject(op) ) { 
            build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
            goto clean_on_error; 
        }

        /* Create a new CMPIInstance to store this resource. */
        instance = CMNewInstance( _BROKER, op, &status );
        if ( CMIsNullObject( instance ) ) {
            build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIInstance failed") );
            goto clean_on_error; 
        }

        /* Setup a filter to only return the desired properties. */
        status = CMSetPropertyFilter( instance, properties, _KEYNAMES );
        if ( status.rc != CMPI_RC_OK ) {
            build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property filter") );
            goto clean_on_error; 
        }

        /* Set the instance property values from the resource data. */
        ra_status = Linux_DHCPParams_setInstanceFromResource( resource, instance, _BROKER );
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status );
            goto clean_on_error; 
        }

        /* Free the resource data. */
        ra_status = Linux_DHCPParams_freeResource( resource );
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
            goto clean_on_error; 
        }

        /* Return the CMPIInstance for this instance. */
        CMReturnInstance(results, instance);
        found++;
        ra_status = Linux_DHCPParams_getNextResource( resources, &resource );
    }

    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPParams_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone( results );
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPParams_freeResource( resource );
    ra_status = Linux_DHCPParams_freeResources( resources );

exit:

    return status;
}

// ----------------------------------------------------------------------------
// GetInstance()
// Return the instance data for the specified instance only.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_GetInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace, classname and object path. */
            const char ** properties)            /* [in] List of desired properties (NULL=all). */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    const char * namespace = CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL);

    if ( !isGetSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPParams_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPParams_getResourceForObjectPath( resources, &resource, reference );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;

    } else if ( !resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("Target instance not found") );
        goto clean_on_error;
    }

    /* Create a new CMPIObjectPath to store this resource. */
    op = CMNewObjectPath( _BROKER, namespace, _CLASSNAME, &status );
    if( CMIsNullObject( op ) ) { 
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed")); 
        goto clean_on_error;
    }

    /* Create a new CMPIInstance to store this resource. */
    instance = CMNewInstance( _BROKER, op, &status );
    if( CMIsNullObject( instance ) ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIInstance failed"));
        goto clean_on_error;
    }

    /* Setup a filter to only return the desired properties. */
    status = CMSetPropertyFilter( instance, properties, _KEYNAMES );
    if ( status.rc != CMPI_RC_OK ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property filter") );
        goto clean_on_error;
    }

    /* Set the instance property values from the resource data. */
    ra_status = Linux_DHCPParams_setInstanceFromResource( resource, instance, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free the resource data. */
    ra_status = Linux_DHCPParams_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPParams_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    /* Return the CMPIInstance for this instance. */
    CMReturnInstance( results, instance );

    CMReturnDone( results );
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPParams_freeResource( resource );
    ra_status = Linux_DHCPParams_freeResources( resources );

exit:

    return status;
}

// ----------------------------------------------------------------------------
// ModifyInstance()
// Save modified instance data for the specified instance.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_ModifyInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace, classname and object path. */
            const CMPIInstance * newinstance,    /* [in] Contains the new instance data. */
            const char** properties)             /* [in] List of desired properties (NULL=all). */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

    if ( !isModifySupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPParams_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPParams_getResourceForObjectPath( resources, &resource, reference );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;

    } else if ( !resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("Target instance not found") );
        goto clean_on_error;
    }

    /* Update the target resource data with the new instance property values. */
    ra_status = Linux_DHCPParams_setResourceFromInstance( &resource, newinstance, properties, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to modify resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free the resource data. */
    ra_status = Linux_DHCPParams_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPParams_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPParams_freeResource( resource );
    ra_status = Linux_DHCPParams_freeResources( resources );

exit:

    return status;
}

// ----------------------------------------------------------------------------
// CreateInstance()
// Create a new instance from the specified instance data.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_CreateInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace, classname and object path. */
            const CMPIInstance * newinstance)    /* [in] Contains the new instance data. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    const char * namespace = CMGetCharsPtr( CMGetNameSpace( reference, NULL ), NULL );

    if ( !isCreateSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
    }

    /* WORKAROUND FOR PEGASUS BUG?! reference does not contain object path, only namespace & classname. */
    reference = CMGetObjectPath( newinstance, NULL );

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPParams_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPParams_getResourceForObjectPath( resources, &resource, reference );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;

    } else if ( resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_ALREADY_EXISTS, _("Target instance already exists") );
        goto clean_on_error;
    }

    /* Create a new resource with the new instance property values. */
    ra_status = Linux_DHCPParams_createResourceFromInstance( resources, &resource, newinstance, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to create resource data"), ra_status );
        goto clean_on_error;
    }

    /* Return the object path for the newly created instance. */
    CMPIObjectPath * objectpath = CMGetObjectPath( newinstance, NULL );
    ra_status = Linux_DHCPParams_BuildObjectPath(objectpath, (CMPIInstance*)newinstance , (char*)namespace, resource);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to build object path for the new instance"), ra_status );
        goto clean_on_error;
    }
    /* Free the resource data. */
    ra_status = Linux_DHCPParams_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPParams_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnObjectPath( results, objectpath );
    CMReturnDone( results );
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPParams_freeResource( resource );
    ra_status = Linux_DHCPParams_freeResources( resources );

exit:

    return status;
}

// ----------------------------------------------------------------------------
// DeleteInstance()
// Delete or remove the specified instance from the system.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_DeleteInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference)  	 /* [in] Contains the target namespace, classname and object path. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

    if ( !isDeleteSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
	goto exit;
    }
    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPParams_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPParams_getResourceForObjectPath( resources, &resource, reference );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;

    } else if ( !resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("Target instance not found") );
        goto clean_on_error;
    }

    /* Delete the target resource. */
    ra_status = Linux_DHCPParams_deleteResource( resources, resource, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to delete resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free the resource data. */
    ra_status = Linux_DHCPParams_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPParams_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }
    goto exit;


clean_on_error:

    ra_status = Linux_DHCPParams_freeResource( resource );
    ra_status = Linux_DHCPParams_freeResources( resources );

exit:

    return status;
}


// ----------------------------------------------------------------------------
// ExecQuery()
// Return a list of all the instances that satisfy the specified query filter.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_ExecQuery(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace and classname. */
            const char * language,               /* [in] Name of the query language. */
            const char * query)                  /* [in] Text of the query written in the query language. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

    /* EXECQUERY() IS NOT YET SUPPORTED FOR THIS CLASS */
    CMSetStatus( &status, CMPI_RC_ERR_NOT_SUPPORTED );

    CMReturnDone( results );

    return status;
}


// ----------------------------------------------------------------------------
// Initialize()
// Perform any necessary initialization immediately after this provider is
// first loaded.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPParams_Initialize(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context)         /* [in] Additional context info, if any. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

    /* Initialize instance provider */
    ra_status = Linux_DHCPParams_InstanceProviderInitialize( &ra_status);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to initialize instance provider"), ra_status );
    }


    return status;
}

// ----------------------------------------------------------------------------
// Cleanup()
// Perform any necessary cleanup immediately before this provider is unloaded.
// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPParams_Cleanup(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            CMPIBoolean terminating)             /* [in] Switch to wether to teminate or not*/ 
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    bool lTerminating = false;

    if (terminating) {
        lTerminating = true;
    }

    /* Cleanup instance provider */
    ra_status = Linux_DHCPParams_InstanceProviderCleanUp(lTerminating);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to cleanup instance provider"), ra_status );
    }


    return status;
}


// ============================================================================
// CMPI INSTANCE PROVIDER FUNCTION TABLE SETUP
// ============================================================================
CMInstanceMIStub( Linux_DHCPParams_ , Linux_DHCPParamsProvider, _BROKER, Linux_DHCPParams_Initialize( &mi, ctx ) );
