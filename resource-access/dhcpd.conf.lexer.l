/*
 * dhcpd.conf.lexer.l
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

#include "dhcpd.conf.parser.tab.h"
#ifdef DEBUG1
#define LOG printf("LEXER:%s .\n", conftext);
#else
#define LOG
#endif

char **symbol_table = NULL; 
extern FILE *confin;

void add_word(const char *) ;
int  match_word(const char *); 
void display_symbol_table(); 

%}

%option		    prefix="conf"
%option		    yylineno
number		    [0-9]+ 
space		    [\t ]
newline		    [\r\n]
emptyline	    [ \t]
hexdigit	    [0-9a-fA-F]
ip_octect	    ([01]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5]) 

boolean		    true|false|on|off|TRUE|FALSE|ON|OFF
boolean_operator    and|or|not
hardware_type	    ethernet|token-ring 
ddns_update_style   ad-hoc|interim 

string		    \"([^\\"]|\\.)*\"
generic_name	    ([A-Za-z0-9][A-Za-z0-9\-\+\_]*)
filename	    ([A-Za-z0-9\.][A-Za-z0-9\.\-\+\_]*)
pathname	    (\/?[A-Za-z0-9\.][A-Za-z0-9\.\-\+\_]*(\/[A-Za-z0-9\.][A-Za-z0-9\.\-\+\_]*)*\/?)
hostname	    ([A-Za-z][A-Za-z0-9\-\_]*(\.[A-Za-z][A-Za-z0-9\-\_]*)*)
ipaddr		    ({ip_octect}(\.{ip_octect}){3})
macaddr		    (({hexdigit}?{hexdigit}:){5}{hexdigit}?{hexdigit})
colon_seperated_string ({hexdigit}?{hexdigit}(:[ \n]*{hexdigit}?{hexdigit})*) 
date		    ([0-6]{space}{number}(\/{number}){2}{space}({number}\:){2}{number}) 

 
 /* Lexer State definitions */ 
%x MACADDR_STATE
%x FILENAME_STATE
%x PATHNAME_STATE
%x GENERIC_STATE
%x CLASS_STATE
%x ALLOW_MEMBERS_STATE
%x IF_CON_STATE

%%

{string}  { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(STRING); 
          }
 
<CLASS_STATE>{string} { 
            BEGIN INITIAL;

	    LOG;
            add_word(conftext); 
            display_symbol_table(); 
            conflval.string = strdup(conftext); 
            return(STRING); 
          }

<ALLOW_MEMBERS_STATE>{string} {

            BEGIN INITIAL; 

	    LOG;
            conflval.string = strdup(conftext); 
            return(STRING); 
          }

{ipaddr}  { 
	    LOG;
            conflval.string = strdup(conftext);
            return(IPADDR); 
          } 

{number}  { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(NUMBER);
          } 

{boolean} { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(BOOLEAN);
          }

{date}    {
	    LOG;
            conflval.string = strdup(conftext);
            return(DATE);
	  }

<MACADDR_STATE>{macaddr} { 
            BEGIN INITIAL; 
	    LOG;
            conflval.string = strdup(conftext); 
            return(MACADDR); 
          }
{ddns_update_style} { 
	    LOG;
            conflval.string = strdup(conftext); 
            return (DDNS_UPDATE_STYLE_TYPE);
          }
 /*
{filename} { 
          }
 */
server-identifier { 
	    LOG;
            conflval.string = strdup(conftext);
            return(SERVER_IDENTIFIER); 
          }

dynamic-bootp-lease-length {
	    LOG;
            conflval.string = strdup(conftext); 
            return(DYNAMIC_BOOTP_LEASE_LENGTH);
          }

boot-unknown-clients {
	    LOG;
            conflval.string = strdup(conftext);
            return(BOOT_UNKNOWN_CLIENTS); 
          }

allow     {
	    LOG;
            conflval.string = strdup(conftext);
            return(ALLOW); 
          }

deny      {
	    LOG;
            conflval.string = strdup(conftext);
            return(DENY);
          }

ignore    { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(IGNORE); 
          }

duplicates { 
	    LOG;
            conflval.string = strdup(conftext);            
            return(DUPLICATES); 
          }

subclass  {
	    LOG;
            conflval.string = strdup(conftext); 
            return(SUBCLASS); 
          }

lease     { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(LEASE);
          }

limit     { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(LIMIT); 
          }

known-clients { 
	    LOG;
            conflval.string = strdup(conftext);
            return(KNOWN_CLIENTS); 
          }

unknown-clients { 
	    LOG;
            conflval.string = strdup(conftext);
            return(UNKNOWN_CLIENTS); 
          }

bootp |
booting |
declines |
client-updates |
dynamic |
authenticated |
unauthenticated  {
	    LOG;
                    conflval.string = strdup(conftext);
                    return(UNSUPPORTED); 
                 }

all       {
	    LOG;
            conflval.string = strdup(conftext); 
            return(ALL); 
          }
 
clients   { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(CLIENTS); 
          }

if        { 
	    int c, i = 0;
	    char *str = (char *)calloc(512,1);
	    while((c = input()) == ' ');
	    str[i++] = c;
	    while((c = input()) != '}')
		str[i++] = c;
	    str[i] = c;
	    conflval.string = strdup(str);
            free(str);
            return(IF); 
          }

elsif     {
	    int c, i = 7;
	    char *str = (char *)calloc(512,1);
	    sprintf(str," elsif ");
	    while((c = input()) == ' ');
	    str[i++] = c;
	    while((c = input()) != '}')
		str[i++] = c;
	    str[i] = c;
	    conflval.string = strdup(str);
            free(str);
            return(ELSIF); 
          }
 
else      {
	    int c, i = 6;
	    char *str = (char *)calloc(512,1);
	    sprintf(str," else ");
	    while((c = input()) == ' ');
	    str[i++] = c;
	    while((c = input()) != '}')
		str[i++] = c;
	    str[i] = c;
	    conflval.string = strdup(str); 
            free(str);
            return(ELSE); 
          }

members   { 
	    LOG;
            BEGIN ALLOW_MEMBERS_STATE; 
            return(MEMBERS); 
          } 

<ALLOW_MEMBERS_STATE>of { 
	    LOG;
            return(OF); 
          }

server-name { 
	    LOG;
            conflval.string = strdup(conftext);
            return(SERVERNAME); 
          }

filename  {
            BEGIN FILENAME_STATE;  
	    LOG;
            conflval.string = strdup(conftext);
            return(FILENAME); 
          } 

next-server { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(NEXTSERVER); 
          }


ddns-update-style { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(DDNS_UPDATE_STYLE); 
          }

default-lease-time { 
	    LOG;
            conflval.string = strdup(conftext);
            return(DEFAULT_LEASE_TIME); 
          }

max-lease-time { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(MAX_LEASE_TIME); 
          }

get-lease-hostnames {
	    LOG;
            conflval.string = strdup(conftext);
            return(GET_LEASE_HOSTNAMES); 
          }

use-host-decl-names { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(USE_HOST_DECL_NAMES); 
          }

option    {
	    LOG;
            conflval.string = strdup(conftext); 
            return(OPTION);
          } 

host      {
            BEGIN GENERIC_STATE;
	    LOG;
            conflval.string = strdup(conftext); 
            return(HOST); 
          }

subnet    { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(SUBNET); 
          }

class     {
            BEGIN CLASS_STATE; 
	    LOG;
            conflval.string = strdup(conftext); 
            return(CLASS); 
          }

pool      { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(POOL); 
          }

match | 
spawn	  {  
	    int c,i = 0;
	    char *str = (char *)calloc(512,1);
	    while( (c = input()) != ';')
		str[i++] = c;
	    str[i] = '\0';
	    unput(c);
	    conflval.string = strdup(str);
            free(str);
	    if(conftext[0] == 'm')
		return (MATCH);
	    else
		return (SPAWN);
          }

shared-network { 
            BEGIN GENERIC_STATE; 
	    LOG;
            conflval.string = strdup(conftext); 
            return(SHARED_NETWORK); 
          }

group     { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(GROUP); 
          } 

netmask   {
	    LOG;
            conflval.string = strdup(conftext); 
            return(NETMASK); 
          }

     /* different types of options */
routers | 
subnet-mask |
domain-name |
time-offset |
interface-mtu |
domain-name-servers | 
static-routes | 
trailer-encapsulation | 
ip-forwarding | 
non-local-source-routing | 
policy-filter | 
default-ip-ttl | 
default-tcp-ttl | 
broadcast-address | 
host-name |
hostname |
dhcp-client-identifier {
	    LOG;
                          conflval.string = strdup(conftext); 
                          return(OPTION_TYPE);
                       } 


all-subnets-local |
arp-cache-timeout |
bootfile-name |
boot-size |
cookie-servers |
dhcp-leae-time |
dhcp-max-message-size |
dhcp-message |
dhcp-message-type |
dhcp-option-overload |
dhcp-parameter-request-list |
dhcp-rebinding-time |
dhcp-renewal-time |
dhcp-requested-address |
dhcp-server-identifier |
extensions-path |
finger-server |
font-servers |
ieee802-3-encapsulation |
ien116-name-servers |
impress-servers |
irc-server |
log-servers |
lpr-servers |
mask-supplier |
max-dgram-reassembly |
merit-dump |
mobile-ip-home-agent |
nds-context |
nds-servers |
nds-tree-name |
netbios-dd-server |
netbios-name-servers |
netbios-node-type |
netbios-scope |
nis-domain |
nis-servers |
nisplus-domain |
nisplus-servers |
nntp-server |
ntp-servers |
nwip-domain |
nwip-suboptions |
path-mtu-aging-timeout |
path-mtu-plateau-table |
perform-mask-discovery |
pop-server |
resource-location-servers |
root-path |
router-discovery |
router-solicitation-address |
slp-directory-agent |
slp-service-scope |
smtp-server |
streettalk-directory-assistance-server |
streettalk-server |
subnet-selection |
swap-server |
tcp-keepalive-garbage |
tcp-keepalive-interval |
tftp-server-name |
time-servers |
uap-servers |
user-class |
vendor-class-identifier |
vendor-encapsulated-options |
www-server |
x-display-manager {
	    LOG;
                      conflval.string = strdup(conftext);
                      return(UNRECOGNISED);
                  }
		     
always-broadcast |
always-reply-rfc1048 | 
authoritative |
ddns-hostname |
ddns-domainname |
ddns-rev-domainname |
ddns-updates |
do-forward-updates |
dynamic-bootp-lease-cutoff |
local-port |
local-address |
min-lease-time |
min-secs |
omapi-port |
one-lease-per-client |
ping-check |
ping-timeout |
site-option-space |
stash-agent-options |
update-optimization |
update-static-leases |
use-lease-addr-for-default-route |
vendor-option-space  {	
	    LOG;
                  	conflval.string = strdup(conftext);
                  	return(UNSUPPORTED); 
	      	     }                

lease-file-name |
log-facility |
pid-file-name  {
	          BEGIN PATHNAME_STATE ;
	    LOG;
                  conflval.string = strdup(conftext);
                  return(UNSUPPORTED); 
	       }                

hardware  { 
            BEGIN MACADDR_STATE ; 
	    LOG;
            conflval.string = strdup(conftext);
            return(HARDWARE); 
          }

range     { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(RANGE); 
          }

nothingtobedone {
		    REJECT;
		}

fixed-address { 
	    LOG;
            conflval.string = strdup(conftext);
            return(FIXEDADDRESS); 
          }

<MACADDR_STATE>{hardware_type} { 
	    LOG;
            conflval.string = strdup(conftext);
            return(HARDWARE_TYPE); 
          }

"{"       {
            return(LBRACE);
          }

"}"       {
            return(RBRACE); 
          }

";"       {
            return(SEMICOLON); 
          }

","       {
            return(COMMA);
          }


"#"(.*)(\n)? {
	    LOG;
	    conflval.string = strdup(conftext);
	    return(COMMENT);
          } 
 
{hostname} { 
	    LOG;
            conflval.string = strdup(conftext); 
            return(HOST_NAME); 
          }

<FILENAME_STATE>{generic_name} |
<FILENAME_STATE>{string} |
<FILENAME_STATE>{filename} {
            BEGIN INITIAL;
	    LOG;
            conflval.string = strdup(conftext);
            return(FILE_NAME); 
          }

<PATHNAME_STATE>{pathname} { 
            BEGIN INITIAL;
	    LOG;
            conflval.string = strdup(conftext);
            return(PATH_NAME); 
          }  

<GENERIC_STATE>{generic_name} {
            BEGIN INITIAL;
	    LOG;
            conflval.string = strdup(conftext); 
            return(GENERIC_NAME); 
          }

<*>{newline} {
		LOG;
		conflval.string = strdup(conftext); 
		return (NEWLINE);
	      }
<*>{emptyline}* {
		LOG;
	        }  
{colon_seperated_string} {
	    LOG;
	    conflval.string = strdup(conftext);
	    return(COLON_STRING);
	 }

<*>.	{ return conftext[0]; }
<*><<EOF>> { return 0; }


%%

int confwrap(void ) {
   conflineno = 1; 
   return 1; 
} 

void init_lex() { 
    symbol_table = (char **)calloc(1,sizeof(char**));
} 

void add_word(const char * word) { 
    int i = 0;
    
    while(symbol_table[i]!=NULL) i++; 

    symbol_table = realloc(symbol_table, (sizeof(char**)*(i+2))); 
   
    symbol_table[i] = strdup(word); 
    symbol_table[i+1] = NULL; 

    
}

int match_word(const char * word) { 
    int i = 0; 
    while (symbol_table[i]!=NULL) {
         if (strcmp(symbol_table[i],word)==0) return 1; 
         i++;
    } 
    return 0; 
}

void cleanup_lex() {
    int i = 0; 
 
    while (symbol_table[i]!=NULL) {
         free(symbol_table[i]);
         i++;
    }
    free(symbol_table); 
}

void display_symbol_table() {
    int i = 0; 
    
    while (symbol_table[i]!=NULL) {
         i++;
    }
}

#ifdef DEBUG_LEXX
int main(int argc, char** argv)
{
	confin = fopen("./dhcpd.conf","r"); 
	confout = stdout;
        init_lex(); 
	while(conflex()) { }
        cleanup_lex(); 
	return 0;
}
#endif

