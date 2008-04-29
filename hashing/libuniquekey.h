///====================================================================
/// libuniquekey.h
///
/// © Copyright IBM Corp. 2007
///
/// THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
/// ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
/// CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
///
/// You can obtain a current copy of the Common Public License from
/// http://www.opensource.org/licenses/cpl1.0.php
///
/// Authors :	    Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
///
///====================================================================


/** The list of routines that are available from this library implementation */
int getUniqueKey(const char * , int , unsigned long long * );
void modifiedEntity(char * );
int addedEntity(char * );
void deletedEntity(char * );
void resetFileData(char * );
void setFileData(char * );
unsigned long long * getAllUniqueKey(char *);
