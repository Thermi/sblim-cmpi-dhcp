///===================================================================
/// ra-support.c
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
///===================================================================
 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "ra-support.h"

NODE *dhcp_conf_tree = NULL;  /** dhcp_conf_tree - the head node pointer for the conf tree structure */
pthread_mutex_t dsLock;       /** mutex lock for the conf tree structure */
pthread_mutex_t fileLock;     /** mutex lock for the conf file */
pthread_mutex_t raLock;

/** Method used to create a new node for the dhcp conf tree */
NODE *ra_createNode()
{
    NODE *temp;

    temp = (NODE *) malloc(sizeof(NODE));
    if (temp == NULL) {
	///Dynamic Memory Allocation Failed
	return NULL;
    }

    memset(temp, '\0', sizeof(NODE));
    return temp;
}

/** Method used to delete the specified node from the dhcp conf tree */
/** same method takes care of deleting all the children recursively */
void ra_deleteNode(NODE * node)
{
    if (node == NULL)
	return;

    if(node->obName)
	free(node->obName);
    node->obName = NULL;
    if(node->obValue)
	free(node->obValue);
    node->obValue = NULL;

    node->parent = NULL;
    if (node->nextup)
	node->nextup->nextdown = node->nextdown;
    if (node->nextdown)
	node->nextdown->nextup = node->nextup;

    node->nextup = node->nextdown = NULL;

    ra_deleteNode(node->descend);

    free(node);
    node = NULL;
}

/** Add the node in the same hierarchy  */
NODE *ra_addRight(NODE * current, NODE * new)
{
    NODE *nptr;

    if (current == NULL || new == NULL)
	return current;

    current->descend = new;
    for (nptr = new; nptr; nptr = nptr->nextdown)
	nptr->parent = current;

    return current;
}

/** Add the new node as the child of the current node and the current as the parent of the new node */
NODE *ra_addDown(NODE * current, NODE * new)
{
    NODE *temp = NULL;
    if (current == NULL || new == NULL)
	return new;

    new->parent = current->parent;
    new->nextup = current;
    for (temp = new; temp->nextdown != NULL; temp = temp->nextdown);

    temp->nextdown = current->nextdown;
    current->nextdown = new;

    return new;
}

/** append the node to the end of the list of nodes */
NODE *ra_appendNode(NODE * begin, NODE * end)
{
    NODE *temp;

    if (begin == NULL || end == NULL)
	return begin;
    for (temp = begin; temp->nextdown != NULL; temp = temp->nextdown);
    ra_addDown(temp, end);

    return begin;
}

/** Add the 'child' node as the child of the 'parent' node */
NODE *ra_dropChild(NODE * parent, NODE * child)
{
    if (parent == NULL || child == NULL)
	return parent;
    child->parent = parent;
    if (parent->descend == NULL) {
	ra_addRight(parent, child);
	return parent;
    }

    ra_appendNode(parent->descend, child);

    return parent;
}

/** Add the new node in the same level of hierarchy and as the child of the parent of the current node */
NODE *ra_insertDescend(NODE * current, NODE * new)
{
    NODE *temp;

    if (current == NULL || new == NULL)
	return current;
    new->parent = current;
    if (current->descend == NULL) {
	ra_addRight(current, new);
	return current;
    }
    if (new->obFlags & PARAMSF) {
	temp = current->descend;

	if (!(temp->obFlags & PARAMSF)) {
	    current->descend = new;
	    new->nextdown = temp;
	    temp->nextup = new;

	    return current;
	}

	for (; temp->obFlags & PARAMSF; temp = temp->nextdown);
	new->nextup = temp->nextup;
	new->nextdown = temp;
	new->descend = NULL;
	temp->nextup = new;

	return current;
    }

    ra_appendNode(current->descend, new);

    return current;
}

/** Method used to poputate the values for the perticular node of the dhcp conf tree */
/** The flags indicate the type of node, i.e value represented by the node - subnet, pool etc */
NODE *ra_populateNode(NODE * obj, char *name, void *value, int flags,
		      unsigned long long id)
{
    obj->obName = name;
    obj->obFlags = flags;
    obj->obID = id;
    obj->obValue = (char *) value;

    return obj;
}

/** Method used to generate the list to hold the values associated with a parameter */
LIST *ra_genListNode(char *content)
{
    LIST *temp;

    temp = (LIST *) malloc(sizeof(LIST));
    if (temp == NULL) {
	///Dynamic Memory Allocation Failed
	return NULL;
    }
    memset(temp, '\0', sizeof(LIST));
    temp->next = NULL;
    temp->content = (char *) content;

    return temp;
}

/** Method used to append values to the list related to a parameter.  Invoked when the parameter has more than one
    value associated to it */
LIST *ra_appendToList(LIST * list, char *content)
{
    LIST *temp;

    if (list == NULL || content == NULL)
	return list;
    for (temp = list; temp->next != NULL; temp = temp->next);
    temp->next = (LIST *) ra_genListNode(content);

    return list;
}

/** Method used to remove a list gracefully */
void ra_deleteList(LIST * list)
{
    if (list == NULL)
	return;
    if (list->content != NULL) {
	free(list->content);
	list->content = NULL;
    }
    if (list->next != NULL)
	ra_deleteList(list->next);
    free(list);
    list = NULL;
}

/** Method to pring the ID to the specified file */
void ra_printID(FILE * fd, int offset, NODE * node)
{
    fprintf(fd, "\n");
}

/** main method invoked to write the content of the dhcpconf tree to the new conf file */
/** The file is locked to prevent simultaneous access */
int ra_writeConf(NODE * tree, char *file)
{
    FILE *fd = NULL;

    pthread_mutex_lock(&fileLock);
    fd = fopen(file, "w");
    ra_writeTree(tree, fd, 0);
    fclose(fd);
    pthread_mutex_unlock(&fileLock);

    return 0;
}

/** Method used to represent the contents of the dhcp conf tree structure as a config file */
/** uses the flag in every node to determine the type of node and represent it accordingly 
    in the generated file.  The indentation depending on the hierarchy of the occurance of the
    value is also taken care of */
int ra_writeTree(NODE * node, FILE * fd, int level)
{
    NODE *temp;

    if (node == NULL)
	return 0;

    for (temp = node; temp != NULL; temp = temp->nextdown) {
	if (temp->obFlags & DUMMYF)
	    continue;

	if (temp->obFlags & COMMENTF) {
	    ra_writeComment(temp, fd);
	    continue;
	}
	if (temp->obFlags & IF_CONF) {
	    ra_writeIfCond(temp, fd, level);
	    continue;
	}

	switch (temp->obFlags & DECL_MASK) {
	case PARAMSF:
	    ra_writeParams(temp, fd, level);
	    break;
	case SUBNETF:
	    ra_writeSubnet(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n", '}');
	    break;
	case SHAREDNETF:
	    ra_writeSharednet(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n", '}');
	    break;
	case HOSTF:
	    ra_writeHost(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n", '}');
	    break;
	case GROUPF:
	    ra_writeGroup(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n", '}');
	    break;
	case CLASSF:
	    ra_writeClass(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n", '}');
	    break;
	case POOLF:
	    ra_writePool(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n", '}');
	    break;
	case GLOBALF:
	    ra_writeTree(temp->descend, fd, level);
	    break;
	}
    }
    return 0;
}


/** constants assigned to represent different values in the conf file  */
/** These values are used to represent the entities in the IDs */
#define SUBT 0  /** Indicates subnet */
#define SHRT 1  /** Indicates sharednet */
#define POOT 2  /** Indicates pool */
#define HOST 3  /** Indicates host */
#define GRPT 4  /** Indicates group */
#define UNUSDT 7 /** unused - for future enhancements if necessary */

/** Method used to generate the InstanceID for every entity of the conf file */
int ra_generateID(NODE * node, unsigned long long key)
{
    int count = 0;
    long long seed = 0xa1b2c3d4;

    if ((node->obFlags & DECL_MASK) == SUBNETF) {
	for (count = 1; count < 10; count++) {
	    seed *= count;
	    seed += key;
	}
    } else {
	for (count = 0; node->obName[count]; count++) {
	    seed *= node->obName[count];
	    seed += key;
	}
    }
    node->obID = seed;

    return 0;
}

/** Method used to set the instance ID value stored in every node.  This value depends on
    the hierarchial level of the entity in the conf file and its nature */
int ra_setInstForNode(NODE * parent, NODE * child, int level)
{
    int count = 0, type = child->obFlags;
    unsigned long long key = parent->obID, inst = 0;
    NODE *nptr = NULL;

    for (nptr = parent->descend; nptr; nptr = nptr->nextdown)
	if (nptr->obFlags == type)
	    count++;

    switch (child->obFlags & DECL_MASK) {
    case PARAMSF:
	ra_generateID(child, key);
	break;
    case SUBNETF:
	inst = SUBT | (count << 3);
	ra_generateID(child, (inst << (10 * level)) + key);
	break;
    case SHAREDNETF:
	inst = SHRT | (count << 3);
	ra_generateID(child, (inst << (10 * level)) + key);
	break;
    case POOLF:
	inst = POOT | (count << 3);
	ra_generateID(child, (inst << (10 * level)) + key);
	break;
    case GROUPF:
	inst = GRPT | (count << 3);
	ra_generateID(child, (inst << (10 * level)) + key);
	break;
    case HOSTF:
	inst = HOST | (count << 2);
	ra_generateID(child, (inst << (10 * level)) + key);
	break;
    default:
	break;
    }
    return count;
}

/** Method used to set the instance ID value stored in every node.  This value depends on
      the hierarchial level of the entity in the conf file and its nature */
int ra_setInstaceID(NODE * node, long long key, int level)
{
    unsigned int inst = 0;
    unsigned int subc = 0, shrc = 0, pooc = 0, hosc = 0, grpc = 0;
    NODE *nptr = NULL;
    if (node == NULL)
	return 0;
    for (nptr = node; nptr != NULL; nptr = nptr->nextdown) {
	switch (nptr->obFlags & DECL_MASK) {
	case PARAMSF:
	    ra_generateID(nptr, key);
	    break;
	case SUBNETF:
	    inst = SUBT | (subc << 3);
	    ra_generateID(nptr, (inst << (10 * level)) + key);
	    subc++;
	    ra_setInstaceID(nptr->descend, nptr->obID, level + 1);
	    break;
	case SHAREDNETF:
	    inst = SHRT | (shrc << 3);
	    ra_generateID(nptr, (inst << (10 * level)) + key);
	    shrc++;
	    ra_setInstaceID(nptr->descend, nptr->obID, level + 1);
	    break;
	case HOSTF:
	    inst = HOST | (hosc << 2);
	    ra_generateID(nptr, (inst << (10 * level)) + key);
	    hosc++;
	    ra_setInstaceID(nptr->descend, nptr->obID, level + 1);
	    break;
	case GROUPF:
	    inst = GRPT | (grpc << 3);
	    ra_generateID(nptr, (inst << (10 * level)) + key);
	    grpc++;
	    ra_setInstaceID(nptr->descend, nptr->obID, level + 1);
	    break;
	case POOLF:
	    inst = POOT | (pooc << 3);
	    ra_generateID(nptr, (inst << (10 * level)) + key);
	    ++pooc;
	    ra_setInstaceID(nptr->descend, nptr->obID, level + 1);
	    break;
	case GLOBALF:
	    ra_setInstaceID(nptr->descend, key, 0);
	    break;
	}
    }
    return 1;
}


/** A comment in the orignal conf file stored as a node in the tree structure is written back as a comment to the conf file */
int ra_writeComment(NODE * node, FILE * fd)
{
    fprintf(fd, "%s", node->obName);
    return 0;
}

/** A subnet in the orignal conf file stored as a node in the tree structure is written back as a subnet to the conf file */
int ra_writeSubnet(NODE * node, FILE * fd, int level)
{
    int offset = 0;

    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset +=
	fprintf(fd, "subnet %s netmask %s {", node->obName, node->obValue);
    ra_printID(fd, offset, node);
    return 0;
}

/** A sharednet in the orignal conf file stored as a node in the tree structure is written back as a sharednet to the conf file */
int ra_writeSharednet(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s %s {", node->obName, node->obValue);
    ra_printID(fd, offset, node);
    return 0;
}

/** A group in the orignal conf file stored as a node in the tree structure is written back as a group to the conf file */
int ra_writeGroup(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s {", node->obName);
    ra_printID(fd, offset, node);
    return 0;
}

/** A pool in the orignal conf file stored as a node in the tree structure is written back as a pool to the conf file */
int ra_writePool(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s {", node->obName);
    ra_printID(fd, offset, node);
    return 0;
}

/** A class in the orignal conf file stored as a node in the tree structure is written back as a class to the conf file */
int ra_writeClass(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s %s {\n", node->obName, node->obValue);
    return 0;
}

/** A host in the orignal conf file stored as a node in the tree structure is written back as a host to the conf file */
int ra_writeHost(NODE * node, FILE * fd, int level)
{
    int offset = 0;

    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s %s {", node->obName, node->obValue);
    ra_printID(fd, offset, node);
    return 0;
}

/** A conditional statement in the orignal conf file stored as a node in the tree structure is written back as a conditional statement to the conf file */
int ra_writeIfCond(NODE * node, FILE * fd, int level)
{
    if (level)
	fprintf(fd, "%*s", 8 * level, "");

    fprintf(fd, "%s %s", node->obName, node->obValue);
    return 0;
}

/** A parameter/options in the orignal conf file stored as a node in the tree structure is written back as a parameter/option to the conf file */
int ra_writeParams(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if(node->obFlags & BLANKLINF) {
	fprintf(fd, "\n");
	return 0;
    }

    if (node->obFlags & IRLVNTF) {
	fprintf(fd, "%s", node->obFlags & MATCHF ? "match" : "");
	fprintf(fd, "%s", node->obFlags & SPAWNF ? "spawn" : "");
	fprintf(fd, "%s", node->obFlags & OPTIONF ? "option " : "");
	fprintf(fd, "%s\n", node->obName);
	return 0;
    }

    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");

    offset += fprintf(fd, "%s", node->obFlags & MATCHF ? "match" : "");
    offset += fprintf(fd, "%s", node->obFlags & SPAWNF ? "spawn" : "");
    offset += fprintf(fd, "%s", node->obFlags & OPTIONF ? "option " : "");
    offset += fprintf(fd, "%s", node->obName);

    if (node->obFlags & NULLVALF)
	offset += fprintf(fd, ";");
    else
	offset += fprintf(fd, " %s;", node->obValue);

    if (node->obID)
	ra_printID(fd, offset, node);
    else
	fprintf(fd, "\n");
    return 0;

}

/** if the parameter has more than one value associated with it, all the values are stored in a list structure */
char *ra_multiValuedString(LIST * valueList, int coma)
{
    int count = 0, offset = 0;
    char *ret = NULL;
    LIST *list = valueList;

    count = 1 + strlen(list->content);
    for (list = list->next; list != NULL; list = list->next) {
	if (strcasecmp("dummy", (char *) list->content) == 0)
	    continue;
	count += coma ? 2 : 1;
	count += strlen(list->content);
    }
    ret = (char *) calloc(count + 10, sizeof(char));


    list = valueList;
    offset += sprintf(&ret[offset], "%s", (char *) list->content);
    for (list = list->next; list != NULL; list = list->next) {
	if (strcasecmp("dummy", (char *) list->content) == 0)
	    continue;
	offset +=
	    sprintf(&ret[offset], "%s%s", coma ? ", " : " ",
		    (char *) list->content);
    }
    ret[offset + 1] = '\0';

    return ret;
}

unsigned long long ra_convertToID(char *str)
{
    unsigned long long val = 0;
    for (; *str; str++) {
	val <<= 4;
	if (isalpha(*str))
	    val |= (int) (*str) - 'a' + 10;
	else if (isdigit(*str))
	    val |= (int) (*str) - '0';
    }

    return val;
}

/** Method used to update the conf file */
void ra_updateDhcpdFile()
{
    struct conf* dhcp_conf = read_conf( PROVIDER_CONFFILE, PROVIDER_CONFFILE);
    char* conffile = get_conf(dhcp_conf, DHCPDCONF);
    
    if(!conffile) 
	conffile = strdup(DEFAULT_DHCPCONF);
 
    ra_writeConf(dhcp_conf_tree, conffile);
    /// give time for file statistics updation
    sleep(1);
   
}

/** Method used to insert the key  */
unsigned long long ra_getInsertKey(){
	int line;
	unsigned long long newId = 0;
	struct conf * dhcp_conf = read_conf( PROVIDER_CONFFILE, PROVIDER_CONFFILE);
	char * conffile = get_conf(dhcp_conf, DHCPDCONF);

	line = addedEntity(conffile);
	getUniqueKey(conffile, line, &newId);
	return newId;		
}

void ra_deletedEntity(){
	struct conf * dhcp_conf = read_conf( PROVIDER_CONFFILE, PROVIDER_CONFFILE);
	char * conffile = get_conf(dhcp_conf, DHCPDCONF);
	deletedEntity(conffile);
}

void ra_modifiedEntity(){
	struct conf * dhcp_conf = read_conf( PROVIDER_CONFFILE, PROVIDER_CONFFILE);
	char * conffile = get_conf(dhcp_conf, DHCPDCONF);
	modifiedEntity(conffile);
}

static int _ra_usageCount=0;
static time_t updateTime = 0;
static pthread_mutex_t updateLock;

/** Method used to update the data structure on identifying some changes */
void ra_updateDataStructure(_RA_STATUS * ra_status)
{
    struct conf* dhcp_conf = read_conf( PROVIDER_CONFFILE, PROVIDER_CONFFILE);
    char* conffile = get_conf(dhcp_conf, DHCPDCONF);
    struct stat fileStat;

    if(!conffile)
	conffile = strdup(DEFAULT_DHCPCONF);

    pthread_mutex_lock(&updateLock);

    if((stat(conffile, &fileStat)) == -1) {
	if(errno == ENOENT) {
	    setRaStatus(ra_status, RA_RC_FAILED, DHCP_CONF_FILE_NOT_FOUND,
			_("dhcpd.conf not found"));
	    return;
	}
    }


    if (fileStat.st_mtime > updateTime) {
	
	pthread_mutex_lock(&dsLock);
	ra_deleteNode(dhcp_conf_tree);
	pthread_mutex_unlock(&dsLock);

	parseConfigFile(conffile, NULL);
    }

    updateTime = fileStat.st_mtime;
    pthread_mutex_unlock(&updateLock);
}

/** Method used as a wrapper to the method for initializing and updating the data structure */
void ra_Initialize(_RA_STATUS * ra_status)
{
    if( _ra_usageCount == 0)
	ra_updateDataStructure(ra_status);
    _ra_usageCount++;

    return;
}

/** Method used to completely clean up the dhcpconf tree */
void ra_CleanUp()
{
    --_ra_usageCount;
    if( _ra_usageCount == 0){

	pthread_mutex_lock(&dsLock);
	ra_deleteNode(dhcp_conf_tree);
	pthread_mutex_unlock(&dsLock);
    }

    return;
}

void ra_lockRaData(){
    pthread_mutex_lock(&raLock);
}

void ra_unlockRaData(){
    pthread_mutex_unlock(&raLock);
}

