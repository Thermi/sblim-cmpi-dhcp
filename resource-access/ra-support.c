/*
 * ra-support.c
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
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include "ra-support.h"

NODE *dhcp_conf_tree = NULL;
NODE *current_parent = NULL;
NODE *current_node = NULL;
NODE *error_node = NULL;
NODE *service_node = NULL;
NODE *serconf_node = NULL;


NODE *ra_createNode()
{
    NODE *temp;

    temp = (NODE *) malloc(sizeof(NODE));
    if (temp == NULL) {
	//Dynamic Memory Allocation Failed
	return NULL;
    }

    memset(temp, 0, sizeof(NODE));

    return temp;
}

void ra_deleteNode(NODE * node)
{
    if (node == NULL)
	return;

    free(node->obName);
    node->obName = NULL;
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

NODE *ra_appendNode(NODE * begin, NODE * end)
{
    NODE *temp;

    if (begin == NULL || end == NULL)
	return begin;
    for (temp = begin; temp->nextdown != NULL; temp = temp->nextdown);
    ra_addDown(temp, end);

    return begin;
}

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

NODE *ra_populateNode(NODE * obj, char *name, void *value, int flags,
		      int id)
{
    obj->obName = name;
    obj->obFlags = flags;
    obj->obID = id;
    obj->obValue = (char *) value;

    return obj;
}


LIST *ra_genListNode(char *content)
{
    LIST *temp;

    temp = (LIST *) malloc(sizeof(LIST));
    if (temp == NULL) {
	//Dynamic Memory Allocation Failed
	return NULL;
    }
    temp->next = NULL;
    temp->content = (char *) content;

    return temp;
}

LIST *ra_appendToList(LIST * list, char *content)
{
    LIST *temp;

    if (list == NULL || content == NULL)
	return list;
    for (temp = list; temp->next != NULL; temp = temp->next);
    temp->next = (LIST *) ra_genListNode(content);

    return list;
}

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

void ra_printID(FILE * fd, int offset, NODE * node)
{
    //int i = 75-offset;
    //fprintf(fd,"%*s%llx\n",i>0?i:14 , "\t", node->obID);
    fprintf(fd, "\n");
}
int ra_writeConf(NODE * tree, char *file)
{
    FILE *fd = NULL;

    fd = fopen(file, "w");
    /*if(fd == NULL) {
       //file could not be opened
       //condition handled at the beginning of invoking the provider
       } */
    ra_writeTree(tree, fd, 0);
    fclose(fd);

    return 0;
}

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
	    fprintf(fd, "%c\n\n", '}');
	    break;
	case SHAREDNETF:
	    ra_writeSharednet(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n\n", '}');
	    break;
	case HOSTF:
	    ra_writeHost(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n\n", '}');
	    break;
	case GROUPF:
	    ra_writeGroup(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n\n", '}');
	    break;
	case CLASSF:
	    ra_writeClass(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n\n", '}');
	    break;
	case POOLF:
	    ra_writePool(temp, fd, level);
	    ra_writeTree(temp->descend, fd, level + 1);
	    if (level)
		fprintf(fd, "%*s", 8 * level, "");
	    fprintf(fd, "%c\n\n", '}');
	    break;
	case GLOBALF:
	    ra_writeTree(temp->descend, fd, level);
	    break;
	}
    }
    return 0;
}



#define SUBT 0
#define SHRT 1
#define POOT 2
#define HOST 3
#define GRPT 4
#define UNUSDT 7


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



int ra_writeComment(NODE * node, FILE * fd)
{
    fprintf(fd, "%s\n", node->obName);
    return 0;
}

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

int ra_writeSharednet(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s %s {", node->obName, node->obValue);
    ra_printID(fd, offset, node);
    return 0;
}

int ra_writeGroup(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s {", node->obName);
    ra_printID(fd, offset, node);
    return 0;
}

int ra_writePool(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s {", node->obName);
    ra_printID(fd, offset, node);
    return 0;
}

int ra_writeClass(NODE * node, FILE * fd, int level)
{
    int offset = 0;
    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s %s {\n", node->obName, node->obValue);
    /*  printID(fd, offset, node); */
    return 0;
}

int ra_writeHost(NODE * node, FILE * fd, int level)
{
    int offset = 0;

    if (level)
	offset += fprintf(fd, "%*s", 8 * level, "");
    offset += fprintf(fd, "%s %s {", node->obName, node->obValue);
    ra_printID(fd, offset, node);
    return 0;
}
int ra_writeIfCond(NODE * node, FILE * fd, int level)
{
    if (level)
	fprintf(fd, "%*s", 8 * level, "");

    fprintf(fd, "%s %s\n", node->obName, node->obValue);
    return 0;
}

int ra_writeParams(NODE * node, FILE * fd, int level)
{
    int offset = 0;

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

time_t updateTime = 0;
struct stat fileStat;
FILE *inFile = NULL, *outFile = NULL;

void ra_updateDhcpdFile()
{
    struct conf* dhcp_conf = read_conf( PROVIDER_CONFFILE, PROVIDER_CONFFILE);
    char* conffile = get_conf(dhcp_conf, DHCPDCONF);
    
    if(!conffile) 
	conffile = strdup(DEFAULT_DHCPCONF);
 
    ra_writeConf(dhcp_conf_tree, conffile);
    // give time for file statistics updation
    sleep(1);
   
}


void ra_updateDataStructure(_RA_STATUS * ra_status)
{
    struct conf* dhcp_conf = read_conf( PROVIDER_CONFFILE, PROVIDER_CONFFILE);
    char* conffile = get_conf(dhcp_conf, DHCPDCONF);

    if(!conffile)
	conffile = strdup(DEFAULT_DHCPCONF);

    stat(conffile, &fileStat);

    if (fileStat.st_mtime > updateTime) {
	inFile = fopen(conffile, "r");
	if (inFile == NULL) {
	    //File not found
	    setRaStatus(ra_status, RA_RC_FAILED, DHCP_CONF_FILE_NOT_FOUND,
			_("dhcpd.conf not found"));
	    return;
	}

	ra_deleteNode(dhcp_conf_tree);
	parseConfigFile(inFile, outFile);
	ra_setInstaceID(dhcp_conf_tree, 0, 0);
	fclose(inFile);
    }

    updateTime = fileStat.st_mtime;
}

void ra_Initialize(_RA_STATUS * ra_status)
{
    ra_updateDataStructure(ra_status);
    return;
}

void ra_CleanUp()
{
    ra_deleteNode(dhcp_conf_tree);
    return;
}
