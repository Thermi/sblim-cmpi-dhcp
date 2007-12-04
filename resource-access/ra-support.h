/*
 * ra-support.h
 *
 * © Copyright IBM Corp. 2007
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Common Public License from
 * http://www.opensource.org/licenses/cpl1.0.php
 *
 * Authors : Ashoka Rao.S <ashoka.rao (at) in.ibm.com>
 *           Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sblim-dhcp.h"

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

#define SUPPORTF    0x00000001
#define UNSUPPORTF  0x00000002
#define COMMENTF    0x00000004
//#define MULTIPLEF   0x00000008
#define OPTIONF     0x00000010
#define NOOPTIONF   0x00000020
#define COMMAF	    0x00000040
#define NULLVALF    0x00000080
#define PARAM_MASK  0x000000FF

#define PARAMSF	    0x00000100
#define SUBNETF	    0x00000200
#define SHAREDNETF  0x00000400
#define HOSTF	    0x00000800
#define GROUPF	    0x00001000
#define CLASSF	    0x00002000
#define POOLF	    0x00004000
#define GLOBALF	    0x00008000
#define SERVICEF    0x00010000
#define SERCONFF    0x00020000
#define DECL_MASK   0x000FFF00

#define DUMMYF	    0x00100000
#define MATCHF	    0x00200000
#define SPAWNF	    0x00400000
#define IF_CONF	    0x00800000
#define IRLVNTF	    0x01000000

typedef struct _LIST {
    void *content;
    struct _LIST * next;
} LIST;

typedef struct _NODE {
    char * obName;
    char * obValue;
    /*    
      union {
	char * single;
	LIST * multiple;
    } obValue; 
     */
    int obFlags;
    unsigned long long obID;

    struct _NODE * parent;
    struct _NODE * nextup;
    struct _NODE * nextdown;
    struct _NODE * descend;
} NODE;

extern NODE * dhcp_conf_tree ;
extern NODE * current_parent ;
extern NODE * current_node ;
extern NODE * error_node;
extern NODE * service_node;
extern NODE * serconf_node;

extern NODE * ra_createNode();
extern void ra_deleteNode(NODE * );
extern NODE * ra_addRight(NODE * , NODE * );
extern NODE * ra_addDown(NODE * , NODE * );
extern NODE * ra_appendNode(NODE * , NODE * );
extern NODE * ra_dropChild(NODE * , NODE * );
extern NODE * ra_insertDescend(NODE * , NODE * );
extern NODE * ra_populateNode(NODE * , char * , void * , int , int );
extern LIST * ra_genListNode(char * );
extern LIST * ra_appendToList(LIST * , char * );
extern void ra_deleteList(LIST * );
extern void ra_setHashValue(int );
extern int ra_getHashValue();
extern int ra_writeConf(NODE * , char * );
extern int ra_writeTree(NODE * , FILE * , int );
extern int ra_writeComment(NODE * , FILE * );
extern int ra_writeSubnet(NODE * , FILE * , int );
extern int ra_writeSharednet(NODE * , FILE * , int );
extern int ra_writeGroup(NODE * , FILE * , int );
extern int ra_writePool(NODE * , FILE * , int );
extern int ra_writeClass(NODE * , FILE * , int );
extern int ra_writeHost(NODE * , FILE * , int );
extern int ra_writeParams(NODE * , FILE * , int );
extern int ra_writeIfCond(NODE *, FILE *, int);


extern int ra_retriveHashKey();
extern void ra_storeHashKey(int);
extern char * ra_multiValuedString(LIST *, int );

extern void ra_Initialize( _RA_STATUS* );
extern void ra_CleanUp();

extern void ra_updateDhcpdFile();

extern unsigned long long ra_convertToID(char *);
extern void ra_updateDataStructure();
extern int ra_setInstaceID(NODE * , long long , int );
extern int ra_setInstForNode(NODE * , NODE * , int );
extern NODE * parseConfigFile (FILE *, FILE *);
#endif
