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

/* The include for common DHCP settings */
#include "sblim-dhcp.h"

/* Include the abstract resource access functions and abstracted _RESOURCES and _RESOURCE data types. */
#include "Linux_DHCPService_Resource.h"

#ifndef CMPI_VER_100
#define Linux_DHCPService_ModifyInstance Linux_DHCPService_SetInstance
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
static const char * _KEYNAMES[] = {"SystemCreationClassName", "SystemName", "CreationClassName", "Name", NULL};

// ----------------------------------------------------------------------------
// EnumInstanceNames()
// Return a list of all the instances names (return their object paths only).
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_EnumInstanceNames(
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
    _RA_STATUS ra_status;
    const char * namespace =  CMGetCharsPtr( CMGetNameSpace( reference, &status ), NULL );
    int found = 0;

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing enumerate instance names\n",__FUNCTION__);
#endif
    if ( !isEnumerateInstanceNamesSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPService_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPService_getNextResource( resources, &resource );
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
        ra_status = Linux_DHCPService_setInstanceFromResource( resource, instance, _BROKER );
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status );
            goto clean_on_error; 
        }

        /* Free the resource data. */
        ra_status = Linux_DHCPService_freeResource( resource );
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
        ra_status = Linux_DHCPService_getNextResource( resources, &resource );
    }

    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone( results );
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPService_freeResource( resource );
    ra_status = Linux_DHCPService_freeResources( resources );

exit:

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing enumerate instance names - completed\n",__FUNCTION__);
#endif
    return status;
}

// ----------------------------------------------------------------------------
// EnumInstances()
// Return a list of all the instances (return all the instance data).
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_EnumInstances(
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
    _RA_STATUS ra_status;
    const char * namespace = CMGetCharsPtr( CMGetNameSpace( reference, NULL ), NULL );
    int found = 0;

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing enumerate instances\n",__FUNCTION__);
#endif

    if ( !isEnumerateInstancesSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPService_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Enumerate thru the list of system resources and return a CMPIInstance for each. */
    ra_status = Linux_DHCPService_getNextResource( resources, &resource );
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
        ra_status = Linux_DHCPService_setInstanceFromResource( resource, instance, _BROKER );
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status );
            goto clean_on_error; 
        }

        /* Free the resource data. */
        ra_status = Linux_DHCPService_freeResource( resource );
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
            goto clean_on_error; 
        }

        /* Return the CMPIInstance for this instance. */
        CMReturnInstance(results, instance);
        found++;
        ra_status = Linux_DHCPService_getNextResource( resources, &resource );
    }

    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnDone( results );
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPService_freeResource( resource );
    ra_status = Linux_DHCPService_freeResources( resources );

exit:

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing enumerate instances - completed\n",__FUNCTION__);
#endif
    return status;
}

// ----------------------------------------------------------------------------
// GetInstance()
// Return the instance data for the specified instance only.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_GetInstance(
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
    _RA_STATUS ra_status;
    const char * namespace =  CMGetCharsPtr(CMGetNameSpace(reference, NULL), NULL);

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing get instance\n",__FUNCTION__);
#endif


    if ( !isGetSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPService_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPService_getResourceForObjectPath( resources, &resource, reference );
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
    ra_status = Linux_DHCPService_setInstanceFromResource( resource, instance, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property values from resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free the resource data. */
    ra_status = Linux_DHCPService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    /* Return the CMPIInstance for this instance. */
    CMReturnInstance( results, instance );

    CMReturnDone( results );
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPService_freeResource( resource );
    ra_status = Linux_DHCPService_freeResources( resources );

exit:

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing get instance - completed\n",__FUNCTION__);
#endif
    return status;
}

// ----------------------------------------------------------------------------
// ModifyInstance()
// Save modified instance data for the specified instance.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_ModifyInstance(
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
    _RA_STATUS ra_status;

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing modify instance\n",__FUNCTION__);
#endif

    if ( !isModifySupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPService_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPService_getResourceForObjectPath( resources, &resource, reference );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;

    } else if ( !resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("Target instance not found") );
        goto clean_on_error;
    }

    /* Update the target resource data with the new instance property values. */
    ra_status = Linux_DHCPService_setResourceFromInstance( &resource, newinstance, properties, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to modify resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free the resource data. */
    ra_status = Linux_DHCPService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }
    goto exit;

clean_on_error:

    ra_status = Linux_DHCPService_freeResource( resource );
    ra_status = Linux_DHCPService_freeResources( resources );

exit:

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing modify instance - completed\n",__FUNCTION__);
#endif
    return status;
}

// ----------------------------------------------------------------------------
// CreateInstance()
// Create a new instance from the specified instance data.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_CreateInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace, classname and object path. */
            const CMPIInstance * newinstance)    /* [in] Contains the new instance data. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status;
    CMPIObjectPath * objectpath = NULL;

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing create instance\n",__FUNCTION__);
#endif

    if ( !isCreateSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    ra_status = Linux_DHCPService_getObjectPathForInstance( &objectpath, newinstance );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get the object path for the new instance"), ra_status );
        goto exit;
    }
    
    /* Get a handle to the list of resources. */
    ra_status = Linux_DHCPService_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Verification if target instance already exists */
    ra_status = Linux_DHCPService_getResourceForObjectPath( resources, &resource, objectpath );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;

    } else if ( resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_ALREADY_EXISTS, _("The instance already exists") );
        goto clean_on_error;
    }

    /* Create the resource from the instance */
    ra_status = Linux_DHCPService_createResourceFromInstance( resources, &resource, newinstance, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to create the new resource"), ra_status );
        goto clean_on_error;
    }

    /* Free the resource data. */
    ra_status = Linux_DHCPService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error;
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }

    CMReturnObjectPath(results, objectpath);
    CMReturnDone(results);
    goto exit;
    
clean_on_error:

    ra_status = Linux_DHCPService_freeResource( resource );
    ra_status = Linux_DHCPService_freeResources( resources );

exit:

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing create instance - completed\n",__FUNCTION__);
#endif

    return status;
}

// ----------------------------------------------------------------------------
// DeleteInstance()
// Delete or remove the specified instance from the system.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_DeleteInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference)  	 /* [in] Contains the target namespace, classname and object path. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RESOURCES * resources = NULL;
    _RESOURCE * resource = NULL;
    _RA_STATUS ra_status;

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing delete instance\n",__FUNCTION__);
#endif

    if ( !isDeleteSupported() ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );
        goto exit;
    }

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPService_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources"), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPService_getResourceForObjectPath( resources, &resource, reference );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data"), ra_status );
        goto clean_on_error;

    } else if ( !resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("Target instance not found") );
        goto clean_on_error;
    }

    /* Delete the target resource. */
    ra_status = Linux_DHCPService_deleteResource( resources, resource, _BROKER );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to delete resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free the resource data. */
    ra_status = Linux_DHCPService_freeResource( resource );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free resource data"), ra_status );
        goto clean_on_error; 
    }

    /* Free list of system resources */
    ra_status = Linux_DHCPService_freeResources( resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to free list of system resources"), ra_status );
        goto clean_on_error;
    }


clean_on_error:

    ra_status = Linux_DHCPService_freeResource( resource );
    ra_status = Linux_DHCPService_freeResources( resources );

exit:

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing delete instance - completed\n",__FUNCTION__);
#endif

    return status;
}


// ----------------------------------------------------------------------------
// ExecQuery()
// Return a list of all the instances that satisfy the specified query filter.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_ExecQuery(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace and classname. */
            const char * language,               /* [in] Name of the query language. */
            const char * query)                  /* [in] Text of the query written in the query language. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing exec query\n",__FUNCTION__);
#endif

    /* EXECQUERY() IS NOT YET SUPPORTED FOR THIS CLASS */
    CMSetStatus( &status, CMPI_RC_ERR_NOT_SUPPORTED );

    CMReturnDone( results );

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing exec query - completed\n",__FUNCTION__);
#endif

    return status;
}

// ----------------------------------------------------------------------------
// Initialize()
// Perform any necessary initialization immediately after this provider is
// first loaded.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_InstanceInitialize(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context)         /* [in] Additional context info, if any. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing instance provider initialization\n",__FUNCTION__);
#endif

    /* Initialize method provider */
    ra_status = Linux_DHCPService_InstanceProviderInitialize();
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to initialize instance provider"), ra_status );
    }

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing instance provider initialization - completed\n",__FUNCTION__);
#endif

    return status;
}

// ----------------------------------------------------------------------------
// Cleanup()
// Perform any necessary cleanup immediately before this provider is unloaded.
// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPService_Cleanup(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            CMPIBoolean terminating) 
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    bool lTerminating = false;

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing instance provider cleanup\n",__FUNCTION__);
#endif

    if (terminating) {
        lTerminating = true;
    }

    /* Cleanup method provider */
    ra_status = Linux_DHCPService_InstanceProviderCleanUp( lTerminating );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to cleanup instance provider"), ra_status );
    }

#ifdef DEBUG
    fprintf(stderr,"%s -- Processing instance provider cleanup - completed\n",__FUNCTION__);
#endif

    return status;
}

// ============================================================================
// CMPI INSTANCE PROVIDER FUNCTION TABLE SETUP
// ============================================================================
CMInstanceMIStub( Linux_DHCPService_ , Linux_DHCPServiceProvider, _BROKER, Linux_DHCPService_InstanceInitialize( &mi, ctx ) );


// ----------------------------------------------------------------------------
// InvokeMethod()
// Executes the specified extrinsic method
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_InvokeMethod(
            CMPIMethodMI * self,                 /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace and classname. */
            const char* methodName,              /* [in] The name of the method to execute. */
            const CMPIArgs* inDataArray,         /* [in] The input values for the method. */
            CMPIArgs* outDataArray)              /* [out] The output values for the method. */
{
    CMPIStatus  status = {CMPI_RC_OK, NULL};
    CMPIString* class = NULL;
    _RESOURCES* resources = NULL;
    _RESOURCE*  resource = NULL;
    _RA_STATUS  ra_status = {RA_RC_OK, 0, NULL};
    
//#ifdef DEBUG
    //ffprintf(stderr,stderr,"%s -- Processing invoke method\n",__FUNCTION__);
//#endif

    /* Get a handle to the list of system resources. */
    ra_status = Linux_DHCPService_getResources( &resources );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get list of system resources."), ra_status );
        goto exit;
    }

    /* Get the target resource. */
    ra_status = Linux_DHCPService_getResourceForObjectPath( resources, &resource, reference );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to get resource data."), ra_status );
        goto clean_on_error;

    } else if ( !resource ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("Target instance was not found.") );
        goto clean_on_error;
    }

    class = CMGetClassName(reference, &status);
    if ( !class || strcasecmp((char*) CMGetCharsPtr(class, NULL), _CLASSNAME) ) {
        build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("The submitted reference contains an invalid class.") );
        goto clean_on_error;
    }
    
    if( !strcasecmp("StartService",methodName) ) {
    
    //
    //  uint32 StartService()
    //  
//#ifdef DEBUG
    //ffprintf(stderr,stderr,"%s -- Processing invoke method : %s\n",__FUNCTION__, methodName);
//#endif
        int inArgCount = CMGetArgCount(inDataArray, &status);
//#ifdef DEBUG
//        fprintf(stderr,"%s --- Count of input parameter = %d\n",__FUNCTION__, inArgCount);
//#endif
        if ( inArgCount != 0 ) {
             build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_INVALID_PARAMETER, _("Incorrect number of input arguments for method \"SartService\".") );
//#ifdef DEBUG
    //        ffprintf(stderr,stderr,"%s -- Processing invoke method : %s - completed with error\n",__FUNCTION__, methodName);
//#endif
            goto clean_on_error;
        }
        
        unsigned int methodResult=0;
        ra_status = Linux_DHCPService_Method_StartService(&methodResult, resource);
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Execution of method \"StartService\" failed."), ra_status );
//#ifdef DEBUG
      //      ffprintf(stderr,stderr,"%s -- Processing invoke method : %s - completed with error\n",__FUNCTION__, methodName);
//#endif
            goto clean_on_error;
        }
        CMReturnData ( results, (CMPIValue *)&methodResult, CMPI_uint32 );

//#ifdef DEBUG
    //ffprintf(stderr,stderr,"%s -- Processing invoke method : %s - completed\n",__FUNCTION__, methodName);
//#endif

    } else if( !strcasecmp("StopService",methodName) ) {
    //
    //  uint32 StopService()
    //
//#ifdef DEBUG
    //ffprintf(stderr,stderr,"%s -- Processing invoke method : %s\n",__FUNCTION__, methodName);
//#endif
        int inArgCount = CMGetArgCount(inDataArray, &status);
//#ifdef DEBUG
  //      fprintf(stderr,"%s --- Count of input parameter = %d\n",__FUNCTION__, inArgCount);
//#endif
        if ( inArgCount != 0 ) {
             build_cmpi_error_msg ( _BROKER, &status, CMPI_RC_ERR_INVALID_PARAMETER, _("Incorrect number of input arguments for method \"SopService\".") );
//#ifdef DEBUG
      //      ffprintf(stderr,stderr,"%s -- Processing invoke method : %s - completed with error\n",__FUNCTION__, methodName);
//#endif
            goto clean_on_error;
        }
        
        unsigned int methodResult=0;
        ra_status = Linux_DHCPService_Method_StopService(&methodResult, resource);
        if ( ra_status.rc != RA_RC_OK ) {
            build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Execution of method \"StopService\" failed."), ra_status );
//#ifdef DEBUG
        //    ffprintf(stderr,stderr,"%s -- Processing invoke method : %s - completed with error\n",__FUNCTION__, methodName);
//#endif
            goto clean_on_error;
        }
        CMReturnData ( results, (CMPIValue *)&methodResult, CMPI_uint32 );

//#ifdef DEBUG
    //ffprintf(stderr,stderr,"%s -- Processing invoke method : %s - completed\n",__FUNCTION__, methodName);
//#endif
    } else {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_METHOD_NOT_FOUND, _("This function is not known by this provider."));
        
    }

    CMReturnDone( results );

clean_on_error:
    ra_status = Linux_DHCPService_freeResource( resource );
    ra_status = Linux_DHCPService_freeResources( resources );

exit:

//#ifdef DEBUG
    //ffprintf(stderr,stderr,"%s -- Processing invoke method - completed\n",__FUNCTION__);
//#endif
    return status;
}

// ----------------------------------------------------------------------------
// MethodInitialize()
// Perform any necessary initialization immediately after this provider is
// first loaded.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPService_MethodInitialize(
            CMPIMethodMI * self,                 /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context)         /* [in] Additional context info, if any. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};

    /* Initialize method provider */
    ra_status = Linux_DHCPService_MethodProviderInitialize(&ra_status);
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to initialize method provider"), ra_status );
    }
    return status;
}

// ----------------------------------------------------------------------------
// Cleanup()
// Perform any necessary cleanup immediately before this provider is unloaded.
// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPService_MethodCleanup(
            CMPIMethodMI * self,                 /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * ctx,             /* [in] Additional context info, if any. */
            CMPIBoolean terminating)             /* [in] Switch to wether to teminate or not*/ 
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    _RA_STATUS ra_status = {RA_RC_OK, 0, NULL};
    bool lTerminating = false;

    if (terminating) {
        lTerminating = true;
    }

    /* Cleanup method provider */
    ra_status = Linux_DHCPService_MethodProviderCleanUp( lTerminating );
    if ( ra_status.rc != RA_RC_OK ) {
        build_ra_error_msg ( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to cleanup method provider"), ra_status );
    }

    return status;
}

// ============================================================================
// CMPI METHOD PROVIDER FUNCTION TABLE SETUP
// ============================================================================
CMMethodMIStub( Linux_DHCPService_ , Linux_DHCPServiceProvider, _BROKER, Linux_DHCPService_MethodInitialize( &mi, ctx ) );

