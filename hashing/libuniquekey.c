///=====================================================================
/// libuniquekey.c
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
/// Authors :	    Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
///
///=====================================================================
 

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef DEBUG1
#define LOG(s,t)    printf("%s: ......... %s\n", s, t);
#else
#define LOG(s,t)
#endif

#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif

/** The structure _UQ_linkedList is a linked list. Each node in this list would carry */
/** the line number and the line itself from a file that this list points to.  */
typedef struct _UQ_linkedList{
    int lineno;
    char * line;
    struct _UQ_linkedList * nextline;
} _UQ_linkedList;

/** Method to delete all the nodes from a list following the node pointer passed as the argument to the method */
void _UQ_delLinkedList(_UQ_linkedList * list)
{
    LOG("delLinkedList", "Entered");
    _UQ_linkedList * temp = NULL;
    /** when the passed list pointer is the only node in the list */
    if(!list->nextline){
	free(list->line);
	free(list);
    }
 
    /** when there exist more nodes as a link list with the node passed as the argument being the head node, delete the entire list */
    for(temp = list->nextline; temp; temp = temp->nextline)
    {
	free(list->line);
	free(list);
	list = temp;
    }
    LOG("delLinkedList", "Exit");
}

/** Method to return a hexadecimal value represented by the string argument passed to the function*/
unsigned long long _UQ_strToHex(char * buf)
{
    unsigned long long val = 0;
    char * str;

    for (str = buf; isxdigit(*str) ; str++) {
	    val <<= 4;
	    if (isalpha(*str))
		    val |= (int) (*str) - 'a' + 10;
	    else if (isdigit(*str))
		    val |= (int) (*str) - '0';
    }

    return val;
}

/** Method to return a decimal value represented by the string argument passed to the function*/
unsigned long long _UQ_strToDec(char * buf)
{
    unsigned long long val = 0;
    char * str;

    for (str = buf; isdigit(*str) ; str++) {
	    val *= 10;
	    val += (int) (*str) - '0';
    }

    return val;
}


/** Method used to arrive at the file name from the file name specified as a path */
/** Method accepts the path to the file as the argument and returns the filename */
char * _UQ_extractServiceName(char * filepath)
{
	char * ret = NULL;
	char * sptr = filepath;

	LOG("extractServiceName", filepath);

	for( ; *sptr ; sptr++);
	for( ; *sptr != '/'; sptr--);

	ret = strdup(++sptr);
	return ret;
}

/** Method used to arrive at a key value to represent every line in the config file */
/** accepts the seed as an argument which is the filename for which the keys are getting generated
and returns a unsigned long long value as the key after manipulation with a predetermined constant */
unsigned long long _UQ_initialiseKey(char * seed)
{
	unsigned long long key = 0xa1b2c3d4;
	char * str = seed;

	for(;*str; str++)
		key = key * (int)(*str);

	return key;
}
	
/** Method used to generate the required files to support the technical key generation and manipulation */
/** expects the filepath, the name of the file, the name of the file to hold the copy of the config file 
    and the name of the file to hold the keys as arguments, returns nothing */
void _UQ_setupFiles(char * filepath, char * filename, char * wbemname, char * keyname)
{
	char * line = NULL, idfile[100];
	size_t len = 0;
	ssize_t read = 0;
	unsigned long long keyvalues = _UQ_initialiseKey(filename); /** the keyvalue is initialized */
	FILE * fconf = NULL, * fwbem = NULL, * fkey = NULL;  /** file pointers for different files */

	LOG("setUpFiles", "Entered");

	fconf = fopen(filepath, "r"); /** fconf - the config file is to be opened in the read mode */
	fwbem = fopen(wbemname, "w"); /** fwbem - the file to retain the copy of the config file to be opened in write mode */
	fkey  = fopen(keyname, "w");  /** fkey -the file to maintain the key values to be opened in write mode */

	LOG("setUpFiles", "Copying");
	
        /** loop reads the contents from the fconf file, if the read function is successful
            copies the contents to the same line in the fwbem file, copies the keyvalue earlier generated to the 
            fkey file and increments the keyvalue.
            This continues till the entire fconf file is read and a copy is maintained in the fwbem file and
            also a key is generated for every line and maintained in the fkey file */
	while( (read = getline(&line, &len, fconf)) != -1){
		fprintf(fwbem, line);
		fprintf(fkey,"%llx\n",++keyvalues?keyvalues:++keyvalues);
		if(line)
		    free(line);
		line = NULL;
	}

	LOG(filepath, wbemname);

	if(line)
	    free(line);

        /** A file is created to store the final key value so that it stores the next value to start */
	sprintf(idfile,SYSCONFDIR "/UniqueKey/.%s.Id", filename);

	fclose(fconf);
	fclose(fwbem);
	fclose(fkey);
	fconf = fopen(idfile, "w");
	fprintf(fconf,"%llx\n", ++keyvalues? keyvalues: ++keyvalues);
	fclose(fconf);
}

/** Method used to convert the contents of the file passed as argument to a linked list with each node of type _UQ_linkedList
    holding the linenumber, the line string and a pointer for the next node */
_UQ_linkedList * _UQ_fileToLinkedList(char * file)
{
    char * line = NULL; 
    size_t len = 0;
    ssize_t read;
    FILE * fd;
    int i = 1;
    _UQ_linkedList * ret = NULL, * list = NULL;

    LOG("fileToLinkedList",  file);

    ret = (_UQ_linkedList *)malloc(sizeof(_UQ_linkedList));
    memset(ret, '\0', sizeof(_UQ_linkedList));
    list = ret;

    fd = fopen(file, "r"); /** open the file to be read */
    /** populate the linked list and stop when the EOF is met*/
    while( (read = getline(&line, &len, fd )) != -1) {
	list->lineno = i++;
	list->line = strdup(line);
	list->nextline = (_UQ_linkedList *)malloc(sizeof(_UQ_linkedList));
	memset(list->nextline, '\0', sizeof(_UQ_linkedList));
	list = list->nextline;
	list->nextline = NULL;
	if(line)
	    free(line);
	line = NULL;
    }

    if(line)
	free(line);

    fclose(fd); /** close the file */

    return ret;
}

/** A method to insert new keys into the the positions mentioned using i and j lines in the .key file*/
void _UQ_insertEntity(char * filename, _UQ_linkedList * lptr, int i, int j)
{
    char idFile[50], value[20], *cptr;
    unsigned long long key;
    FILE * fd;
    int count = j - i + 1;

    LOG("insertEntity", "Entered");
    _UQ_linkedList * first = NULL, * temp = NULL, * prev = NULL; /** pointers to traverse the linked list */

    first = (_UQ_linkedList *)malloc(sizeof(_UQ_linkedList));
    memset(first, '\0', sizeof(_UQ_linkedList));

    sprintf(idFile,SYSCONFDIR "/UniqueKey/.%s.Id",filename); /** get the Id file */

    fd = fopen(idFile, "r"); /** Read the Id file to retrieve the value of the key stored */
    cptr = fgets(value, 19, fd);
    fclose(fd);

    key = _UQ_strToHex(value);

    /** forms the list of new ids to be inserted */
    for(temp = first; count; count--){
	temp->lineno = 0;
	sprintf(value,"%llx\n", ++key? key: ++key);
	temp->line = strdup(value);
	temp->nextline = (_UQ_linkedList *)malloc(sizeof(_UQ_linkedList));
	memset(temp->nextline, '\0', sizeof(_UQ_linkedList));
	prev = temp;
	temp = temp->nextline;
    }
    fd = fopen(idFile, "w");
    fprintf(fd,"%llx\n", key);
    fclose(fd);

    /** inserts the new list into the existing file list which is available in lptr. */
    prev->nextline = lptr->nextline;
    lptr->nextline = first;
    LOG("insertEntity", "Exit");
}

/** A method to generate a new set of keys for the conf files which would be stored in the .key file */
void _UQ_reCreateKeys(char * filename,char * conffile, char * wbemfile, char *keyfile){
	char idFile[50], value[20], * line = NULL;
	unsigned long long key;
	size_t len = 0;
	ssize_t read = 0;
	FILE *fd = NULL, * fconf = NULL, * fwbem = NULL, * fkey = NULL;
	LOG("reCreateKeys", "Entered");

	/** picks up the seed from the .Id file */
	sprintf(idFile, SYSCONFDIR "/UniqueKey/.%s.Id", filename);
	fd = fopen(idFile, "r");
	line = fgets(value, 19, fd);
	line = NULL;
	fclose(fd);

	key = _UQ_strToHex(value);

	fconf = fopen(conffile, "r");
	fwbem = fopen(wbemfile, "w");
	fkey  = fopen(keyfile, "w");
	
	/** generates a new .key file using the seed got earlier. */
	while( (read = getline(&line, &len, fconf)) != -1){
		fprintf(fwbem, line);
		fprintf(fkey,"%llx\n",++key?key:++key);
		if(line)
		    free(line);
		line = NULL;
	}

	fclose(fconf);
	fclose(fwbem);
	fclose(fkey);

	if(line)
	    free(line);

	fd = fopen(idFile, "w");
	fprintf(fd, "%llx\n", key);
	fclose(fd);
	LOG("reCreateKey", "Exit");
}

/** A method to remove the lines between i and j from the .key file */
void _UQ_deleteEntity(char * filename, _UQ_linkedList * lptr, _UQ_linkedList * prev, int i, int j)
{
    LOG("deleteEntity", "Entered");
    _UQ_linkedList * temp;
#ifdef DEBUG1
    printf("Line Start %d, End %d: List Start %d\n", i, j, lptr->lineno);
#endif
    for(temp = lptr; temp->lineno != j; temp = temp->nextline);
#ifdef DEBUG1
    printf("The last line %d\n", temp->lineno);
#endif
    prev->nextline = temp->nextline;
    temp->nextline = NULL;
    _UQ_delLinkedList(lptr);
    LOG("deleteEntity", "Exit");
}

/** Method used to update the different files on which the hashing mechanism depends on.  This is invoked if any changes
    are observed in the config file - due to manual modifications, adding or deleting any entities of conf file */
int _UQ_upToDate(char * filename, char * conffile, char * wbemfile, char * keyfile, char * tmpfile, char * seed)
{
    char * line, l1[5], l2[5], l3[5], l4[5], comm;
    int i, i1, i2, i3, i4, ret = 0;
    _UQ_linkedList tmplist, keylist, *lptr, *alptr, *dlptr, *prelptr = NULL;
    FILE * fd, *fconf, *fwbem;
    size_t len = 0;
    ssize_t read = 0;
    LOG("upToDate", "Entered");
    tmplist.lineno = 0;
    keylist.lineno = 0;

    tmplist.nextline = (_UQ_linkedList *)malloc(sizeof(_UQ_linkedList));
    memset(tmplist.nextline, '\0', sizeof(_UQ_linkedList));
    keylist.nextline = (_UQ_linkedList *)malloc(sizeof(_UQ_linkedList));
    memset(keylist.nextline, '\0', sizeof(_UQ_linkedList));

    keylist.nextline = _UQ_fileToLinkedList(keyfile);
    tmplist.nextline = _UQ_fileToLinkedList(tmpfile);

    /** extracting the variations that have occurred in the conf file */
    for(lptr = tmplist.nextline; lptr->nextline; lptr = lptr->nextline){
	if(!isdigit((int)(lptr->line[0])))
	    continue;

	line = lptr->line;

	for(i = 0; isdigit(*line) ; line++)
	    l1[i++] = *line;

	l1[i] = 0;
	i1 = _UQ_strToDec(l1);

	if(*line == ','){
	    for(i = 0, line++; isdigit(*line) ; line++)
		l2[i++] = *line;

	    l2[i] = 0;
	    i2 = _UQ_strToDec(l2);
	} else 
	    i2 = i1;
	comm = *line;

	for(i = 0, line++; isdigit(*line) ; line++)
	    l3[i++] = *line;

	l3[i] = 0;
	i3 = _UQ_strToDec(l3);

	if(*line == ','){
	    for(i = 0, line++; isdigit(*line) ; line++)
		l4[i++] = *line;

	    l4[i] = 0;
	    i4 = _UQ_strToDec(l4);
	} else
	    i4 = i3;

	switch(comm){
	    case 'a':	/** Adding some new keys in the key file for the additions observed in the conf file */ 
		LOG("File Diff", " Something Added");
		for(alptr = &keylist; alptr->lineno != i1; alptr = alptr->nextline);
		_UQ_insertEntity(filename, alptr, i3, i4);
		ret = i4;
		break;
	    case 'c':
		LOG("File Diff", "Something Changed");
		_UQ_reCreateKeys(filename, conffile, wbemfile, keyfile);
		ret = -1;
		goto exit;
		break;
	    case 'd': /** Removing some lines from the .key file for the deletions observed in the conf file */
		LOG("File Diff", "Something Deleted");
		for(dlptr = &keylist; dlptr->lineno != i1; dlptr = dlptr->nextline)
		    prelptr = dlptr;
		_UQ_deleteEntity(filename, dlptr, prelptr, i1, i2);
		ret = 0;
		break;
	}
    }

    fd = fopen(keyfile, "w");
    for(lptr = keylist.nextline; lptr->nextline; lptr = lptr->nextline)
	fprintf(fd, "%s", lptr->line);
    fclose(fd);

    fconf = fopen(conffile, "r");
    fwbem = fopen(wbemfile, "w");
    
    LOG("Coping", conffile);
    while( (read = getline(&line, &len, fconf)) != -1){
	    fprintf(fwbem, line);
	    if(line)
		free(line);
	    line = NULL;
    }
    LOG("Copied into", wbemfile);

    if(line)
	free(line);

    fclose(fconf);
    fclose(fwbem);

exit:

    _UQ_delLinkedList(keylist.nextline);
    _UQ_delLinkedList(tmplist.nextline);
    LOG("updateToDate", "Exit");

    return ret;
}

/** A method to get the key stored in the lineno line of the .key file */
unsigned long long _UQ_getKeyFromData(char * file, int lineno)
{
	char * linebuf = NULL;
	size_t len = 0;
	ssize_t read = 0;
	FILE * fd = NULL;
	unsigned long long val = 0;
	LOG("getKeyFromData", "Entered");

	if(lineno <= 0)
		return 0;

	fd = fopen(file, "r");
	if(fd == NULL)
		return 0;

	while(lineno--)
		read = getline(&linebuf, &len, fd);

	fclose(fd);
	val = _UQ_strToHex(linebuf);
	if(linebuf)
		free(linebuf);

	LOG("getKeyFromData", "Exit");

	return val;
}

/** The primary routine that serves as the  wrapper routine that are called from outside */
int getUniqueKey(char * filepath, int lineno, unsigned long long * uniqueKey)
{
	char * filename, wbemname[100], keyname[100], tmpfile[100];
	int status, i, ret = 0;
	pid_t pId;

	LOG("getUniqueKey", "Entered");

	if(lineno <= 0)
	    return 0;

	/** if the said directory does not exist, create one - access() checks the users permissions for the specified file */
        if(access(SYSCONFDIR "/UniqueKey",F_OK))
		mkdir(SYSCONFDIR "/UniqueKey", 0777);
	
	filename = _UQ_extractServiceName(filepath); /** derive the filename */
	sprintf( wbemname,SYSCONFDIR "/UniqueKey/.%s.wbem",filename); /** create the names of the files preceeding with a . */
	sprintf( keyname,SYSCONFDIR "/UniqueKey/.%s.key",filename);
	sprintf(tmpfile,SYSCONFDIR "/UniqueKey/.%s.tmp",filename);

	if( access(filepath, R_OK))  /** check the users permissions to read the conf file */
		return -1;

	if(access(wbemname, W_OK) && access(keyname, W_OK)) /** check for users permissions to open the specified files in write mode */
	{
		_UQ_setupFiles(filepath, filename, wbemname, keyname); /** copy the detais necessary to the files 
  									   copy of conf file in wbemname file and
									   keyvalues in the keyname file */
	}
	else
	{
		struct stat buf;
		pId = fork();
		if(pId)
		{
		    LOG("getUniqueKey", "Forked - in the parent process");
			wait(&status);
		}
		else
		{
		    LOG("getUniqueKey", "Forked - in the child process");
			for (i=getdtablesize();i>=0;--i)  /** close all the opened files for this process */
				close(i); 
			i=open(tmpfile, O_RDWR|O_CREAT|O_TRUNC, 0600);
			i = dup(i); 
			execlp("diff","diff", wbemname, filepath,(char *) 0); /** check if there exists any differences
										  between the copy of the config file and the
										  orignal config file */
			close(i);
			LOG("getUniquekey", "Submitted the file difference");
		}
		stat(tmpfile, &buf); /** check the status of the file to verify if any differences were found */
                                     /** if differences were found, update the wbemname and keyname file to 
 					 reflect the changes */
		if(buf.st_size){
		    LOG("getUniqueKey", "Found Some diff in files");
		    ret = _UQ_upToDate(filename, filepath, wbemname, keyname, tmpfile, filename);
		}
		
	}

	free(filename);
	(*uniqueKey) = _UQ_getKeyFromData(keyname, lineno); /** arrive at the key for the specified line */ 
	LOG("getUniqueKey", "Exit");
	return ret;
}

/** A wrapper routine that forms an array out of the .key file and returns the array */
unsigned long long * getAllUniqueKey(char * filepath)
{
    unsigned long long hashId, * hashIdArray;
    int fsize = 0;
    char * linebuf = NULL, * filename = NULL, keyfile[100];
    struct stat fileStat;
    FILE * fileD = NULL;
    size_t len = 0;
    ssize_t read = 0;
    LOG("getAllUniqueKey", "Entered");


    getUniqueKey(filepath, 1, &hashId);
    filename = _UQ_extractServiceName(filepath); /** get the conf file name */
    sprintf( keyfile,SYSCONFDIR "/UniqueKey/.%s.key",filename); /** create the keyfile depending on the conf file name */

    stat(keyfile, &fileStat);      /** get the status and size of keyfile */
    fsize = (int)fileStat.st_size;

    hashIdArray = (unsigned long long *)calloc((fsize/17 + 5), sizeof(unsigned long long)); 
    fileD = fopen(keyfile, "r");
    if(keyfile == NULL) 
	return NULL;

    for(fsize = 0; ((read = getline(&linebuf, &len, fileD)) != -1); fsize++)
	hashIdArray[fsize] = _UQ_strToHex(linebuf); /** prepare the hexstring and add it to the hashIdArray */

    LOG("getAllUniqueKey", "Prepared the hash table");

    fclose(fileD);

    if(linebuf)
	free(linebuf);

    free(filename);
    return hashIdArray;
}

/** A method invoked when the modification done to the conf file was through the provider code */
void modifiedEntity(char * filePath)
{
	char * filename, wbemname[100], *line = NULL;
	FILE * fconf = NULL, * fwbem = NULL;
	size_t len;
	ssize_t read;
	LOG("modifEntity", "Entered");

	filename = _UQ_extractServiceName(filePath); /** get the conf file name from the path */
	sprintf( wbemname,SYSCONFDIR "/UniqueKey/.%s.wbem",filename); /** prepare the wbemname to retain a copy of the conf file */

	fconf = fopen(filePath, "r");  /** open conf file in read and wbemname file in write mode */
	fwbem = fopen(wbemname, "w");
	
        /** copy the entire contents of the conf file into the wbemname file */
	while( (read = getline(&line, &len, fconf)) != -1){
		fprintf(fwbem, line);
		if(line)
		    free(line);
		line = NULL;
	}

	fclose(fwbem); /** close both conf and wbemname files */
	fclose(fconf);

	if(line)
	    free(line); /** free all memory allocated */

	free(filename);
	LOG("modifyEntity", "Exit");
}

/** method invoked when anything is added to the conf file */
int addedEntity(char * filePath)
{
    unsigned long long key;
    LOG("addEntity", "Entered");

    return getUniqueKey(filePath, 1, &key);
}

/** Method invoked when anything is deleted from the conf file */
void deletedEntity(char * filePath)
{
    unsigned long long key;
    LOG("deleteEntity", "Entered");

    getUniqueKey(filePath, 1, &key);
}

/** Method to remove all the files created for the purpose of supporting the hashing mechanism */
/** Used in cleanup routine */
void resetFileData(char * filePath)
{
    char *filename, wbemfile[50], tmpfile[50], keyfile[50], idfile[50];

    LOG("resetFileData", "Entered");
    filename = _UQ_extractServiceName(filePath); /** arrive at the filename of the service conf file
						     set the respective file names of files created to support hashing
						     delete all the files created */

    sprintf( wbemfile,SYSCONFDIR "/UniqueKey/.%s.wbem",filename);
    sprintf( keyfile,SYSCONFDIR "/UniqueKey/.%s.key",filename);
    sprintf( tmpfile,SYSCONFDIR "/UniqueKey/.%s.tmp",filename);
    sprintf( idfile, SYSCONFDIR "/UniqueKey/.%s.Id", filename);

    free(filename);
    remove(wbemfile);
    remove(tmpfile);
    remove(keyfile);
    remove(idfile);

}

/** A dummy wrapper routine that can be called to initialise the files used for a particular conf file */
void setFileData(char * filePath)
{
    unsigned long long key;
    LOG("SetFileData", "Entered");
    getUniqueKey(filePath, 1, &key);
}
