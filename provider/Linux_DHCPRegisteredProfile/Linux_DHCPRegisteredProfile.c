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
#include <string.h>
#include <stdlib.h>

/** Include the required CMPI data types, function headers, and macros */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

/** The include for common MetaCluster settings */
#include "sblim-dhcp.h"

#ifndef CMPI_VER_100
#define Linux_DHCPRegisteredProfile_ModifyInstance Linux_DHCPRegisteredProfile_SetInstance
#endif

/// ----------------------------------------------------------------------------
/// COMMON GLOBAL VARIABLES
/// ----------------------------------------------------------------------------

/** Handle to the CIM broker. Initialized when the provider lib is loaded. */
static const CMPIBroker *_BROKER;

/**** CUSTOMIZE FOR EACH PROVIDER ***/
static const char * _CLASSNAME = "Linux_DHCPRegisteredProfile";

/// ----------------------------------------------------------------------------
/// Info for the class supported by the instance provider
/// ----------------------------------------------------------------------------

/**** CUSTOMIZE FOR EACH PROVIDER ***/
/** NULL terminated list of key properties of this class. */
const static char * _KEYNAMES[] = {"InstanceID", NULL};


///----------------------------------------------------------------------------
/// Helper function to set the instance properties
///----------------------------------------------------------------------------
static void setInstanceData(const CMPIBroker * broker, const CMPIInstance * instance) {
    /** Set the CMPIInstance properties from the resource data. */
    CMSetProperty(instance, "RegisteredName",(CMPIValue *)PROFILE_REGISTERED_NAME, CMPI_chars);
    CMSetProperty(instance, "RegisteredVersion",(CMPIValue *)PROFILE_REGISTERED_VERSION, CMPI_chars);

    unsigned int lAdvertiseType = PROFILE_ADVERTISE_TYPE;
    CMPIArray * advertisetypes = CMNewArray(broker, 1, CMPI_uint16, NULL);
    CMSetArrayElementAt(advertisetypes, 0, (CMPIValue *)&lAdvertiseType, CMPI_uint16);
    CMSetProperty(instance, "AdvertiseTypes",(CMPIValue *)&advertisetypes, CMPI_uint16A);

    unsigned int lRegisteredOrganization = PROFILE_REGISTERED_ORGANIZATION;
    CMSetProperty(instance, "RegisteredOrganization",(CMPIValue *)&lRegisteredOrganization, CMPI_uint16);
    CMSetProperty(instance, "OtherRegisteredOrganization",(CMPIValue *)PROFILE_OTHER_REGISTERED_ORGANIZATION, CMPI_chars);

    char instanceid[1024];
    sprintf(instanceid, "%s:%s-%s", PROFILE_OTHER_REGISTERED_ORGANIZATION, PROFILE_REGISTERED_NAME, PROFILE_REGISTERED_VERSION); 
    CMSetProperty(instance, "InstanceID",(CMPIValue *)instanceid, CMPI_chars);	
};


/// ============================================================================
/// CMPI INSTANCE PROVIDER FUNCTION TABLE
/// ============================================================================


/// ----------------------------------------------------------------------------
/// EnumInstanceNames()
/// Return a list of all the instances names (return their object paths only).
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_EnumInstanceNames(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            const CMPIResult * results,          /** [out] Results of this operation. */
            const CMPIObjectPath * reference)    /** [in] Contains target namespace and classname. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    const char * lnamespace =  CMGetCharsPtr(CMGetNameSpace(reference, &status), NULL);

    /** Create a new CMPIObjectPath to store this resource. */
    op = CMNewObjectPath( _BROKER, lnamespace, _CLASSNAME, &status);
    if ( CMIsNullObject(op) ) { 
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
        goto exit; 
    }

    /** Create a new CMPIInstance to store this resource. */
    instance = CMNewInstance( _BROKER, op, &status);
    if ( CMIsNullObject(instance) ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIInstance failed") );
        goto exit; 
    }

    setInstanceData(_BROKER, instance);

    /** Return the CMPIObjectPath for this instance. */
    CMPIObjectPath * objectpath = CMGetObjectPath(instance, &status);
    if ( (status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath) ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Failed to get CMPIObjectPath from CMPIInstance") );
        goto exit;
    }
    CMSetNameSpace( objectpath, lnamespace ); /** Note - CMGetObjectPath() does not preserve the namespace! */
     
    CMReturnObjectPath( results, objectpath );

    CMReturnDone( results );

exit:
  
    return status;
}

/// ----------------------------------------------------------------------------
/// EnumInstances()
/// Return a list of all the instances (return all the instance data).
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_EnumInstances(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            const CMPIResult * results,          /** [out] Results of this operation. */
            const CMPIObjectPath * reference,    /** [in] Contains target namespace and classname. */
            const char ** properties)            /** [in] List of desired properties (NULL=all). */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    const char * lnamespace =  CMGetCharsPtr(CMGetNameSpace(reference, &status), NULL);

    /** Create a new CMPIObjectPath to store this resource. */
    op = CMNewObjectPath( _BROKER, lnamespace, _CLASSNAME, &status);
    if( CMIsNullObject(op) ) { 
      CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
      goto exit; 
    }

    /** Create a new CMPIInstance to store this resource. */
    instance = CMNewInstance( _BROKER, op, &status);
    if ( CMIsNullObject(instance) ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIInstance failed") ); 
        goto exit; 
    }

    /** Setup a filter to only return the desired properties. */
    status = CMSetPropertyFilter( instance, properties, _KEYNAMES );
    if ( status.rc != CMPI_RC_OK ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property filter") );
        goto exit;
    }
   
    setInstanceData( _BROKER, instance );
   
    CMReturnInstance( results, instance );

    CMReturnDone( results );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// GetInstance()
/// Return the instance data for the specified instance only.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_GetInstance(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            const CMPIResult * results,          /** [out] Results of this operation. */
            const CMPIObjectPath * reference,    /** [in] Contains the target namespace, classname and object path. */
            const char ** properties)            /** [in] List of desired properties (NULL=all). */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance * instance = NULL;
    CMPIObjectPath * op = NULL;
    CMPIData cmpi_info;
    const char * cmpi_name = NULL;
    const char * lnamespace = CMGetCharsPtr( CMGetNameSpace( reference, &status ), NULL );
    char registrationString[1024];

    cmpi_info = CMGetKey(reference, "InstanceID", &status);
    cmpi_name = CMGetCharsPtr(cmpi_info.value.string, NULL);

    sprintf(registrationString, "%s:%s-%s", 
	    PROFILE_OTHER_REGISTERED_ORGANIZATION, 
	    PROFILE_REGISTERED_NAME, 
	    PROFILE_REGISTERED_VERSION); 
    if(strcasecmp(registrationString, cmpi_name) != 0) {
	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Target Instance Not Found") );
	goto exit;
    }

    /** Create a new CMPIObjectPath to store this resource. */
    op = CMNewObjectPath( _BROKER, lnamespace, _CLASSNAME, &status );
    if ( CMIsNullObject( op ) ) { 
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") ); 
        goto exit; 
    }

    /** Create a new CMPIInstance to store this resource. */
    instance = CMNewInstance( _BROKER, op, &status );
    if ( CMIsNullObject( instance ) ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIInstance failed") );
        goto exit; 
    }

    /** Setup a filter to only return the desired properties. */
    status = CMSetPropertyFilter( instance, properties, _KEYNAMES );
    if ( status.rc != CMPI_RC_OK ) {
        CMSetStatusWithChars(_BROKER, &status, CMPI_RC_ERR_FAILED, _("Failed to set property filter"));
        goto exit;
    }

    setInstanceData( _BROKER, instance );
    CMReturnInstance( results, instance );
    CMReturnDone( results );

exit:
    return status;
}

/// ----------------------------------------------------------------------------
/// ModifyInstance()
/// Save modified instance data for the specified instance.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_ModifyInstance(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            const CMPIResult * results,          /** [out] Results of this operation. */
            const CMPIObjectPath * reference,    /** [in] Contains the target namespace, classname and object path. */
            const CMPIInstance * newinstance,    /** [in] Contains the new instance data. */
            const char** properties)	
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

    /** ModifyInstance() IS NOT YET SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported")); 

    return status;
}

/// ----------------------------------------------------------------------------
/// CreateInstance()
/// Create a new instance from the specified instance data.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_CreateInstance(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            const CMPIResult * results,          /** [out] Results of this operation. */
            const CMPIObjectPath * reference,    /** [in] Contains the target namespace, classname and object path. */
            const CMPIInstance * newinstance)    /** [in] Contains the new instance data. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
	
    /** CreateInstance() IS NOT YET SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported")); 

    return status;
}

/// ----------------------------------------------------------------------------
/// DeleteInstance()
/// Delete or remove the specified instance from the system.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_DeleteInstance(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            const CMPIResult * results,          /** [out] Results of this operation. */
            const CMPIObjectPath * reference)    /** [in] Contains the target namespace, classname and object path. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

    /** DeleteInstance() IS NOT YET SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported")); 

    return status;
}


/// ----------------------------------------------------------------------------
/// ExecQuery()
/// Return a list of all the instances that satisfy the specified query filter.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_ExecQuery(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            const CMPIResult * results,          /** [out] Results of this operation. */
            const CMPIObjectPath * reference,    /** [in] Contains the target namespace and classname. */
            const char * language,               /** [in] Name of the query language. */
            const char * query)                  /** [in] Text of the query written in the query language. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

    /** EXECQUERY() IS NOT YET SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported"));

    CMReturnDone(results);

    return status;
}

/// ----------------------------------------------------------------------------
/// Initialize()
/// Perform any necessary initialization immediately after this provider is
/// first loaded.
/// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPRegisteredProfile_Initialize(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context)         /** [in] Additional context info, if any. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

    return status;
}

/// ----------------------------------------------------------------------------
/// Cleanup()
/// Perform any necessary cleanup immediately before this provider is unloaded.
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPRegisteredProfile_Cleanup(
            CMPIInstanceMI * self,               /** [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /** [in] Additional context info, if any. */
            CMPIBoolean terminating)
{
	CMPIStatus status = { CMPI_RC_OK, NULL };

	return status;
}

/// ============================================================================
/// CMPI INSTANCE PROVIDER FUNCTION TABLE SETUP
/// ============================================================================
CMInstanceMIStub( Linux_DHCPRegisteredProfile_ , Linux_DHCPRegisteredProfileProvider, _BROKER, Linux_DHCPRegisteredProfile_Initialize(&mi, ctx));

