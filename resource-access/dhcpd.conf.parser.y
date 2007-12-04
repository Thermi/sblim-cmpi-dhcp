/*
 * dhcpd.conf.parser.y
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

%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ra-support.h"
#include "provider-support.h"

#ifdef DEBUG1
#define LOG(s)  printf("PARSER[%d]:%s .\n", conflineno, s);
#else
#define LOG(s)
#endif

extern FILE *confin;
extern FILE *confout;
extern int conflex(void); 
extern int conferror(char *);
extern void init_lex(void); 
extern void confrestart(FILE *); 
extern int confwrap(void ); 
extern int cleanup_lex(void);

extern int conflineno; 

char * comment_string = NULL;
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
%token <string> LBRACE RBRACE SEMICOLON COMMA LPARANTHESIS RPARANTHESIS EQUALS 
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
		    LOG("dhcp conf decl");
		    $$ = ra_dropChild($1, $2);
		}
		| dhcp_config_file parameter 
		{ 
		    LOG("dhcp conf parameter");
		    $$ = ra_dropChild($1,$2); 
		}
		| dhcp_config_file comment 
		{ 
		    LOG("dhcp conf comment");
		    $$ = ra_dropChild($1,$2); 
		}
		| /* empty */ 
		{
		    LOG("dhcp conf empty");
		    $$ = dhcp_conf_tree;
		}
                ;

entities :	  entities declaration
		{
		    LOG("entities.. decl");
		    $$ = ra_appendNode($1, $2);
		}
		| entities parameter
		{
		    LOG("entities.. decl");
		    $$ = ra_appendNode($1, $2);
		}
		| entities comment
		{
		    LOG("entities.. decl");
		    $$ = ra_appendNode($1, $2);
		}
		| /* empty */
		{
		    LOG("entities ... empty");
		    $$ = ra_createDummy(); 
		}
		;

parameters: parameters parameter 
		{ 
		    LOG("params param");
		    $$ = ra_appendNode($1, $2); 
		}
		| /* empty */ 
		{
		    LOG("params empty ");
		    $$ = ra_createDummy();
		}
          ; 

declaration:  subnet
		{ 
		    LOG("decl subnet");
		    $$ = $1; 
		}
	    |  sharednet
		{ 
		    LOG("decl sharednet");
		    $$ = $1; 
		}
	    |  group
		{ 
		    LOG("decl group");
		    $$ = $1; 
		}
	    |  class
		{ 
		    LOG("decl class");
		    $$ = $1; 
		}
	    |  host
		{ 
		    LOG("decl host");
		    $$ = $1; 
		}
	    | pool
		{ 
		    LOG("decl pool");
		    $$ = $1; 
		}
	    | error LBRACE
		{
		    LOG("_______________decl { error ____________");
		    $$ = error_node;
		    yyerrok;
		}
	    | error RBRACE 
		{  
		    LOG("_______________decl } error__________________");
		    $$ = error_node;
		    yyerrok;
		} 
           ; 

subnet: SUBNET IPADDR NETMASK IPADDR LBRACE entities RBRACE
        {
	    NODE * current = NULL;
	    LOG("subnet entered...........................");

	    current = ra_createSubnet($2, $4, 0);
	    ra_addRight(current, $6);
	    $$ = current;

        }; 

sharednet: SHARED_NETWORK GENERIC_NAME LBRACE entities RBRACE
         {
	     NODE * current = NULL;
	     LOG("sharednet entered.............");

	     current = ra_createSharedNet($1, $2, 0);
	     ra_addRight(current, $4);
	     $$ = current;
	 };

group: GROUP LBRACE entities RBRACE 
	{
	     NODE * current = NULL;
	     LOG("group entered ..................");

	     current = ra_createGroup($1, 0);
	     ra_addRight(current, $3);
	     $$ = current;
	
	};

comment: COMMENT
	{
	    LOG("comment entered...........");
	    $$ = ra_createComment($1);
	};

pool: POOL LBRACE entities RBRACE 
	{
	     NODE * current = NULL;
	     LOG("pool entered.........................");

	     current = ra_createPool($1, 0);
	     ra_addRight(current, $3);
	     $$ = current;
	};

class: CLASS STRING LBRACE parameters RBRACE
	{
	     NODE * current = NULL;
	     LOG("class entered....................");

	     current = ra_createClass($1, $2, 0);
	     ra_addRight(current, $4);
	     $$ = current;
	};


host: HOST GENERIC_NAME LBRACE entities RBRACE
	{
	     NODE * current = NULL;
	     LOG("host entered ...........");

	     current = ra_createHost($1, $2, 0);
	     ra_addRight(current, $4);
	     $$ = current;
	};

parameter: OPTION OPTION_TYPE NUMBER SEMICOLON  
              {
		     int flag;
		     LOG("param OPTION TYPE NUM");
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     $$ = ra_createParam($2, $3, flag, 0);
              }
	  | OPTION OPTION_TYPE IPADDR SEMICOLON  
              {
		     int flag;
		     LOG("param OPTION TYPE IPADDR ");
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     $$ = ra_createParam($2, $3, flag, 0);
              }
          | OPTION OPTION_TYPE BOOLEAN SEMICOLON 
              {
		     int flag;
		     LOG("param OPTION TYPE BOOL");
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     $$ = ra_createParam($2, $3, flag, 0);
              }
          | OPTION OPTION_TYPE IPADDR ipaddr_lists SEMICOLON  
              {
        	     int flag;
		     char * value;
		     LIST * list;
		     LOG("param OPTION TYPE IPADDR list");
		     
		     list = ra_genListNode($3);
		     list->next = $4;
		     flag = SUPPORTF | PARAMSF | OPTIONF ;
		     flag |= ipListBegin? COMMAF:0;
		     ipListBegin = 0;
		     value = ra_multiValuedString(list, 1);
		     $$ = ra_createParam($2, value, flag, 0);
              }

          | OPTION UNRECOGNISED IPADDR ipaddr_lists SEMICOLON  
	      {
               	     int flag;
		     char * value;
		     LIST * list;
		     LOG("param OPTION UNRECOGNISED IPADDR list");
		     
		     list = ra_genListNode($3);
		     list->next = $4;
		     flag = UNSUPPORTF | PARAMSF | OPTIONF ;
		     flag |= ipListBegin? COMMAF: 0;
		     ipListBegin = 0;
		     value = ra_multiValuedString(list, 1);
		     $$ = ra_createParam($2, value, flag, 0);
              }
          | OPTION OPTION_TYPE IPADDR IPADDR SEMICOLON  
              {
               	     int flag;
		     char * value;
		     LIST * list;
		     LOG("param OPTION TYPE IPADDR IPADDR");
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     list = ra_genListNode($3);
		     list = ra_appendToList(list, $4);
		     value = ra_multiValuedString(list, 0);
		     $$ = ra_createParam($2, value, flag, 0);
              }
          | OPTION OPTION_TYPE STRING SEMICOLON  
              {
		     int flag;
		     LOG("param OPTION TYPE STRING ");
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     $$ = ra_createParam($2, $3, flag, 0);
              }
          | OPTION OPTION_TYPE HOST_NAME SEMICOLON 
              { 
		     int flag;
		     LOG("param OPTION TYPE HOST_NAME ");
		     
		     flag = SUPPORTF | PARAMSF | OPTIONF;
		     $$ = ra_createParam($2, $3, flag, 0);
              } 
          | DDNS_UPDATE_STYLE DDNS_UPDATE_STYLE_TYPE SEMICOLON 
              {
		     int flag;
		     LOG("param DDNS_UPDATE_STYLE_TYPE ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | SERVER_IDENTIFIER HOST_NAME SEMICOLON 
              {
		     int flag;
		     LOG("param SERVER_IDENTIFIER");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | DEFAULT_LEASE_TIME NUMBER SEMICOLON 
              {
		     int flag;
		     LOG("param DEFAULT_LEASE_TIME");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | MAX_LEASE_TIME NUMBER SEMICOLON 
              {
		     int flag;
		     LOG("param MAX_LEASE_TIME");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | GET_LEASE_HOSTNAMES BOOLEAN SEMICOLON 
              {
		     int flag;
		     LOG("param GET_LEASE_HOSTNAMES");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | DYNAMIC_BOOTP_LEASE_LENGTH NUMBER SEMICOLON 
              {
		     int flag;
		     LOG("param DYNAMIC_BOOTP_LEASE_LENGTH");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED BOOLEAN SEMICOLON 
              {
		     int flag;
		     LOG("param UNSUPPORTED BOOL");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED SEMICOLON 
              {
		     int flag;
		     LOG("param UNSUPPORTED");
		     
		     flag = UNSUPPORTF | PARAMSF | NULLVALF;
		     $$ = ra_createParam($1, NULL, flag, 0);
              }
          | UNSUPPORTED PATH_NAME SEMICOLON 
              {
		     int flag;
		     LOG("param UNSUPPORTED PATH_NAME");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED NUMBER SEMICOLON 
              {
		     int flag;
		     LOG("param UNSUPPORTED NUM");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED IPADDR SEMICOLON 
              {
		     int flag;
		     LOG("param UNSUPPORTED IPADDR");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED DATE SEMICOLON 
              {
		     int flag;
		     LOG("param UNSUPPORTED DATE");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | UNSUPPORTED STRING SEMICOLON 
              {
		     int flag;
		     LOG("param UNSUPPORTED STRING");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | BOOT_UNKNOWN_CLIENTS BOOLEAN SEMICOLON 
              {
		     int flag;
		     LOG("param BOOT_UNKNOWN_CLIENTS");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | USE_HOST_DECL_NAMES BOOLEAN SEMICOLON 
              {
		     int flag;
		     LOG("param USE_HOST_DECL_NAMES");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | FILENAME FILE_NAME SEMICOLON 
              {
		     int flag;
		     LOG("param FILENAME");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | SERVERNAME STRING SEMICOLON  
              { 
		     int flag;
		     LOG("param SERVERNAME");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | NEXTSERVER HOST_NAME SEMICOLON  
              { 
		     int flag;
		     LOG("param NEXTSERVER");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | ALLOW UNKNOWN_CLIENTS SEMICOLON 
              { 
		     int flag;
		     LOG("param ALLOW UNKNOWN_CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | ALLOW KNOWN_CLIENTS SEMICOLON 
              { 
		     int flag;
		     LOG("param ALLOW KNOWN_CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | ALLOW ALL CLIENTS SEMICOLON 
              { 
		     int  flag;
		     char * s;
		     LOG("param ALLOW CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     s = strdup("all clients");
		     $$ = ra_createParam($1, s, flag, 0);
              } 
          | ALLOW DUPLICATES SEMICOLON 
              { 
		     int flag;
		     LOG("param ALLOW DUPLICATES ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | ALLOW MEMBERS OF STRING SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param ALLOW MEMBERS OF STRING ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     s = strdup("allow members of");
		     $$ = ra_createParam(s, $4, flag, 0);
              } 
          | ALLOW UNSUPPORTED SEMICOLON  
              { 
		     int flag;
		     LOG("param ALLOW UNSUPPORTED ");
		     
		     flag = UNSUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | DENY UNKNOWN_CLIENTS SEMICOLON 
              { 
		     int flag;
		     LOG("param DENY UNKNOWN_CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | DENY KNOWN_CLIENTS SEMICOLON 
              { 
		     int flag;
		     LOG("param DENY KNOWN_CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | DENY ALL CLIENTS SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param DENY ALL CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     s = strdup("all clients");
		     $$ = ra_createParam($1, s, flag, 0);
              } 
          | DENY DUPLICATES SEMICOLON 
              { 
		     int flag;
		     LOG("param DENY DUPLICATES ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | DENY UNSUPPORTED SEMICOLON 
              { 
		     int flag;
		     LOG("param DENY UNSUPPORTED ");
		     
		     flag = UNSUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | DENY MEMBERS OF STRING SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param DENY MEMBERS OF STRING ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     s = strdup("deny members of");
		     $$ = ra_createParam(s, $4, flag, 0);
              } 
          | IGNORE UNKNOWN_CLIENTS SEMICOLON 
              { 
		     int flag;
		     LOG("param IGNORE UNKNOWN_CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | IGNORE KNOWN_CLIENTS SEMICOLON 
              { 
		     int flag;
		     LOG("param IGNORE KNOWN_CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | IGNORE ALL CLIENTS SEMICOLON 
              { 
		     int flag;
		     char * s;
		     LOG("param IGNORE ALL CLIENTS ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     s = strdup("all clients");
		     $$ = ra_createParam($1, s, flag, 0);
              } 
          | IGNORE DUPLICATES SEMICOLON 
              { 
		     int flag;
		     LOG("param IGNORE DUPLICATES ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     $$ = ra_createParam($1, $2, flag, 0);
              } 
          | IGNORE MEMBERS OF STRING SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param IGNORE MEMBERS OF STRING ");
		     
		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     s = strdup("ignore members of");
		     $$ = ra_createParam(s, $4, flag, 0);
              } 
          | IGNORE UNSUPPORTED SEMICOLON  
              { 
		     int  flag;
		     LOG("param IGNORE UNSUPPORTED ");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam($1, $2, flag, 0);
              }
          | ALLOW UNSUPPORTED UNSUPPORTED CLIENTS SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param ALLOW UNSUPPORTED UNSUPPORTED CLIENTS ");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     s = strdup("dynamic bootp clients");
		     $$ = ra_createParam($1, s, flag, 0);
              }
          | ALLOW UNSUPPORTED CLIENTS SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param ALLOW UNSUPPORTED CLIENTS ");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     if(!strcmp("authenticated", $2))
			 s = strdup("authenticated clients");
		     else
			 s = strdup("unauthenticated clients");
		     $$ = ra_createParam($1, s, flag, 0);
              }
          | DENY UNSUPPORTED UNSUPPORTED CLIENTS SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param DENY UNSUPPORTED UNSUPPORTED CLIENTS ");
		     flag = UNSUPPORTF | PARAMSF;
		     s = strdup("dynamic bootp clients");
		     $$ = ra_createParam($1, s, flag, 0);
              }
          | DENY UNSUPPORTED CLIENTS SEMICOLON  
              { 
		     int flag;
		     char * s;
		     LOG("param DENY UNSUPPORTED CLIENTS ");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     if(!strcmp("authenticated", $2))
			 s = strdup("authenticated clients");
		     else
			 s = strdup("unauthenticated clients");
		     $$ = ra_createParam($1, s, flag, 0);
              }
          | IGNORE UNSUPPORTED UNSUPPORTED CLIENTS SEMICOLON  
              {
		     int flag;
		     char * s;
		     LOG("param IGNORE UNSUPPORTED UNSUPPORTED CLIENTS ");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     s = strdup("dynamic bootp clients");
		     $$ = ra_createParam($1, s, flag, 0);
              }
          | IGNORE UNSUPPORTED CLIENTS SEMICOLON  
              {
		     int flag;
		     char * s;
		     LOG("param IGNORE UNSUPPORTED CLIENTS ");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     if(!strcmp("authenticated", $2))
			 s = strdup("authenticated clients");
		     else
			 s = strdup("unauthenticated clients");
		     $$ = ra_createParam($1, s, flag, 0);
              }
	  | HARDWARE HARDWARE_TYPE MACADDR SEMICOLON 
	     {
		 int flag;
		 char * value;
		 LIST * list;
		 LOG("param HARDWARE_TYPE MACADDR ");

		 flag = SUPPORTF | PARAMSF | NOOPTIONF;
		 list = ra_genListNode($2);
		 list = ra_appendToList(list, $3);
		 value = ra_multiValuedString(list, 0);
		 $$ = ra_createParam($1, value, flag, 0);
	     }
	  | FIXEDADDRESS IPADDR SEMICOLON 
	     {
		 int flag;
		 LOG("param FIXEDADDRESS");
		 
		 flag = SUPPORTF | PARAMSF | NOOPTIONF;
		 $$ = ra_createParam($1, $2, flag, 0);
	     }
	  | if_statement 
	     {
		 $$ =  $1;
	     } 
	  |  RANGE IPADDR IPADDR SEMICOLON 
                 {
		     int flag;
		     char * value;
		     LIST * list;
		     LOG("param RANGE IPADDR");

		     flag = SUPPORTF | PARAMSF | NOOPTIONF;
		     list = ra_genListNode($2);
		     list = ra_appendToList(list, $3);
		     value = ra_multiValuedString(list, 0);
		     $$ = ra_createParam($1, value, flag, 0);
                 }
	  | MATCH SEMICOLON 
                 { 
		     int flag;
		     LOG("param MATCH");

		     flag = UNSUPPORTF | MATCHF | PARAMSF | NULLVALF;
		     $$ = ra_createParam($1, NULL, flag, 0);
                 } 
	  | SPAWN SEMICOLON 
                 { 
		     int flag;
		     LOG("param SPAWN");

		     flag = UNSUPPORTF | SPAWNF | PARAMSF | NULLVALF;
		     $$ = ra_createParam($1, NULL, flag, 0);
                 } 
          | LEASE LIMIT NUMBER SEMICOLON 
                 { 
		     int flag;
		     char * value;
		     LIST * list;
		     LOG("params LEASE");

		     flag = UNSUPPORTF | PARAMSF;
		     list = ra_genListNode($2);
		     list = ra_appendToList(list, $3);
		     value = ra_multiValuedString(list, 0);
		     $$ = ra_createParam($1, value, flag, 0);
                 }
	  | error SEMICOLON 
	      {
		     LOG("____________________param error__________________");
		 
		      $$ = error_node;
		      yyerrok; 
	      } 
	  ;

ipaddr_lists : ipaddr_lists COMMA IPADDR   
                {
		    LOG("ipaddr COMA");
		    ipListBegin = 1;
		    $$ = ra_appendToList($1, $3);
                }
	    | /* empty */
		{
		    LOG("ipaddr empty");
		    $$ = ra_genListNode("dummy"); 
		}
	    ;


if_statement: IF { elseBegin = 1; } elsifs ELSE
		{

		    int flag;
		    char *value, * s = strdup("if");
		    LIST * list;
		    LOG("IF ELSE");
		    
		    list = ra_genListNode($1);
		    list->next =  $3;
		    list = ra_appendToList(list, $4);
		    flag = UNSUPPORTF | PARAMSF | IF_CONF;
		    value = ra_multiValuedString(list, 0);
		    $$ = ra_createParam(s, value, flag, 0);
		}
	    | IF
		{
		     int flag;
		     char * s = strdup("if");
		     LOG("IF");
		     
		     flag = UNSUPPORTF | PARAMSF;
		     $$ = ra_createParam(s , $1, flag, 0);
		}
	    ;

elsifs	    :	elsifs elsif
		{
		    $$ = $2;
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
    char* conffile;

    struct conf* dhcp_conf = read_conf(PROVIDER_CONFFILE, PROVIDER_CONFFILE);
    LOG("Now in Error Recovery");
    ptr = (char *)calloc(250,1);
    conffile = get_conf(dhcp_conf, DHCPDCONF);
    if (!conffile)
	conffile = strdup(DEFAULT_DHCPCONF);

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

    s = strdup(ptr);
    free(ptr);
    error_node = ra_createIrlvnt(s);
    return 0;
}


void init_parser() 
{ 
   struct conf* dhcp_conf = read_conf(PROVIDER_CONFFILE, PROVIDER_CONFFILE);
   init_lex();
   
   dhcp_conf_tree = ra_createNode();
   dhcp_conf_tree->obName = strdup("Global");
   dhcp_conf_tree->obFlags = GLOBALF ;
   dhcp_conf_tree->obID = 0;
   dhcp_conf_tree->parent = dhcp_conf_tree;

   service_node = ra_createNode();
   service_node->obName = strdup("Dhcp");
   service_node->obFlags = SERVICEF;
   service_node->obID = 1; 
   service_node->obValue = strdup(ra_get_hostname());

   ra_appendNode(dhcp_conf_tree, service_node);   

   serconf_node = ra_createNode();
   serconf_node->obName = strdup(get_conf(dhcp_conf, DHCPDCONF));
   serconf_node->obFlags = SERCONFF;
   serconf_node->obID = 2;
   
   ra_appendNode(dhcp_conf_tree, serconf_node);

   current_parent = current_node = dhcp_conf_tree;
   current_state = GLOBALF;
}

NODE * parseConfigFile(FILE * inFile, FILE *outFile) 
{ 
   int i = 0; 
   confin = inFile; 
   confout = outFile;

   init_parser(); 
   i = confparse();
   if (i == 1 || i == 2) {
	//parser failed
	return NULL;
   }
   cleanup_lex();
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
 
