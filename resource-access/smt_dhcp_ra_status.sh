#! /bin/sh

#
# status.sh
#
# (C) Copyright IBM Corp. 2007
#
# THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
# ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
# CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
#
# You can obtain a current copy of the Eclipse Public License from
# http://www.opensource.org/licenses/eclipse-1.0.php
#
# Author:     Riyashmon Haneefa <riayshh1@in.ibm.com>
#             Ashoka S Rao      <ashoka.rao@in.ibm.com>
#
#
#

dhcpd='dhcpd'
prog=$dhcpd
if [ "`pidof -o %PPID $dhcpd`" ]; then
        echo -n $"$dhcpd: Service is already running"
        echo
        exit 2
else
        echo -n $"$dhcpd: Service is not running"
        echo
        exit 0
fi

