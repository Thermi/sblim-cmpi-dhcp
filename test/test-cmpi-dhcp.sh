#!/bin/bash
# ==================================================================
# © Copyright IBM Corp. 2007
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
#
# You can obtain a current copy of the Eclipse Public License from
# http://www.opensource.org/licenses/eclipse-1.0.php
#
# Authors : Ashoka Rao S <ashoka.rao (at) in.ibm.com>
#           Riyashmon Haneefa <riyashh1 (at) in.ibm.com>
# ==================================================================

SCRIPT_PATH=`dirname ${BASH_SOURCE}`
#*****************************************************************************#
export DHCPCONFFILE=/etc/dhcpd.conf 

#*****************************************************************************#
PWD1=`pwd`

init() {
    echo " "
    echo "Initializing DHCP Test case environment"

     if [[ -a $DHCPCONFFILE ]]; then 
    echo " copy the $DHCPCONFFILE file to $DHCPCONFFILE.original "
    cp -p $DHCPCONFFILE $DHCPCONFFILE.original
    cp -p ./dhcpd.conf /etc/
    else
    echo " copying the ./dhcpd.conf to /etc "
    cp -p ./dhcpd.conf /etc/
    fi
}

#*****************************************************************************#
cleanup() {
    echo "Cleanup system from DHCP Test case environment"
    
    if [[ -a $DHCPCONFFILE.original ]]; then
    echo " Copy back the $DHCPCONFFILE file "
    cp -p $DHCPCONFFILE.original $DHCPCONFFILE
    fi

}

#*****************************************************************************#
trap cleanup 2 3 4 6 9 15

#*****************************************************************************#

declare -a CLASSNAMES[];
CLASSNAMES=(
[1]=Linux_DHCPHost
[2]=Linux_DHCPService
[3]=Linux_DHCPServiceConfiguration
[4]=Linux_DHCPParams
[5]=Linux_DHCPOptions
[6]=Linux_DHCPGlobal
[7]=Linux_DHCPSubnet
[8]=Linux_DHCPSharednet
[9]=Linux_DHCPGroup
[10]=Linux_DHCPPool
[11]=Linux_DHCPGroupsForEntity
[12]=Linux_DHCPHostsForEntity
[13]=Linux_DHCPOptionsForEntity
[14]=Linux_DHCPParamsForEntity
[15]=Linux_DHCPPoolsForEntity
[16]=Linux_DHCPServiceConfigurationForService
[17]=Linux_DHCPSubnetsForEntity
[18]=Linux_DHCPGlobalForService
[19]=Linux_DHCPSharednetsForEntity
)

#*****************************************************************************#

init

cd $SCRIPT_PATH

declare -i max=19;
declare -i i=1;

. ${SCRIPT_PATH}/run.sh Linux_DHCPRegisteredProfile -n /root/PG_InterOp || exit 1;
. ${SCRIPT_PATH}/run.sh Linux_DHCPElementConformsToProfile -n /root/PG_InterOp || exit 1;

while(($i<=$max))
do
 . ${SCRIPT_PATH}/run.sh ${CLASSNAMES[$i]} || exit 1;
  i=$i+1;
done

cleanup
