/*
 * dhcpd.conf.parser.y
 *
 * © Copyright IBM Corp. 2007
 *
 * THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 * ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 * CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 *
 * You can obtain a current copy of the Eclipse Public License from
 * http://www.opensource.org/licenses/eclipse-1.0.php
 *
 * Authors : Ashoka Rao.S <ashoka.rao (at) in.ibm.com>
 *           Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
 *
 *
 */

%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ra-support.h"
#include "provider-support.h"
#include "libuniquekey.h"

#ifdef DEBUG1
#define LOG1(s)  printf("[%d]\t\t:%s .\n", conflineno-1, s);
#define LOG2(s,t)  printf("[%d]\t\t:%s %s.\n", conflineno-1, s,t);
#define LOG3(s,t,u)  printf("[%d]\t\t:%s %s %s.\n", conflineno-1, s,t,u);    
#else
#define LOG1(s)
#define LOG2(s,t)
#define LOG3(s,t,u)
#endif

extern FILE *confin;
extern FILE *confout;
extern int conflex(void); 
extern int conferror(char *);
extern void init_lex(void); 
extern void confrestart(FILE *); 
extern int confwrap(void ); 
extern int cleanup_lex(void);

NODE *error_node = NULL;
extern int conflineno; 

char * comment_string = NULL;
char * conffile = NULL;
unsigned long long * hashIDArray = NULL;
int current_state = 0;
int ipListBegin = 0;
int elseBegin = 0;
%}


%name-prefix="conf"
%expect 1

%union { 
    int dig;   
    char * string;
    struct _LIST * list;
    struct _NODE * node;
}

%token <string> GROUP SHARED_NETWORK SUBNET HOST CLASS POOL SUBCLASS
%token <string> BOOLEAN NUMBER STRING COLON_STRING IPADDR MACADDR HARDWARE_TYPE DDNS_UPDATE_STYLE_TYPE BOOLEAN_OPERATOR
%token <string> GENERIC_NAME FILE_NAME HOST_NAME PATH_NAME 
%token <string> ALLOW DENY IGNORE MATCH SPAWN WITH IF ELSIF ELSE MEMBERS OF  
%token <string> NETMASK ALL KNOWN CLIENTS UNKNOWN_CLIENTS KNOWN_CLIENTS IFKNOWN DUPLICATES  
%token <string> OPTION OPTION_TYPE DDNS_UPDATE_STYLE SERVER_IDENTIFIER DEFAULT_LEASE_TIME MAX_LEASE_TIME GROUP_IDENTIFIER
%token <string> GET_LEASE_HOSTNAMES DYNAMIC_BOOTP_LEASE_LENGTH BOOT_UNKNOWN_CLIENTS RANGE USE_HOST_DECL_NAMES  
%token <string> FILENAME SERVERNAME NEXTSERVER HARDWARE FIXEDADDRESS
%token <string> LBRACE RBRACE SEMICOLON COMMA LPARANTHESIS RPARANTHESIS EQUALS NEWLINE
%token <string> SUBSTRING SUFFIX CONFIG_OPTION PACKET CONCAT REVERSE LEASED_ADDRESS BINARY_TO_ASCII ENCODE_INT 
%token <string> PICK_FIRST_VALUE HOST_DECL_NAME EXISTS STATIC EXTRACT_INT LEASE_TIME LIMIT LEASE
%token <string> DATE UNRECOGNISED UNSUPPORTED COMMENT 

%type  <string> error

%type  <node> dhcp_config_file parameters comment declaration entities
%type  <node> subnet sharednet group class host pool parameter
%type  <node> if_statement 

%type  <list> ipaddr_lists elsifs elsif

%start dhcp_config_file 

%%

dhcp_config_file:   dhcp_config_file declaration
		{
		    LOG1("dhcp conf decl");
		    $$ = ra_dropChild($1, $2);
		}
		| dhcp_config_file parameter 
		{ 
		    LOG1("dhcp conf parameter");
		    $$ = ra_dropChild($1,$2); 
		}
		| dhcp_config_file comment 
		{ 
		    LOG1("dhcp conf comment");
		    $$ = ra_dropChild($1,$2); 
		}
		| /* empty */ 
		{
		    LOG1("dhcp conf empty");
		    $$ = dhcp_conf_tree;
		}
                ;

entities :	  entities declaration
		{
		   // LOG1("entities.. decl");
		    $$ = ra_appendNode($1, $2);
		}
		| entities parameter
		{
		    LOG1("entities.. params");
		    $$ = ra_appendNode($1, $2);
		}
		| entities comment
		{
		    LOG1("entities.. comments");
		    $$ = ra_appendNode($1, $2);
		}
		| /* empty */
		{
		    LOG1("entities ... empty");
		    $$ = ra_createDummy(); 
		}
		;

parameters: parameters parameter 
		{ 
		    LOG1("recursive params");
		    $$ = ra_appendNode($1, $2); 
		}
		| /* empty */ 
		{
		    LOG1("params empty ");
		    $$ = ra_createDummy();
		}
          ; 

declaration:  subnet
		{ 
		    LOG1("decl:subnet");
		    $$ = $1; 
		}
	    |  sharednet
		{ 
		    LOG1("decl:sharednet");
		    $$ = $1; 
		}
	    |  group
		{ 
		    LOG1("decl:group");
		    $$ = $1; 
		}
	    |  class
		{ 
		    LOG1("decl:class");
		    $$ = $1; 
		}
	    |  host
		{ 
		    LOG1("decl:host");
		    $$ = $1; 
		}
	    | pool
		{ 
		    LOG1("decl:pool");
		    $$ = $1; 
		}
	    | error LBRACE NEWLINE
		{
		    LOG1("LBRACE Error");
		    $$ = error_node;
		    free($3);
		    yyerrok;
		}
	    | error RBRACE NEWLINE
		{  
		    LOG1("RBRACE Error");
		    $$ = error_node;
		    free($3);
		    yyerrok;
		} 
           ; 

subnet: SUBNET IPADDR NETMASK IPADDR LBRACE NEWLINE entities RBRACE NEWLINE
        {
	    NODE * current = NULL;
	    LOG3($1, $2, $4);

	    current = ra_createSubnet($2, $4, hashIDArray[conflineno - 2]);
	    ra_addRight(current, $7);
	    free($1);free($3);free($6);free($9);
	    $$ = current;

        }; 

sharednet: SHARED_NETWORK GENERIC_NAME LBRACE NEWLINE entities RBRACE NEWLINE
         {
	     NODE * current = NULL;
	     LOG2($1, $2);

	     current = ra_createSharedNet($1, $2, hashIDArray[conflineno - 2]);
	     ra_addRight(current, $5);
	     free($4);free($7);
	     $$ = current;
	 };

group: GROUP LBRACE NEWLINE entities RBRACE NEWLINE
	{
	     NODE * current = NULL;
	     LOG1($1);

	     current = ra_createGroup($1, hashIDArray[conflineno - 2]);
	     ra_addRight(current, $4);
	     free($3);free($6);
	     $$ = current;
	
	};

comment: COMMENT
	{
	    LOG1($1);
	    $$ = ra_createComment($1);
	};

pool: POOL LBRACE NEWLINE entities RBRACE NEWLINE
	{
	     NODE * current = NULL;
	     LOG1($1);

	     current = ra_createPool($1, hashIDArray[conflineno - 2]);
	     ra_addRight(current, $4);
	     free($3);free($6);
	     $$ = current;
	};

class: CLASS STRING LBRACE NEWLINE parameters RBRACE NEWLINE
	{
	     NODE * current = NULL;
	     LOG1($1);

	     current = ra_createClass($1, $2, hashIDArray[conflineno - 2]);
	     ra_addRight(current, $5);
	     free($4);free($7);
	     $$ = current;
	};


host: HOST GENERIC_NAME LBRACE NEWLINE entities RBRACE NEWLINE
	{
	     NODE * current = NULL;
	     LOG2($1, $2);

	     current = ra_createHost($1, $2, hashIDArray[conflineno - 2]);
	     ra_addRight(current, $5);
	     free($4);free($7);
	     $$ = current;
	};

parameter: OPTION OPTION_TYPE NUMBER SEMICOLON NEWLINE
              {
		     int flag;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     free($1);free($5);
		     $$ = ra_createParam($2, $3, flag, hashIDArray[conflineno - 2]);
              }
	  | OPTION OPTION_TYPE IPADDR SEMICOLON NEWLINE  
              {
		     int flag;
		     LOG3($1, $2, $3);
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     free($1);free($5);
		     $$ = ra_createParam($2, $3, flag, hashIDArray[conflineno - 2]);
              }
          | OPTION OPTION_TYPE BOOLEAN SEMICOLON NEWLINE
              {
		     int flag;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     free($1);free($5);
		     $$ = ra_createParam($2, $3, flag, hashIDArray[conflineno - 2]);
              }
          | OPTION OPTION_TYPE IPADDR ipaddr_lists SEMICOLON NEWLINE
              {
        	     int flag;
		     char * value;
		     LIST * list, *lptr;
		     LOG3($1, $2, $3); 
		     
		     list = ra_genListNode($3);
		     list->next = $4;
		     flag = SUPPORTF | PARAMSF | OPTIONF ;
		     flag |= ipListBegin? COMMAF:0;
		     ipListBegin = 0;
		     value = ra_multiValuedString(list, 1);
		     for(lptr = list->next; lptr != NULL; lptr = lptr->next){
			 if(list->content)
			     free(list->content);
			 free(list);
			 list = lptr;
		     }
		     free($1);free($6);
		     $$ = ra_createParam($2, value, flag, hashIDArray[conflineno - 2]);
              }

          | OPTION UNRECOGNISED IPADDR ipaddr_lists SEMICOLON NEWLINE
	      {
               	     int flag;
		     char * value;
		     LIST * list, *lptr;
		     LOG3($1, $2, $3);
		     
		     list = ra_genListNode($3);
		     list->next = $4;
		     flag = UNSUPPORTF | PARAMSF | OPTIONF ;
		     flag |= ipListBegin? COMMAF: 0;
		     ipListBegin = 0;
		     value = ra_multiValuedString(list, 1);
		     for(lptr = list->next; lptr != NULL; lptr = lptr->next){
			 if(list->content)
			     free(list->content);
			 free(list);
			 list = lptr;
		     }
		     free($1);free($6);
		     $$ = ra_createParam($2, value, flag, 0);
              }
          | OPTION OPTION_TYPE IPADDR IPADDR SEMICOLON NEWLINE
              {
               	     int flag;
		     char * value;
		     LIST * list, *lptr;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     list = ra_genListNode($3);
		     list = ra_appendToList(list, $4);
		     value = ra_multiValuedString(list, 0);
		     for(lptr = list->next; lptr != NULL; lptr = lptr->next){
			 if(list->content)
			     free(list->content);
			 free(list);
			 list = lptr;
		     }
		     free($1);free($6);
		     $$ = ra_createParam($2, value, flag, hashIDArray[conflineno - 2]);
              }
          | OPTION OPTION_TYPE STRING SEMICOLON NEWLINE
              {
		     int flag;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     free($1);free($5);
		     $$ = ra_createParam($2, $3, flag, hashIDArray[conflineno - 2]);
              }
          | OPTION OPTION_TYPE HOST_NAME SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     free($1);free($5);
		     $$ = ra_createParam($2, $3, flag, hashIDArray[conflineno - 2]);
              } 
          | DDNS_UPDATE_STYLE DDNS_UPDATE_STYLE_TYPE SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | SERVER_IDENTIFIER HOST_NAME SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | DEFAULT_LEASE_TIME NUMBER SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | MAX_LEASE_TIME NUMBER SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | GET_LEASE_HOSTNAMES BOOLEAN SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1 , $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | DYNAMIC_BOOTP_LEASE_LENGTH NUMBER SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | UNSUPPORTED BOOLEAN SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED SEMICOLON NEWLINE
              {
		     int flag;
		     LOG1($1);
		     
		     flag = UNSUPPORTF | PARAMSF | NULLVALF;
		     free($3);
		     $$ = ra_createParam($1, NULL, flag, 0);
              }
          | UNSUPPORTED PATH_NAME SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED NUMBER SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED IPADDR SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED DATE SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED STRING SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | BOOT_UNKNOWN_CLIENTS BOOLEAN SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | USE_HOST_DECL_NAMES BOOLEAN SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | FILENAME FILE_NAME SEMICOLON NEWLINE
              {
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | SERVERNAME STRING SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | NEXTSERVER HOST_NAME SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              }
          | ALLOW UNKNOWN_CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | ALLOW KNOWN_CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | ALLOW ALL CLIENTS SEMICOLON NEWLINE
              { 
		     int  flag;
		     char * s;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($2);free($3);free($5);
		     s = strdup("all clients");
		     $$ = ra_createParam($1, s, flag, hashIDArray[conflineno - 2]);
              } 
          | ALLOW DUPLICATES SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2);
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | ALLOW MEMBERS OF STRING SEMICOLON NEWLINE 
              { 
		     int flag;
		     char * s;
		     LOG3($1, $2, $4); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     s = strdup("allow members of");
		     free($1);free($6);
		     $$ = ra_createParam(s, $4, flag, hashIDArray[conflineno - 2]);
              } 
          | ALLOW UNSUPPORTED SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | DENY UNKNOWN_CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | DENY KNOWN_CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | DENY ALL CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     char * s;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($2);free($3);free($5);
		     s = strdup("all clients");
		     $$ = ra_createParam($1, s, flag, hashIDArray[conflineno - 2]);
              } 
          | DENY DUPLICATES SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | DENY UNSUPPORTED SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = UNSUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | DENY MEMBERS OF STRING SEMICOLON NEWLINE
              { 
		     int flag;
		     char * s;
		     LOG3($1, $2, $4); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($1);free($6);
		     s = strdup("deny members of");
		     $$ = ra_createParam(s, $4, flag, hashIDArray[conflineno - 2]);
              } 
          | IGNORE UNKNOWN_CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | IGNORE KNOWN_CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | IGNORE ALL CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     char * s;
		     LOG3($1, $2, $3); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($2);free($3);free($5);
		     s = strdup("all clients");
		     $$ = ra_createParam($1, s, flag, hashIDArray[conflineno - 2]);
              } 
          | IGNORE DUPLICATES SEMICOLON NEWLINE
              { 
		     int flag;
		     LOG2($1, $2); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
              } 
          | IGNORE MEMBERS OF STRING SEMICOLON NEWLINE 
              { 
		     int flag;
		     char * s;
		     LOG3($1, $2, $4); 
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     free($1);free($6);
		     s = strdup("ignore members of");
		     $$ = ra_createParam(s, $4, flag, hashIDArray[conflineno - 2]);
              } 
          | IGNORE UNSUPPORTED SEMICOLON NEWLINE
              { 
		     int  flag;
		     LOG2($1, $2);
		     
		     flag = UNSUPPORTF | PARAMSF;
		     free($4);
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | ALLOW UNSUPPORTED UNSUPPORTED CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     char s[100];
		     LOG3($1, $2, $3); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     sprintf(s,"%s %s %s",$2, $3, $4);
		     free($2);free($3);free($4);free($6);
		     $$ = ra_createParam($1, strdup(s), flag, 0);
              }
          | ALLOW UNSUPPORTED CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     char s[100];
		     LOG3($1, $2 , $3); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     sprintf(s,"%s %s",$2,$3);
		     free($2);free($3);free($5);
		     $$ = ra_createParam($1, strdup(s), flag, 0);
              }
          | DENY UNSUPPORTED UNSUPPORTED CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     char s[100];
		     LOG3($1, $2, $3); 
		     flag = UNSUPPORTF | PARAMSF;
		     sprintf(s,"%s %s %s",$2, $3, $4);
		     free($2);free($3);free($4);free($6);
		     $$ = ra_createParam($1, strdup(s), flag, 0);
              }
          | DENY UNSUPPORTED CLIENTS SEMICOLON NEWLINE
              { 
		     int flag;
		     char s[100];
		     LOG3($1, $2, $3); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     sprintf(s,"%s %s", $2, $3);
		     free($2);free($3);free($5);
		     $$ = ra_createParam($1, strdup(s), flag, 0);
              }
          | IGNORE UNSUPPORTED UNSUPPORTED CLIENTS SEMICOLON NEWLINE
              {
		     int flag;
		     char s[100];
		     LOG3($1, $2, $3); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     sprintf(s,"%s %s %s", $2, $3, $4);
		     free($2);free($3);free($4);free($6);
		     $$ = ra_createParam($1, strdup(s), flag, 0);
              }
          | IGNORE UNSUPPORTED CLIENTS SEMICOLON NEWLINE
              {
		     int flag;
		     char s[100];
		     LOG3($1, $2, $3); 
		     
		     flag = UNSUPPORTF | PARAMSF;
		     sprintf(s,"%s %s",$2, $3);
		     free($2);free($3);free($5);
		     $$ = ra_createParam($1, strdup(s), flag, 0);
              }
	  | HARDWARE HARDWARE_TYPE MACADDR SEMICOLON NEWLINE
	     {
		 int flag;
		 char * value;
		 LIST * list, *lptr;
		 LOG3($1, $2, $3); 

		 flag = SUPPORTF | PARAMSF | NOOPTIONF;
		 free($5);
		 list = ra_genListNode($2);
		 list = ra_appendToList(list, $3);
		 value = ra_multiValuedString(list, 0);
		 for(lptr = list->next; lptr != NULL; lptr = lptr->next){
		     if(list->content)
			 free(list->content);
		     free(list);
		     list = lptr;
		 }
		 $$ = ra_createParam($1, value, flag, hashIDArray[conflineno - 2]);
	     }
	  | FIXEDADDRESS IPADDR SEMICOLON NEWLINE
	     {
		 int flag;
		 LOG2($1, $2);
		 
		 flag = SUPPORTF | PARAMSF | NOOPTIONF;
		 free($4);
		 $$ = ra_createParam($1, $2, flag, hashIDArray[conflineno - 2]);
	     }
	  | if_statement 
	     {
		 $$ =  $1;
	     } 
	  |  RANGE IPADDR IPADDR SEMICOLON NEWLINE
                 {
		     int flag;
		     char * value;
		     LIST * list, *lptr;
		     LOG3($1, $2, $3); 

		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     list = ra_genListNode($2);
		     list = ra_appendToList(list, $3);
		     value = ra_multiValuedString(list, 0);
		     for(lptr = list->next; lptr != NULL; lptr = lptr->next){
			 if(list->content)
			     free(list->content);
			 free(list);
			 list = lptr;
		     }
		     free($5);
		     $$ = ra_createParam($1, value, flag, hashIDArray[conflineno - 2]);
                 }
	  | MATCH SEMICOLON NEWLINE
                 { 
		     int flag;
		     LOG1($1);

		     flag = UNSUPPORTF | MATCHF | PARAMSF | NULLVALF;
		     free($3);
		     $$ = ra_createParam($1, NULL, flag, 0);
                 } 
	  | SPAWN SEMICOLON NEWLINE
                 { 
		     int flag;
		     LOG1($1);

		     flag = UNSUPPORTF | SPAWNF | PARAMSF | NULLVALF;
		     free($3);
		     $$ = ra_createParam($1, NULL, flag, 0);
                 } 
          | LEASE LIMIT NUMBER SEMICOLON NEWLINE
                 { 
		     int flag;
		     char * value;
		     LIST * list, *lptr;
		     LOG3($1, $2, $3); 

		     flag = UNSUPPORTF | PARAMSF;
		     list = ra_genListNode($2);
		     list = ra_appendToList(list, $3);
		     value = ra_multiValuedString(list, 0);
		     for(lptr = list->next; lptr != NULL; lptr = lptr->next){
			 if(list->content)
			     free(list->content);
			 free(list);
			 list = lptr;
		     }
		     free($5);
		     $$ = ra_createParam($1, value, flag, 0);
                 }
	  | error SEMICOLON NEWLINE
	      {
		     LOG1("SEMICOLON Error");
		 
		     $$ = error_node;
		     free($3);
		     yyerrok; 
	      }
	  | NEWLINE
	    {
		//int flag = UNSUPPORTF | PARAMSF | BLANKLINF;
		//$$ = ra_createParam(NULL, NULL, flag, 0);
		$$ = ra_createComment($1);
	    }
	  ;

ipaddr_lists : ipaddr_lists COMMA IPADDR   
                {
		    LOG1("ipaddr COMA");
		    ipListBegin = 1;
		    $$ = ra_appendToList($1, $3);
                }
	    | /* empty */
		{
		    LOG1("ipaddr empty");
		    $$ = ra_genListNode(strdup("dummy")); 
		}
	    ;


if_statement: IF { elseBegin = 1; } elsifs ELSE
		{

		    int flag;
		    char *value, * s = strdup("if");
		    LIST * list, *lptr;
		    LOG1("IF ELSE");
		    
		    list = ra_genListNode($1);
		    list->next =  $3;
		    list = ra_appendToList(list, $4);
		    flag = UNSUPPORTF | PARAMSF | IF_CONF;
		    value = ra_multiValuedString(list, 0);
		    for(lptr = list->next; lptr != NULL; lptr = lptr->next){
			 if(list->content)
			     free(list->content);
			 free(list);
			 list = lptr;
		    }
		    $$ = ra_createParam(s, value, flag, 0);
		}
	    | IF
		{
		     int flag;
		     char * s = strdup("if");
		     LOG1("IF");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam(s , $1, flag, 0);
		}
	    ;

elsifs	    :	elsifs elsif
		{
		    LIST *temp;
		    for (temp = $1; temp->next != NULL; temp = temp->next);
		    temp->next = $2;
		    $$ = $1;
		}
	    | elsif
        	    {
			$$ = $1;
		    }
	    ;

elsif	    : ELSIF
		{
		    if(elseBegin)
		    {
			elseBegin = 0;
			$$ = ra_genListNode($1);
		    }
		    else
			$$ = ra_appendToList($$, $1);

		}
	    ;


%% 
int conferror(char *errmsg)
{
    char c, *s, *ptr;
    int i, counter;
    FILE * file;

    LOG1("Now in Error Recovery");
    ptr = (char *)calloc(250,1);

    file = fopen(conffile,"r");
    counter = conflineno;

    for(--counter; counter;){
	c = fgetc(file);
	if(c == '\n')
	    counter--;
	else
	    continue;
    }
    c = fgetc(file);

    for(i = 0; !( c == '{' || c == '}' || c == ';');i++){
	ptr[i] = c;
	c = fgetc(file);
    }
    ptr[i++] = c;
    fclose(file);

    s = strdup(ptr);
    free(ptr);
    error_node = ra_createIrlvnt(s);
    return 0;
}


void init_parser() 
{
    NODE * temp_node = NULL;
   struct conf* dhcp_conf = read_conf(PROVIDER_CONFFILE, PROVIDER_CONFFILE);
   init_lex();
   
   dhcp_conf_tree = ra_createNode();
   dhcp_conf_tree->obName = strdup("Global");
   dhcp_conf_tree->obFlags = GLOBALF ;
   dhcp_conf_tree->obID = 0;
   dhcp_conf_tree->parent = dhcp_conf_tree;

   temp_node = ra_createNode();
   temp_node->obName = strdup("Dhcp");
   temp_node->obFlags = SERVICEF;
   temp_node->obID = 1; 
   temp_node->obValue = ra_get_hostname();

   ra_appendNode(dhcp_conf_tree, temp_node);   

   temp_node = ra_createNode();
   temp_node->obName = get_conf(dhcp_conf, DHCPDCONF);
   temp_node->obFlags = SERCONFF;
   temp_node->obID = 2;
   
   ra_appendNode(dhcp_conf_tree, temp_node);
   temp_node = NULL;

   //current_parent = current_node = dhcp_conf_tree;
   //current_state = GLOBALF;
   free(dhcp_conf);
}

NODE * parseConfigFile(char * inFile, char *outFile) 
{ 
    int i = 0; 
    struct conf* dhcp_conf = read_conf(PROVIDER_CONFFILE, PROVIDER_CONFFILE);
    FILE * inF = NULL;

    conffile = get_conf(dhcp_conf, DHCPDCONF);
    if (!conffile)
	conffile = strdup(DEFAULT_DHCPCONF);

    pthread_mutex_lock(&fileLock);
    hashIDArray = getAllUniqueKey(conffile);
    pthread_mutex_unlock(&fileLock);

    confout = NULL;
    pthread_mutex_lock(&fileLock);
    inF = fopen(inFile, "r");
    confin = inF; 
    pthread_mutex_lock(&dsLock);
    init_parser(); 
    i = confparse();
    pthread_mutex_unlock(&dsLock);
    fclose(inF);
    pthread_mutex_unlock(&fileLock);

    //cleanup_lex();
    if(hashIDArray){
	free(hashIDArray);
	hashIDArray = NULL;
    }
    if(dhcp_conf) {
	free(dhcp_conf);
	dhcp_conf = NULL;
    }
    return dhcp_conf_tree; 
}

#ifdef DEBUG_YACCC
int main(int argc, char **argv) { 
  
   confin = fopen("./dhcpd.conf","r");
   confout = stdout;    
   init_lex(); 
   confparse(); 
  

}
#endif
 
