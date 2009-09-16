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
/// Authors:	Sam Kannan <samkannan (at) us.ibm.com
///
/// ============================================================================

#include <string.h>
#include <stdlib.h>

/** Include the required CMPI data types, function headers, and macros. */
#include <cmpidt.h>
#include <cmpift.h>
#include <cmpimacs.h>

/** Include the DHCP API. */
#include "sblim-dhcp.h"
#include "Linux_ElementConformsToProfile_Resource.h"

/* NULL terminated list of key properties of this class. */
const static char * _KEYNAMES[] = {_SRCPROPERTYNAME, _TARGETPROPERTYNAME, NULL};

// ----------------------------------------------------------------------------
// GetInstance()
// ----------------------------------------------------------------------------
CMPIStatus Linux_ElementConformsToProfile_GetInstance( const CMPIBroker *broker, const CMPIContext *context, const CMPIObjectPath *reference,    
            const char ** properties, CMPIInstance **instance ) {
	
	CMPIStatus status = {CMPI_RC_OK, NULL};
    _RESOURCEP *resource = NULL;
    
    resource = (_RESOURCEP *)malloc(sizeof(_RESOURCEP));
    if ( resource == NULL ) {
    	CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Dynamic Memory Allocation Failed") );
        return status;
    }
        
    status = Linux_ElementConformsToProfile_getResource(broker, context, reference, resource);

    if ( (status.rc != CMPI_RC_OK) ) {
        CMSetStatusWithChars( broker, &status, CMPI_RC_ERR_FAILED, _("Failed to load profile resource") );
        goto exit;
    }
    
    status = Linux_ElementConformsToProfile_GetInstanceFromResource(broker, context, reference, properties, resource, instance);

	exit:
		//free(resource); // update this call  
	    return status;
}

// ----------------------------------------------------------------------------
// GetInstance()
// ----------------------------------------------------------------------------
CMPIStatus Linux_ElementConformsToProfile_GetInstanceFromResource( const CMPIBroker *broker, const CMPIContext *context, 
		const CMPIObjectPath *reference, const char ** properties, _RESOURCEP *resource, CMPIInstance **instance ) {
	
	CMPIStatus status = {CMPI_RC_OK, NULL};
    CMPIObjectPath *op = NULL;
    
    const char * lnamespace =  CMGetCharsPtr(CMGetNameSpace(reference, &status), NULL);
   
    /* Create a new CMPIObjectPath to store this resource. */
    op = CMNewObjectPath( broker, lnamespace, _CLASSNAME, &status);
    if ( CMIsNullObject(op) ) { 
        CMSetStatusWithChars( broker, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIObjectPath failed") );
        goto exit; 
    }

    /* Create a new CMPIInstance to store this resource. */
    *instance = CMNewInstance( broker, op, &status);
    if ( CMIsNullObject(*instance) ) {
        CMSetStatusWithChars( broker, &status, CMPI_RC_ERR_FAILED, _("Creation of CMPIInstance failed") );
        goto exit; 
    }

    if (properties != NULL) {
    	/** Setup a filter to only return the desired properties.*/ 
		status = CMSetPropertyFilter(*instance, properties, _KEYNAMES);
		if (status.rc != CMPI_RC_OK) {
			CMSetStatusWithChars( broker, &status, CMPI_RC_ERR_FAILED, _("Cannot set property filter"));
		    goto exit;
		}	
    }
	
	Linux_ElementConformsToProfile_SetInstanceData(broker, *instance, resource);
   
    /* Return the CMPIObjectPath for this instance. */
    CMPIObjectPath * objectpath = CMGetObjectPath(*instance, &status);
    if ( (status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath) ) {
        CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Failed to get CMPIObjectPath from CMPIInstance") );
        goto exit;
    }
    CMSetNameSpace( objectpath, lnamespace ); /* Note - CMGetObjectPath() does not preserve the namespace! */

	exit:
		//free(resource); // update this call  
	    return status;
}

/* 
 * 	Query CIMOM for enum of Linux_DHCPRegisteredProfile & Linux_DHCPService
 * 	Instantiate Linux_DHCPElementConformsToProfile & populate the references with 
 * 		the correct src & target ref objects.
 * 	Validation: There should essentially be only one instance of each object for this implementation
 * 		If more than one instance of each object type exist create association object with first element 
 * 		in enum for this impl
 * 		(alternate implementation would be to throw an exception ... )	   
 * 	Samkannan:
 *  	Query List of objects : DHCPRegisteredProfile & DHCPService using: 
 *
 *  	// Create the object path for the object that has to be queried  
 *  	CMPIObjectPath * objectpath = CMNewObjectPath(broker, namespace, CLASSNAME, &status);
 *
 * 		// Api to get enumeration of all objects in CIMOM for specifed object path 
 *  	CMPIEnumeration * lhs_ops = CBEnumInstanceNames(broker, context, objectpath, &status);
 */
CMPIStatus Linux_ElementConformsToProfile_getResource 
	( const CMPIBroker *broker, const CMPIContext *context, const CMPIObjectPath *reference, _RESOURCEP *resource) {
	
    CMPIStatus status = {CMPI_RC_OK, NULL};
    	       
    CMPIObjectPath * objectpath = CMNewObjectPath(broker, _SOURCECLASSNS, _SRCCLASSNAME, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath)) {
    	CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Object Path is NULL") );        
        return status;
    }
   
    CMPIEnumeration *srcOPS = CBEnumInstanceNames(broker, context, objectpath, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(srcOPS)) {
    	CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Object Path is NULL") );        
        return status;
    }

   	if (CMHasNext(srcOPS, NULL)) {
   		CMPIData srcData = CMGetNext(srcOPS, NULL);
    	resource->src = srcData.value.ref;
    } else {
    	CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Object Path is NULL") );
    	return status;
    }
    
   	// Look up Service class from root/cimv2 name space
   	CMPIObjectPath *tobjectpath = CMNewObjectPath(broker, _TARGETCLASSNS, _TARGETCLASSNAME, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(objectpath)) {
    	CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Object Path is NULL") );
    	return status;
    }
    
    CMPIEnumeration *targetOPS = CBEnumInstanceNames(broker, context, tobjectpath, &status);
    if ((status.rc != CMPI_RC_OK) || CMIsNullObject(srcOPS)) {
    	CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Object Path is NULL") );        
        return status;
    }

   	if (CMHasNext(targetOPS, NULL)) {
   		CMPIData targetData = CMGetNext(targetOPS, NULL);
    	resource->target = targetData.value.ref;
    } else {
    	CMSetStatusWithChars( broker, &status, CMPI_RC_ERROR, _("Object Path is NULL") );
    	return status;
    }
         
    return status;
}

//----------------------------------------------------------------------------
// Helper function to set the instance properties
//----------------------------------------------------------------------------
void Linux_ElementConformsToProfile_SetInstanceData(const CMPIBroker * broker, const CMPIInstance * instance, const _RESOURCEP *resource ) {
	
	CMSetProperty(instance, _SRCPROPERTYNAME, (CMPIValue*) &resource->src, CMPI_ref);
    CMSetProperty(instance, _TARGETPROPERTYNAME, (CMPIValue*) &resource->target, CMPI_ref);
  
};


bool Linux_ElementConformsToProfile_EqualsObjectPath 
	( const CMPIBroker *broker, const CMPIObjectPath *op1, const CMPIObjectPath *op2) {

	CMPIStatus status = {CMPI_RC_OK, NULL};		 	
	const char *keyName = NULL;
	int i;
	
	if (op1 == NULL || op2 == NULL) {
		return false;		
	}
	// Compare class names
	const char *srcClass1 =  CMGetCharsPtr(CMGetClassName(op1, &status), NULL);
	const char *srcClass2 =  CMGetCharsPtr(CMGetClassName(op2, &status), NULL);
	
	if (!strcmp(srcClass1, srcClass2) == 0) {		
		return false;
	}

	
	int count = CMGetKeyCount(op1, &status);
	if (status.rc != CMPI_RC_OK) {    		
	    return false;
	}	

	for (i = 0; i < count; i++) {
		CMPIData data1, data2;
        CMPIString *keyString;
        data1 = CMGetKeyAt(op1, i, &keyString, &status);
        if (status.rc != CMPI_RC_OK) {
        	return false;
        }
        keyName = CMGetCharPtr(keyString);
        data2 = CMGetKey(op2, keyName, &status);
        if (status.rc != CMPI_RC_OK) {
        	return false;
        }
        if (!Linux_ElementConformsToProfile_Compare_CMPIData( broker, &data1, &data2 )) {
        	return false;
        }
	}
	
	return true;
}


bool Linux_ElementConformsToProfile_Compare_CMPIData 
			(const CMPIBroker * broker, const CMPIData *data1, const CMPIData *data2) {
	
    if (data1->type != data2->type) {
        return false;
    } 
    
    if (data1->type == CMPI_ref) {
    	return Linux_ElementConformsToProfile_EqualsObjectPath( broker, data1->value.ref, data2->value.ref);
    } else if (data1->type == CMPI_string) {
		const char *value1 = CMGetCharPtr( data1->value.string );
		const char *value2 = CMGetCharPtr( data2->value.string );
		return (strcasecmp(value1, value2)==0);	
    } else if (data1->type == CMPI_INTEGER) {
    	return (data1->value.Int == data2->value.Int);
    } 
    return false;
}


bool Linux_ElementConformsToProfile_Validate(const CMPIBroker *broker, 
		const CMPIObjectPath *op1, const CMPIObjectPath *op2,
		const CMPIObjectPath *resultOP,
		const char *role1, char * role2, 
		const char *resultRole1, const char * resultRole2,
		const char *resultClass) {
	
	CMPIStatus status = {CMPI_RC_OK, NULL};

	//printf("Objectpath1: \nObjectPath2: \nRole1: %s\nRole2: %s\nResultRole1:%s\nResultRole2:%s\nResultclass:%s\n",
		//CMGetCharPtr(CMObjectPathToString(&op1, status)), CMGetCharPtr(CMObjectPathToString(op2, status)), 
	//	role1, role2, resultRole1, resultRole2, resultClass);
	
	
	if (!Linux_ElementConformsToProfile_EqualsObjectPath(broker, op1, op2)) {
		printf("failed object path compare \n");
		return false;
	}    	

	if ((role1 != NULL) && (strcmp(role1, role2)!=0)) {
		printf("failed role compare %s %s\n", role1, role2);
		return false;    		
	}
	
	if ((resultRole1 != NULL) && (strcmp(resultRole1, resultRole2)!=0)) {
		printf("failed resultrole compare %s %s\n", resultRole1, resultRole2);
		return false;    		
	}

	if ( resultClass != NULL  ) {
		if (resultOP == NULL) {
			printf("failed resultOp null\n");
			return false;			
		}
		if (!CMClassPathIsA(broker, resultOP, resultClass, &status) ) {
			printf("failed resultClass compare \n");
			return false;
		}
	}

	return true;
}
