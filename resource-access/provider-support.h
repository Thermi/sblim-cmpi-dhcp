///==================================================================
/// provider-support.h
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
///====================================================================

#include <unistd.h>
#include "ra-support.h"
#include "sblim-dhcp.h"

/** Predetermined constants to define how the Nodes need to be matched  */
#define IDONLY	    0x000  /** Match using the ID only */
#define IDNTYPE	    0x001  /** Match using the ID and TYPE information  */
#define TYPEONLY    0x002  /** Match using the TYPE informatin only */
#define TYPE_OR	    0x000  /** Match using the TYPE information or something else specified in the method where in used */
#define TYPE_AND    0x010  /** Match using the TYPE information and someother information as specified in the method wherein used */

/** A structre which holds the details of the number of node elements present in the queue with a pointer to the head element  */
typedef struct _queue {
    int count;
    LIST * current;
} QUEUE;

extern NODE * ra_createSerConf(int ); /** Method to create a ServiceConfiguratin Node */
extern NODE * ra_createService(int ); /** Method to create a Service Node */
extern NODE * ra_createSubnet(char * , char * , unsigned long long ); /** Method to create a Subnet Node */
extern NODE * ra_createSharedNet(char * , char * , unsigned long long  ); /** Method to create a Sharednet Node */
extern NODE * ra_createGroup(char * , unsigned long long ); /** Method to create a Group Node */
extern NODE * ra_createPool(char * , unsigned long long ); /** Method to create a Pool Node */
extern NODE * ra_createClass(char * , char * , unsigned long long ); /** Method to create a Class Node */
extern NODE * ra_createHost(char * , char * , unsigned long long ); /** Method to create a Host Node */
extern NODE * ra_createParam(char * , char * , int , unsigned long long ); /** Method to create a Parameter Node */
extern NODE * ra_createParamList(char * , LIST * , int , unsigned long long); /** Method to create a list of Parameteres */
extern NODE * ra_createComment(char * ); /*Method to create a Node to hold the comment */
extern NODE * ra_createDummy(); /** Method to create a Dummy Node */
extern NODE * ra_createIrlvnt(char * ); /** Method to create a Node to hold the Irrelevant value in the configuratio file */
extern char * ra_instanceId(NODE *, char * ); /** Method to determine the InstanceID of the entity whose node is passed as an argument */
extern NODE * ra_getEntity(unsigned long long , NODE * , _RA_STATUS*); /** Method to arrive at the Node of interest */
extern NODE * ra_getTheEntity(unsigned long long, int , NODE * ); /** Method to get the entity of interest */
extern NODE ** ra_getAllEntity(int , NODE *, _RA_STATUS* ); /** Method to get all the entities */
extern NODE * ra_getEntityinEntity(unsigned long long, NODE * ); /** Method to get the entity of interest from the entity specified */
extern NODE ** ra_getAllEntitiesinEntity(int, NODE * ); /**Mehtod to get all the entities in the entity specified */
extern unsigned long long ra_getKeyFromInstance(char * ); /** Method to get the key value of the Instance */
extern char * ra_tokenize(char * , int); /** Method to tokenize the object path */
extern char * ra_get_hostname(); /** Method to get the hostname */
extern char * ra_removeQuotes(char * ); /** Method to remove the quotes from the string specified */
extern int ra_findLevel(const char * ); /** Method to find the hierarchial leve of the entity */
