dnl
 dnl sblim-datagather.m4
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
 dnl Author:       Daniel de Araujo   <dan (at) us.ibm.com>
 dnl
 dnl Description:
 dnl The main function to check for the SBLIM gather header.
 dnl Modifies the CPPFLAGS with the right include directory and sets
 dnl the 'have_DATAGATHER' to either 'no' or 'yes'
dnl
AC_DEFUN([_CHECK_DATAGATHER_HEADER],
  [
      AC_MSG_CHECKING($1)
      AC_TRY_COMPILE(
      [
	  #include "cimplug.h"
	  #include "mplugin.h"
      ],
      [
          printf(METRIC_DEFINITIONPROC_S);
	  printf(COP4VALID_S);

      ],
      [
          have_DATAGATHER=yes
      ],
      [
          have_DATAGATHER=no
      ])
  ]
)

AC_DEFUN([CHECK_DATAGATHER_HEADER],
  [
      AC_MSG_CHECKING(for SBLIM GATHER headers)
      DATAGATHER_CPP_FLAGS="$CPPFLAGS"
      _CHECK_DATAGATHER_HEADER(standard)
      
      if test "have_DATAGATHER" == "yes"; then
          AC_MSG_RESULT(yes)
      else
          _DIRS_="/usr/include/ \
                  /usr/include/gather/ \
                  /usr/local/include/ \
                  /usr/local/include/gather/"
          for _DIR_ in $_DIRS_ ; do
              _cppflags=$CPPFLAGS
              _include_DATAGATHER="$_DIR_"
              CPPFLAGS="$CPPFLAGS -I$_include_DATAGATHER"
              _CHECK_DATAGATHER_HEADER($_DIR_)
              
              if test "$have_DATAGATHER" == "yes"; then
                  AC_MSG_RESULT(yes)
                  DATAGATHER_CPP_FLAGS="$CPPFLAGS"
                  break
              fi
              CPPFLAGS=$_cppflags
          done
      fi
      CPPFLAGS=$DATAGATHER_CPP_FLAGS
      AC_SUBST(LIBDATAGATHER)
      
      if test "$have_DATAGATHER" == "no"; then
          AC_MSG_RESULT(no.)
      fi
  ]
)
