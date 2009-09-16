///====================================================================
/// ra-support.h
///
/// © Copyright IBM Corp. 2007
///
/// THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
/// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
/// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
///
/// You can obtain a current copy of the Eclipse Public License from
/// http://www.opensource.org/licenses/eclipse-1.0.php
///
/// Authors : Ashoka Rao.S <ashoka.rao (at) in.ibm.com>
///           Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
///
///======================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "sblim-dhcp.h"
#include "libuniquekey.h"

#include <sblim/smt_libra_execscripts.h>
#include <sblim/smt_libra_conf.h>
#include <sblim/smt_libra_rastr.h>
#include <sblim/smt_libra_monitors.h>

#ifndef _RA_SUPPORT_H_
#define _RA_SUPPORT_H_ 

#ifndef DHCPDCONF
	#define DHCPDCONF "dhcpconf"
#endif

#define DEFAULT_SERVICE_NAME	        "dhcp"

/** constants use to indicate the nature of the entity */
#define SUPPORTF    0x00000001  /** Indicates that entity is supported */
#define UNSUPPORTF  0x00000002  /** Indicates that the entity is not supporeted */
#define COMMENTF    0x00000004  /** Indicates a comment */
#define BLANKLINF   0x00000008  /** Indicates a blank line */
#define OPTIONF     0x00000010  /** Indicates an option */
#define NOOPTIONF   0x00000020  /** Indicates that the entity is not an option */
#define COMMAF	    0x00000040  /** Indicates a comma */
#define NULLVALF    0x00000080  /** Indicates Null value */
#define PARAM_MASK  0x000000FF  /** Mask used to determine the above flags */

/** constants used to indicate the entity */
#define PARAMSF	    0x00000100  /** Indicates a parameter or option */
#define SUBNETF	    0x00000200  /** Indicates a subnet */
#define SHAREDNETF  0x00000400  /** Indicates a sharednet */
#define HOSTF	    0x00000800  /** Indicates a Host */
#define GROUPF	    0x00001000  /** Indicates a Group */
#define CLASSF	    0x00002000  /** Indicates a Class */
#define POOLF	    0x00004000  /** Indicates a Pool */
#define GLOBALF	    0x00008000  /** Indicates a Global */
#define SERVICEF    0x00010000  /** Indicates service */
#define SERCONFF    0x00020000  /** Indicates Service Configuration */
#define DECL_MASK   0x000FFF00  /** Mask used to determine the above flags */

#define DUMMYF	    0x00100000  /** Indicates dummy value */
#define MATCHF	    0x00200000  /** Indicates a match statement in config file */
#define SPAWNF	    0x00400000  /** Indicates a spawn statement in config file */
#define IF_CONF	    0x00800000  /** Indicates a conditional statement in the config file */
#define IRLVNTF	    0x01000000  /** Indicates a irrelevant statement in the config file */

/** List structure used to hold the values associated with a parameter */
typedef struct _LIST {
    void *content;
    struct _LIST * next;
} LIST;

/** Structure to hold the details of every entity in the dhco tree structure - acts as the main node */
typedef struct _NODE {
    char * obName;   /** Name value */      
    char * obValue;  /** value associated with the name */
    int obFlags;  /** flag to indicate the type */
    unsigned long long obID;  /** ID for the node */

    struct _NODE * parent;  /** Pointer to the parent node in the hierarchy */
    struct _NODE * nextup;  /** Pointer to the preceeding node in the same hierarchial level under the same parent */
    struct _NODE * nextdown; /** Pointer to the succeeding node in the same hierarchial level under the same parent */
    struct _NODE * descend;  /** Pointer to the child node */
} NODE;

extern NODE * dhcp_conf_tree ;
extern pthread_mutex_t dsLock;
extern pthread_mutex_t fileLock;
extern pthread_mutex_t raLock;

extern NODE * ra_createNode(); /** Method to create a node */
extern void ra_deleteNode(NODE * ); /** Method to delete a node */
extern NODE * ra_addRight(NODE * , NODE * ); /** Add the node in the same hierarchy  */ 
extern NODE * ra_addDown(NODE * , NODE * ); /** Add the new node as the child of the current node and the current as the parent of the new node */
extern NODE * ra_appendNode(NODE * , NODE * ); /** Add the node to the list of nodes in the same hierarchy */
extern NODE * ra_dropChild(NODE * , NODE * ); /** Add the 'child' node as the child of the 'parent' node */
extern NODE * ra_insertDescend(NODE * , NODE * ); /** Add the new node in the same level of hierarchy and as the child of the parent of the current node */
extern NODE * ra_populateNode(NODE * , char * , void * , int , unsigned long long); /** Method to populate the node */
extern LIST * ra_genListNode(char * );  /** Method to generate the list node to hold values pertaining to a parameter */ 
extern LIST * ra_appendToList(LIST * , char * ); /** Method to add a list node  */
extern void ra_deleteList(LIST * ); /** Delete a list node */
extern void ra_setHashValue(int ); /** set the hash valueto the node */
extern int ra_getHashValue();  /** derive the hash value */
extern int ra_writeConf(NODE * , char * );  /** write the conf file from dhcp conf tree - wrapper for writeTree method */
extern int ra_writeTree(NODE * , FILE * , int ); /** main methid to write the conf tree to conf file */
extern int ra_writeComment(NODE * , FILE * ); /** write the comment to the conf file */
extern int ra_writeSubnet(NODE * , FILE * , int ); /** write the subnet to the conf file */
extern int ra_writeSharednet(NODE * , FILE * , int ); /** write the sharednet to the conf file */
extern int ra_writeGroup(NODE * , FILE * , int ); /** write the group to the conf file */
extern int ra_writePool(NODE * , FILE * , int ); /** write the pool to the conf file */
extern int ra_writeClass(NODE * , FILE * , int ); /** write the class to the conf file */
extern int ra_writeHost(NODE * , FILE * , int ); /** write the host to the conf file */
extern int ra_writeParams(NODE * , FILE * , int ); /** write the params to the conf file */
extern int ra_writeIfCond(NODE *, FILE *, int); /** write the conditional statement to the conf file */


extern int ra_retriveHashKey(); /** Method to retrieve the hash key */
extern void ra_storeHashKey(int); /** Mehtod to store the hash key */
extern char * ra_multiValuedString(LIST *, int ); /** Method to indicate the value string of a parameter is a multivalued string and 
						      list is the pointer to the structure */

extern void ra_Initialize( _RA_STATUS* );
extern void ra_CleanUp(); /** method to clean up the entire tree structure */

extern void ra_updateDhcpdFile(); /** Method to update the changes to the file */

extern unsigned long long ra_convertToID(char *);  /** Method to convert the string to ID*/
extern void ra_updateDataStructure();  /** Method to update changes to the tree structure */
extern int ra_setInstaceID(NODE * , long long , int ); /** Method to set InstanceID */
extern int ra_setInstForNode(NODE * , NODE * , int ); /** Method to set the InstanceID to the node */
extern NODE * parseConfigFile (char *, char *);  /** Method used to parse the config file to generate the dhcp conf tree*/
extern unsigned long long ra_getInsertKey(); /** Method to generate the key to be inserted */
extern void ra_deletedEntity(); /** Method to indicate the deleted entity for the hashing library */
extern void ra_modifiedEntity(); /** Method to indicate the modified entity for the hashing library */
extern void ra_lockRaData();
extern void ra_unlockRaData();
#endif
