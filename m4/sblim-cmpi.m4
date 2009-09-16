dnl
 dnl sblim-cmpi.m4
dnl
 dnl 
 dnl © Copyright IBM Corp. 2007
 dnl
 dnl THIS FILE IS PROVIDED UNDER THE TERMS OF THE ECLIPSE PUBLIC LICENSE
 dnl ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 dnl CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 dnl
 dnl You can obtain a current copy of the Eclipse Public License from
 dnl  http://www.opensource.org/licenses/eclipse-1.0.php
 dnl
 dnl Author:       Konrad Rzeszutek   <konradr (at) us.ibm.com>
 dnl Contributors: Viktor Mihajlovski <mihajlov (at) de.ibm.com>
 dnl               Wolfgang Taphotn   <taphorn (at) de.ibm.com>
 dnl
 dnl Description:
 dnl The main function to check for CMPI headers
 dnl Modifies the CPPFLAGS with the right include directory and sets
 dnl the 'have_CMPI' to either 'no' or 'yes'
dnl

AC_DEFUN([CHECK_CMPI_HEADER],
  [
      AC_MSG_CHECKING(for CMPI headers)
      dnl Check just with the standard include paths
      CMPI_CPP_FLAGS="$CPPFLAGS"
      _CHECK_CMPI_HEADER(standard)
      if test "$have_CMPI" == "yes"; then
          dnl The standard include paths worked.
          AC_MSG_RESULT(yes)
      else
          _DIRS_="/usr/include/cmpi \
                  /usr/local/include/cmpi \
                  $PEGASUS_ROOT/src/Pegasus/Provider/CMPI \
                  /opt/tog-pegasus/include/Pegasus/Provider/CMPI \
                  /usr/include/Pegasus/Provider/CMPI \
                  /usr/include/openwbem \
                  /usr/sniacimom/include"
          for _DIR_ in $_DIRS_
          do
              _cppflags=$CPPFLAGS
              _include_CMPI="$_DIR_"
              CPPFLAGS="$CPPFLAGS -I$_include_CMPI"
              _CHECK_CMPI_HEADER($_DIR_)
              if test "$have_CMPI" == "yes"; then
                  dnl Found it
                  AC_MSG_RESULT(yes)
                  dnl Save the new -I parameter
                  CMPI_CPP_FLAGS="$CPPFLAGS"
                  break
              fi

              CPPFLAGS=$_cppflags
          done
      fi
      CPPFLAGS="$CMPI_CPP_FLAGS"
      if test "$have_CMPI" == "no"; then
          AC_MSG_ERROR(no. Sorry cannot find CMPI headers files.)
      fi
  ]
)

dnl
dnl Helper functions
dnl
AC_DEFUN([_CHECK_CMPI_HEADER],
  [
      AC_MSG_CHECKING($1)
      AC_TRY_COMPILE(
      [
          #include <cmpimacs.h>
          #include <cmpidt.h>
          #include <cmpift.h>
      ],
      [
          CMPIBroker broker;
          CMPIStatus status = {CMPI_RC_OK, NULL};
          CMPIString *s = CMNewString(&broker, "TEST", &status);
      ],
      [
          have_CMPI=yes
          dnl AC_MSG_RESULT(yes)
      ],
      [
          have_CMPI=no
          dnl AC_MSG_RESULT(no)
      ])
  ]
)

