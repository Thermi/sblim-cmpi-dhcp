/*
 * provider-support.h
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

#include <unistd.h>
#include "ra-support.h"
#include "sblim-dhcp.h"

#define IDONLY	    0x000
#define IDNTYPE	    0x001
#define TYPEONLY    0x002
#define TYPE_OR	    0x000
#define TYPE_AND    0x010

typedef struct _queue {
    int count;
    LIST * current;
} QUEUE;

extern NODE * ra_createSerConf(int );
extern NODE * ra_createService(int );
extern NODE * ra_createSubnet(char * , char * , int );
extern NODE * ra_createSharedNet(char * , char * , int );
extern NODE * ra_createGroup(char * , int );
extern NODE * ra_createPool(char * , int );
extern NODE * ra_createClass(char * , char * , int );
extern NODE * ra_createHost(char * , char * , int );
extern NODE * ra_createParam(char * , char * , int , int );
extern NODE * ra_createParamList(char * , LIST * , int , int );
extern NODE * ra_createComment(char * );
extern NODE * ra_createDummy();
extern NODE * ra_createIrlvnt(char * );
extern char * ra_instanceId(NODE *, char * );
extern NODE * ra_getEntity(unsigned long long , NODE * , _RA_STATUS*);
extern NODE * ra_getTheEntity(unsigned long long, int , NODE * );
extern NODE ** ra_getAllEntity(int , NODE *, _RA_STATUS* );
extern NODE * ra_getEntityinEntity(unsigned long long, NODE * );
extern NODE ** ra_getAllEntitiesinEntity(int, NODE * );
extern unsigned long long ra_getKeyFromInstance(char * );
extern char * ra_tokenize(char * );
extern char * ra_get_hostname();
extern char * ra_removeQuotes(char * );
extern int ra_findLevel(const char * );
