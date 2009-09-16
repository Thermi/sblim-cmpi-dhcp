// ============================================================================
// Copyright © 2007, International Business Machines
//
// THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
//
// You can obtain a current copy of the Eclipse Public License from
// http://www.opensource.org/licenses/eclipse-1.0.php
//
// Authors:             Sam Kannan <samkannan (at) us.ibm.com>
// ============================================================================

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Include the required CMPI data types, function headers, and macros */
#include "cmpidt.h"
#include "cmpift.h"
#include "cmpimacs.h"

#include "sblim-dhcp.h"
#include "Linux_ElementConformsToProfile_Resource.h"

// ----------------------------------------------------------------------------
// COMMON GLOBAL VARIABLES
// ----------------------------------------------------------------------------

/* Handle to the CIM broker. Initialized when the provider lib is loaded. */
static const CMPIBroker *_BROKER;

// ----------------------------------------------------------------------------
// EnumInstanceNames()
// Return a list of all the instances names (return their object paths only).
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_EnumInstanceNames(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference)    /* [in] Contains target namespace and classname. */
{
	
    CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance *instance = NULL;
    
    //( const CMPIBroker *broker, const CMPIContext *context, const CMPIObjectPath *reference, const char **properties, CMPIInstance **instance);
    status = Linux_ElementConformsToProfile_GetInstance(_BROKER, context, reference, NULL, &instance);

    /* Return the CMPIObjectPath for this instance. */
    CMPIObjectPath * objectpath = CMGetObjectPath(instance, &status);
    if ( (status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath) ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Failed to get CMPIObjectPath from CMPIInstance") );
        goto exit;
    }
         
    CMReturnObjectPath( results, objectpath );

    CMReturnDone( results );

exit:
  	//free(resource); // update this call
	// release instance ????
    return status;
}



// ----------------------------------------------------------------------------
// EnumInstances()
// Return a list of all the instances (return all the instance data).
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_EnumInstances(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains target namespace and classname. */
            const char ** properties)            /* [in] List of desired properties (NULL=all). */
{

	CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance * instance = NULL;
    
    status = Linux_ElementConformsToProfile_GetInstance(_BROKER, context, reference, properties, &instance);    
    if ( (status.rc != CMPI_RC_OK) ) {
        goto exit;
    }
    CMReturnInstance( results, instance );
    
    CMReturnDone( results );
    
    exit: 
    	return status;
}


// ----------------------------------------------------------------------------
// GetInstance()
// Return the instance data for the specified instance only.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_GetInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace, classname and object path. */
            const char ** properties)            /* [in] List of desired properties (NULL=all). */
{
	CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance *instance = NULL;
    CMPIObjectPath *instanceOP = NULL;
    
    status = Linux_ElementConformsToProfile_GetInstance(_BROKER, context, reference, properties, &instance);    
    if ( (status.rc != CMPI_RC_OK) ) {    
		CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Failed to get objectpath from instance") );
    	goto exit;
    }

	instanceOP = CMGetObjectPath(instance, &status);
	if ((status.rc != CMPI_RC_OK) || (instanceOP == NULL)) {
		CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Failed to get objectpath from instance") );
        goto exit;
	}

    // compare the objectpaths
	if (!Linux_ElementConformsToProfile_EqualsObjectPath( _BROKER, reference, instanceOP)) {		
		CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_FOUND, _("Target instance not found")  );
		goto exit;		
	} 
    	
    CMReturnInstance( results, instance );
    CMReturnDone( results );
    exit:
		//free(resource); // update this call
    	return status;
}

// ----------------------------------------------------------------------------
// ModifyInstance()
// Save modified instance data for the specified instance.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_ModifyInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace, classname and object path. */
            const CMPIInstance * newinstance,    /* [in] Contains the new instance data. */
            const char** properties) {
    
	CMPIStatus status = {CMPI_RC_OK, NULL};

    /* ModifyInstance() IS NOT SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") ); 

    return status;
}

// ----------------------------------------------------------------------------
// CreateInstance()
// Create a new instance from the specified instance data.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_CreateInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace, classname and object path. */
            const CMPIInstance * newinstance)    /* [in] Contains the new instance data. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};
	
    /* CreateInstance() IS NOT SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") ); 

    return status;
}

// ----------------------------------------------------------------------------
// DeleteInstance()
// Delete or remove the specified instance from the system.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_DeleteInstance(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference)    /* [in] Contains the target namespace, classname and object path. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

    /* DeleteInstance() IS NOT SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") ); 

    return status;
}


// ----------------------------------------------------------------------------
// ExecQuery()
// Return a list of all the instances that satisfy the specified query filter.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_ExecQuery(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            const CMPIResult * results,          /* [out] Results of this operation. */
            const CMPIObjectPath * reference,    /* [in] Contains the target namespace and classname. */
            const char * language,               /* [in] Name of the query language. */
            const char * query)                  /* [in] Text of the query written in the query language. */
{
    CMPIStatus status = {CMPI_RC_OK, NULL};

    /* EXECQUERY() IS NOT SUPPORTED FOR THIS CLASS */
    CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERR_NOT_SUPPORTED, _("This function is not supported") );

    CMReturnDone(results);

    return status;
}


// ----------------------------------------------------------------------------
// Initialize()
// Perform any necessary initialization immediately after this provider is
// first loaded.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_Initialize(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context)         /* [in] Additional context info, if any. */
{
    CMPIStatus status = { CMPI_RC_OK, NULL };

    return status;
}

// ----------------------------------------------------------------------------
// Initialize()
// Perform any necessary initialization immediately after this provider is
// first loaded.
// ----------------------------------------------------------------------------
CMPIStatus Linux_DHCPElementConformsToProfile_AssociationInitialize(
         CMPIAssociationMI * self,  /** [in] Handle to this provider (i.e. 'self'). */		
         const CMPIContext * context) {		/** [in] Additional context info, if any. */

	CMPIStatus status = { CMPI_RC_OK, NULL };
    return status;
}

// ----------------------------------------------------------------------------
// Cleanup()
// Perform any necessary cleanup immediately before this provider is unloaded.
// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPElementConformsToProfile_Cleanup(
            CMPIInstanceMI * self,               /* [in] Handle to this provider (i.e. 'self'). */
            const CMPIContext * context,         /* [in] Additional context info, if any. */
            CMPIBoolean terminating)
{
	CMPIStatus status = { CMPI_RC_OK, NULL };

	return status;
}



/// ----------------------------------------------------------------------------
/// AssociationCleanup()
/// Perform any necessary cleanup immediately before this provider is unloaded.
/// ----------------------------------------------------------------------------
static CMPIStatus Linux_DHCPElementConformsToProfile_AssociationCleanup(
	CMPIAssociationMI * self,	/** [in] Handle to this provider (i.e. 'self'). */
	const CMPIContext * context,		/** [in] Additional context info, if any. */
	CMPIBoolean terminating) {
	
	CMPIStatus status = { CMPI_RC_OK, NULL };	/** Return status of CIM operations. */

	return status;
}


/**
 * AssociatorNames()
 *
 * This method returns the list of DHCPService instances that are associated with DHCPRegisteredProfile 
 * (specfied objectpath) using this DHCPElementConformsToProfile association class.
 * Check if the reference objectpath = resource objectpath if it is same return
 * object path of DHCPService if it is an instance of resultClass (could be parent class)
 * role: if role is not null, ensure that the the source role = ConformantStandard/ManagedElement
 *  resultrole: if resultrole is not null, ensure that the the target role = ManagedElement/ConformantStandard
 *  the resultrole & resultclass should match the class def,
 *  eg:in DHCPElementConformsToProfile,
 *  if resultclass =  Linux_DHCPRegisteredProfile resultrole should be ConformantStandard;
 *  if resultclass =  Linux_DHCPService resultrole should be ManagedElement;
 *  else the data does not match filter & return nothing
 *  if (assocClass != NULL) check to make sure that its = Linux_DHCPElementConformsToProfile or parent 
 * 
 *  Result type is ObjectPath of ResultClass instance 
 * 
 * Arguments:
 * CMPIAssociationMI * self			-[in] Handle to this provider (i.e. 'self'). 
 * const CMPIContext * context		-[in] Additional context info, if any.
 * const CMPIResult * results,	    -[out] Results of this operation. 
 * const CMPIObjectPath * reference,-[in] Contains source namespace, classname and object path.
 * const char * assocClass,
 * const char * resultClass,
 * const char * role,
 * const char * resultRole
 * 
 * Returns:
 * 	CMPIStatus - Contains the service return status 
 *  
 */
static CMPIStatus Linux_DHCPElementConformsToProfile_AssociatorNames(
	CMPIAssociationMI * self, const CMPIContext * context, const CMPIResult * results,	    
	const CMPIObjectPath * reference, const char * assocClass, const char * resultClass,
	const char * role, const char * resultRole) {

    CMPIStatus status = {CMPI_RC_OK, NULL};
	_RESOURCEP *resource = NULL;
	
	resource = (_RESOURCEP *)malloc(sizeof(_RESOURCEP));
    if ( resource == NULL ) {
    	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Dynamic Memory Allocation Failed") );
        return status;
    }
	
    status = Linux_ElementConformsToProfile_getResource(_BROKER, context, reference, resource);        
    const char *srcClass = CMGetCharsPtr(CMGetClassName(reference, &status), NULL);
       
    if (strcmp(srcClass, _SRCCLASSNAME) == 0) {

    	if (Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->src, resource->target, 
    			role, _SRCPROPERTYNAME, resultRole, _TARGETPROPERTYNAME,resultClass)) {
    	    		
    		CMReturnObjectPath(results, resource->target);
    	}    	
    } else if (strcmp(srcClass, _TARGETCLASSNAME) ==0) {

    	if (Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->target, resource->src,
    			role, _TARGETPROPERTYNAME, resultRole, _SRCPROPERTYNAME, resultClass)) {   	    		
        	CMReturnObjectPath(results, resource->src);
    	}
    }
        
	CMReturnDone(results);
	return status;
}


/**
 * Associators()
 *
 * This method returns the list of DHCPService instances that are associated with DHCPRegisteredProfile 
 * (specfied objectpath) using this DHCPElementConformsToProfile association class.
 * Check if the reference objectpath = resource objectpath if it is same return
 * object path of DHCPService if it is an instance of resultClass (could be parent class)
 * role: if role is not null, ensure that the the source role = ConformantStandard/ManagedElement
 *  resultrole: if resultrole is not null, ensure that the the target role = ManagedElement/ConformantStandard
 *  the resultrole & resultclass should match the class def,
 *  eg:in DHCPElementConformsToProfile,
 *  if resultclass =  Linux_DHCPRegisteredProfile resultrole should be ConformantStandard;
 *  if resultclass =  Linux_DHCPService resultrole should be ManagedElement;
 *  else the data does not match filter & return nothing
 *  if (assocClass != NULL) check to make sure that its = Linux_DHCPElementConformsToProfile or parent 
 *  
 * Result type is Instance of ResultClass
 * 
 * Arguments:
 * CMPIAssociationMI * self			-[in] Handle to this provider (i.e. 'self'). 
 * const CMPIContext * context		-[in] Additional context info, if any.
 * const CMPIResult * results,	    -[out] Results of this operation. 
 * const CMPIObjectPath * reference,-[in] Contains source namespace, classname and object path.
 * const char * assocClass,
 * const char * resultClass,
 * const char * role,
 * const char * resultRole
 * 
 * Returns:
 * 	CMPIStatus - Contains the service return status 
 *  
 */
static CMPIStatus Linux_DHCPElementConformsToProfile_Associators(
	CMPIAssociationMI * self, const CMPIContext * context,
	const CMPIResult * results,	 const CMPIObjectPath * reference,
	const char *assocClass, const char *resultClass, const char *role,
	const char *resultRole, const char ** properties) {

	CMPIStatus status = {CMPI_RC_OK, NULL};
	CMPIInstance *instance = NULL;
	_RESOURCEP *resource = NULL;

	resource = (_RESOURCEP *)malloc(sizeof(_RESOURCEP));
    if ( resource == NULL ) {
    	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Dynamic Memory Allocation Failed") );
        return status;
    }

    status = Linux_ElementConformsToProfile_getResource(_BROKER, context, reference, resource);        
    const char *srcClass = CMGetCharsPtr(CMGetClassName(reference, &status), NULL);
    
    if (strcmp(srcClass, _SRCCLASSNAME) ==0) {
    	
    	if (Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->src, resource->target,
    			role, _SRCPROPERTYNAME, resultRole, _TARGETPROPERTYNAME,resultClass)) {
    	    		
    		instance = CBGetInstance(_BROKER, context, resource->target, NULL, &status);
    		if ((CMIsNullObject(instance)) || (status.rc != CMPI_RC_OK)) {
    	    	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, 
    	    			_("Associator call: Failed to get Instance") );
    			goto exit;
    		}
   
    		CMReturnInstance(results, instance);

    	}
    	
    } else if (strcmp(srcClass, _TARGETCLASSNAME) ==0) {

    	if (Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->target, resource->src,
    			role, _TARGETPROPERTYNAME, resultRole, _SRCPROPERTYNAME, resultClass)) {
    	    		
    		instance = CBGetInstance(_BROKER, context, resource->src, NULL, &status);
    		if ((CMIsNullObject(instance)) || (status.rc != CMPI_RC_OK)) {
    			CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, 
    					_("Associator call: Failed to get Instance") );
    			goto exit;
    		}
    		CMReturnInstance(results, instance);
    	}
    }
        
    CMReturnDone(results);
	exit:		
		return status;
}


/**
 * ReferenceNames()
 *
 * This method returns the list of DHCPService instances that are associated with DHCPRegisteredProfile 
 * (specfied objectpath) using this DHCPElementConformsToProfile association class.
 * Check if the reference objectpath = resource objectpath if it is same return
 * object path of DHCPService if it is an instance of resultClass (could be parent class)
 * role: if role is not null, ensure that the the source role = ConformantStandard/ManagedElement
 *  resultrole: if resultrole is not null, ensure that the the target role = ManagedElement/ConformantStandard
 *  the resultrole & resultclass should match the class def,
 *  eg:in DHCPElementConformsToProfile,
 *  if resultclass =  Linux_DHCPRegisteredProfile resultrole should be ConformantStandard;
 *  if resultclass =  Linux_DHCPService resultrole should be ManagedElement;
 *  else the data does not match filter & return nothing
 *  if (assocClass != NULL) check to make sure that its = Linux_DHCPElementConformsToProfile or parent
 *  
 * Result type is ObjectPath of DHCPElementConformsToProfile instance 
 *  
 * Arguments:
 * CMPIAssociationMI * self			-[in] Handle to this provider (i.e. 'self'). 
 * const CMPIContext * context		-[in] Additional context info, if any.
 * const CMPIResult * results,	    -[out] Results of this operation. 
 * const CMPIObjectPath * reference,-[in] Contains source namespace, classname and object path.
 * const char * assocClass,
 * const char * resultClass,
 * const char * role,
 * const char * resultRole
 * 
 * Returns:
 * 	CMPIStatus - Contains the service return status 
 *  
 */
static CMPIStatus Linux_DHCPElementConformsToProfile_ReferenceNames(
		CMPIAssociationMI * self, const CMPIContext * context, const CMPIResult * results, const CMPIObjectPath * reference,
		const char *resultClass, const char *role) {
	
	CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance *instance = NULL;
	_RESOURCEP *resource = NULL;

	resource = (_RESOURCEP *)malloc(sizeof(_RESOURCEP));
    if ( resource == NULL ) {
    	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Dynamic Memory Allocation Failed") );
		goto exit;
    }
       
	status = Linux_ElementConformsToProfile_getResource(_BROKER, context, reference, resource);        
	if ( (status.rc != CMPI_RC_OK) ) {
		CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("failed to get resource") );
		goto exit;
	}

	status  = Linux_ElementConformsToProfile_GetInstanceFromResource(_BROKER, context, reference, 
			NULL, resource, &instance);	
	if ( (status.rc != CMPI_RC_OK) ) {
	   	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("failed to get the reference") );
		goto exit;
	}

    CMPIObjectPath * objectpath = CMGetObjectPath(instance, &status);
    if ( (status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath) ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Failed to get CMPIObjectPath from CMPIInstance") );
        goto exit;
    }		

    if ( Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->src, objectpath,
			role, _SRCPROPERTYNAME, NULL, _TARGETPROPERTYNAME, resultClass) 
			||
		Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->target, objectpath,
					 role, _TARGETPROPERTYNAME, NULL, _SRCPROPERTYNAME, resultClass) ) {

    	CMReturnObjectPath(results, objectpath);		
	} else {
		printf("No data to return \n");
	}
	 
	CMReturnDone(results);
	exit:
		return status;	
}

/**
 * References()
 *
 * This method returns the list of DHCPService instances that are associated with DHCPRegisteredProfile 
 * (specfied objectpath) using this DHCPElementConformsToProfile association class.
 * Check if the reference objectpath = resource objectpath if it is same return
 * object path of DHCPService if it is an instance of resultClass (could be parent class)
 * role: if role is not null, ensure that the the source role = ConformantStandard/ManagedElement
 * resultrole: if resultrole is not null, ensure that the the target role = ManagedElement/ConformantStandard
 * the resultrole & resultclass should match the class def,
 * eg:in DHCPElementConformsToProfile,
 * if resultclass =  Linux_DHCPRegisteredProfile resultrole should be ConformantStandard;
 * if resultclass =  Linux_DHCPService resultrole should be ManagedElement;
 * else the data does not match filter & return nothing
 * 
 * Result type is DHCPElementConformsToProfile 
 *  
 * Arguments: 
 * CMPIAssociationMI * self			-[in] Handle to this provider (i.e. 'self'). 
 * const CMPIContext * context		-[in] Additional context info, if any.
 * const CMPIResult * results,	    -[out] Results of this operation. 
 * const CMPIObjectPath * reference,-[in] Contains source namespace, classname and object path.
 * const char * resultClass,
 * const char * role,
 * const char **properties
 * 
 * Returns:
 * 	CMPIStatus - Contains the service return status 
 *  
 */
static CMPIStatus Linux_DHCPElementConformsToProfile_References(
	CMPIAssociationMI * self, const CMPIContext * context, const CMPIResult * results, const CMPIObjectPath * reference,
	const char *resultClass, const char *role, const char **properties)	{
	
	CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIInstance *instance = NULL;
	_RESOURCEP *resource = NULL;

	resource = (_RESOURCEP *)malloc(sizeof(_RESOURCEP));
    if ( resource == NULL ) {
    	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Dynamic Memory Allocation Failed") );
    	goto exit;
    }

	status = Linux_ElementConformsToProfile_getResource(_BROKER, context, reference, resource);        
	if ( (status.rc != CMPI_RC_OK) ) {
		CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("failed to get resource") );
		goto exit;
	}

	status  = Linux_ElementConformsToProfile_GetInstanceFromResource(_BROKER, context, reference, 
			NULL, resource, &instance);	
	if ( (status.rc != CMPI_RC_OK) ) {
	   	CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("failed to get the reference") );
		goto exit;
	}

	CMPIObjectPath * objectpath = CMGetObjectPath(instance, &status);
    if ( (status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath) ) {
        CMSetStatusWithChars( _BROKER, &status, CMPI_RC_ERROR, _("Failed to get CMPIObjectPath from CMPIInstance") );
        goto exit;
    }
	
	if ( Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->src, objectpath,
			role, _SRCPROPERTYNAME, NULL, _TARGETPROPERTYNAME, resultClass) 
			||
		Linux_ElementConformsToProfile_Validate(_BROKER, reference, resource->target, objectpath,
					 role, _TARGETPROPERTYNAME, NULL, _SRCPROPERTYNAME, resultClass) ) {
		
		CMReturnInstance( results, instance );
	}else {
			printf("No data to return \n");
	}
		
	CMReturnDone(results);
	exit:
		return status;
}


// ============================================================================
// CMPI INSTANCE PROVIDER FUNCTION TABLE SETUP
// ============================================================================
CMInstanceMIStub(Linux_DHCPElementConformsToProfile_, Linux_DHCPElementConformsToProfileProvider, _BROKER, Linux_DHCPElementConformsToProfile_Initialize(&mi, ctx));
CMAssociationMIStub(Linux_DHCPElementConformsToProfile_, Linux_DHCPElementConformsToProfileProvider, _BROKER, Linux_DHCPElementConformsToProfile_AssociationInitialize(&mi, ctx));
