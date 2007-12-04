/*
 * provider-support.c
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


#include "provider-support.h"

NODE *ra_createSerConf(int id)
{
    int flag = 0;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    flag = SUPPORTF | SERCONFF;

    return ra_populateNode(temp, NULL, NULL, flag, id);
}

NODE *ra_createService(int id)
{
    int flag = 0;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    flag = SUPPORTF | SERVICEF;

    return ra_populateNode(temp, NULL, NULL, flag, id);
}

NODE *ra_createSubnet(char *name, char *value, int id)
{
    int flag = 0;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    flag = SUPPORTF | SUBNETF;

    return ra_populateNode(temp, name, value, flag, id);
}

NODE *ra_createSharedNet(char *name, char *value, int id)
{
    int flag = 0;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    flag = SUPPORTF | SHAREDNETF;

    return ra_populateNode(temp, name, value, flag, id);
}

NODE *ra_createGroup(char *name, int id)
{
    int flag = 0;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    flag = SUPPORTF | GROUPF;

    return ra_populateNode(temp, name, NULL, flag, id);
}

NODE *ra_createPool(char *name, int id)
{
    int flag = SUPPORTF | POOLF;
    NODE *temp = ra_createNode();

    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }
    return ra_populateNode(temp, name, NULL, flag, id);
}

NODE *ra_createClass(char *name, char *value, int id)
{
    int flag = UNSUPPORTF | CLASSF;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    return ra_populateNode(temp, name, value, flag, id);
}

NODE *ra_createHost(char *name, char *value, int id)
{
    int flag = SUPPORTF | HOSTF;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    return ra_populateNode(temp, name, value, flag, id);
}

NODE *ra_createParam(char *name, char *value, int flag, int id)
{
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    return ra_populateNode(temp, name, value, flag, id);
}

NODE *ra_createComment(char *comment)
{
    int flag = COMMENTF | NULLVALF;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    return ra_populateNode(temp, comment, NULL, flag, 0);
}

NODE *ra_createIrlvnt(char *comment)
{
    int flag = UNSUPPORTF | PARAMSF | NULLVALF | IRLVNTF;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    return ra_populateNode(temp, comment, NULL, flag, 0);
}

NODE *ra_createDummy()
{
    int flag = DUMMYF | NULLVALF;
    NODE *temp = ra_createNode();
    if (temp == NULL) {
	//Memory allocation failed
	return NULL;
    }

    return ra_populateNode(temp, NULL, NULL, flag, 0);
}

int ra_matchNode(unsigned long long id, int type, int criteria,
		 NODE * node)
{
    int state = 0;

    state =
	criteria & TYPE_AND ? ((node->obFlags & type) ==
			       type) : (node->obFlags & type);
    switch (criteria & 0x03) {
    case TYPEONLY:
	return state;
    case IDNTYPE:
	if (!state)
	    return state;
    case IDONLY:
	return (node->obID == id);
    }
    return 0;
}

void ra_addToList(LIST * list, NODE * node)
{
    LIST *lptr = (LIST *) ((QUEUE *) list->content)->current;
    lptr->next = (LIST *) malloc(sizeof(LIST));
    lptr->next->content = (NODE *) node;
    lptr->next->next = NULL;
    ((QUEUE *) list->content)->current = lptr->next;
    ((QUEUE *) list->content)->count++;
}

int ra_traverseTree(unsigned long long id, int type, int flag, NODE * node,
		    LIST * list, int recurse, _RA_STATUS * ra_status)
{
    int ret = 0;
    NODE *nptr = node ? node : dhcp_conf_tree;

    if (dhcp_conf_tree == NULL) {
	setRaStatus(ra_status, RA_RC_FAILED, DHCP_CONF_FILE_NOT_FOUND,
		    _
		    ("dhcp_conf_tree = NULL OR dhcpd.conf file not found"));
	return -1;
    }

    if (nptr == NULL) {
	setRaStatus(ra_status, RA_RC_FAILED, DHCP_CONF_FILE_NOT_FOUND,
		    _
		    ("Entity does not exist OR dhcpd.conf file not found"));
	return -2;
    }

    for (; nptr; nptr = nptr->nextdown) {
	if (ra_matchNode(id, type, flag, nptr)) {
	    ra_addToList(list, nptr);
	    if (id)
		return 1;
	}
	if (recurse && nptr->descend)
	    ret =
		ra_traverseTree(id, type, flag, nptr->descend, list,
				recurse, ra_status);
	if (ret)
	    return ret;
    }

    return 0;
}

NODE **ra_convertToArray(LIST * list)
{
    int count = ((QUEUE *) list->content)->count;
    LIST *current = NULL;
    NODE **ret = NULL;

    ret = (NODE **) calloc(count + 1, sizeof(NODE *));
    for (current = list->next, count = 0; current;
	 current = current->next, count++)
	ret[count] = (NODE *) current->content;

    for (current = list->next; current; current = list->next) {
	list->next = current->next;
	free(current);
    }
    return ret;
}

NODE *ra_getEntity(unsigned long long id, NODE * node,
		   _RA_STATUS * ra_status)
{
    int flag = IDONLY;
    LIST list = { NULL, NULL };
    QUEUE data = { 0, &list };
    NODE *ret = NULL;
    int result = 0;

    list.content = (QUEUE *) & data;

    ra_updateDataStructure();
    result = ra_traverseTree(id, 0, flag, node, &list, 1, ra_status);
    if (result == -1 || result == -2) {
	setRaStatus(ra_status, RA_RC_FAILED, FAILED_TO_GET_SYSTEM_RESOURCE,
		    "Failed to get Entity");
	return NULL;
    }

    ret = (NODE *) list.next->content;
    free(list.next);

    return ret;
}

NODE **ra_getAllEntity(int type, NODE * node, _RA_STATUS * ra_status)
{
    int flag = TYPEONLY | TYPE_OR;
    LIST list = { NULL, NULL };
    QUEUE data = { 0, &list };
    NODE **ret = NULL;
    int result = 0;

    list.content = (QUEUE *) & data;
    if (type & PARAMSF) {
	flag = TYPEONLY | TYPE_AND;
	type |= SUPPORTF;
    }

    ra_updateDataStructure();
    result = ra_traverseTree(0, type, flag, NULL, &list, 1, ra_status);
    if (result == -1 || result == -2) {
	//setRaStatus( ra_status, RA_RC_FAILED, FAILED_TO_GET_SYSTEM_RESOURCE, "Failed to get Entity");
	return NULL;
    }

    ret = ra_convertToArray(&list);

    return ret;
}

char *formString(int length, char *tag, unsigned long long value,
		 char *suffix)
{
    char *array;
    array = (char *) calloc(length, sizeof(char));

    if (value)
	sprintf(array, "%s%llx%s", tag, value, suffix);
    else
	sprintf(array, "%s%s", tag, suffix);

    return array;
}

char *ra_formInstID(NODE * this, char *suffix, char *class)
{
    int length = 0;
    char *ret = NULL, *global;
    if (!suffix) {
	suffix = (char *) malloc(sizeof(char));
	suffix[0] = '\0';
    }

    switch (this->obFlags & DECL_MASK) {
    case SERVICEF:
	length = 27 + strlen(class) + strlen(suffix);
	global = (char *) calloc((23 + strlen(class)), sizeof(char));
	sprintf(global, "WBEM-SMT:%s:Service=dhcp", class);
	free(suffix);
	return global;
	break;
    case SERCONFF:
	length = 40 + strlen(class) + strlen(suffix);
	global = (char *) calloc((36 + strlen(class)), sizeof(char));
	sprintf(global, "WBEM-SMT:%s:ServiceConfiguration=dhcp", class);
//          ret = formString(length, global, this->obID, suffix);
	free(suffix);
	return global;
	break;
    case GLOBALF:
	length = 31 + strlen(class) + strlen(suffix);
	global = (char *) calloc((27 + strlen(class)), sizeof(char));
	sprintf(global, "WBEM-SMT:%s:Global=0", class);
	ret = formString(length, global, this->obID, suffix);
	free(suffix);
	return ret;
	break;
    case PARAMSF:
	if (this->obFlags & OPTIONF) {
	    length = 9 + 16 + strlen(suffix);
	    ret = formString(length, ":Option=", this->obID, suffix);
	} else {
	    length = 8 + 16 + strlen(suffix);
	    ret = formString(length, ":Param=", this->obID, suffix);
	}
	free(suffix);
	ret = ra_formInstID(this->parent, ret, class);
	break;
    case SUBNETF:
	length = 9 + 16 + strlen(suffix);
	ret = formString(length, ":Subnet=", this->obID, suffix);
	free(suffix);
	ret = ra_formInstID(this->parent, ret, class);
	break;
    case SHAREDNETF:
	length = 12 + 16 + strlen(suffix);
	ret = formString(length, ":Sharednet=", this->obID, suffix);
	free(suffix);
	ret = ra_formInstID(this->parent, ret, class);
	break;
    case HOSTF:
	length = 7 + 16 + strlen(suffix);
	ret = formString(length, ":Host=", this->obID, suffix);
	free(suffix);
	ret = ra_formInstID(this->parent, ret, class);
	break;
    case GROUPF:
	length = 8 + 16 + strlen(suffix);
	ret = formString(length, ":Group=", this->obID, suffix);
	free(suffix);
	ret = ra_formInstID(this->parent, ret, class);
	break;
    case CLASSF:
	length = 8 + 16 + strlen(suffix);
	ret = formString(length, ":Class=", this->obID, suffix);
	free(suffix);
	ret = ra_formInstID(this->parent, ret, class);
	break;
    case POOLF:
	length = 7 + 16 + strlen(suffix);
	ret = formString(length, ":Pool=", this->obID, suffix);
	free(suffix);
	ret = ra_formInstID(this->parent, ret, class);
	break;
    default:
	break;
    }
    return ret;
}

char *ra_instanceId(NODE * node, char *class)
{
    return ra_formInstID(node, NULL, class);
}

unsigned long long ra_getKeyFromInstance(char *inst)
{
    char *c;
    unsigned long long ret;

    for (c = inst; *c; c++);
    for (; *c != '='; c--);
    ret = ra_convertToID(++c);
    return ret;
}

int ra_findLevel(const char *inst)
{
    int level = 0;
    char *cptr = (char *) inst;
    for (; *cptr; cptr++)
	if (*cptr == ':')
	    level++;

    return level - 2;
}

char *ra_tokenize(char *inst)
{
    static char *c = NULL, *start = NULL, *end = NULL, *ret = NULL;
    int count;
    if (inst) {
	start = end = ret = NULL;
	for (c = inst; *c; c++);
    }

    end = c + 1;
    for (count = 1; *c != ':'; c--, count++);
    ret = (char *) calloc(count + 1, sizeof(char));
    start = c--;

    for (count = 0; start != end; start++, count++)
	ret[count] = *start;

    ret[count] = '\0';

    return ret;
}


char *ra_get_hostname()
{
    char *name = NULL, *host_name = NULL;
    char namearry[100];
    size_t name_length = 100;
    int result = -1;

    name = (char *) malloc(sizeof(char *));
    result = gethostname(namearry, name_length);
    name = strdup(namearry);
    host_name = strdup(name);
    free(name);

    return host_name;
}

char *ra_removeQuotes(char *string)
{
    char *ptr = NULL;
    char *str = strdup(string);

    while (((ptr = strpbrk(str, "\"")) != NULL) && *str)
	strcpy(ptr, ptr + 1);

    return str;
}
