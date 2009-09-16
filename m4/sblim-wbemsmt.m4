dnl
 dnl sblim-wbemsmt.m4
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
 dnl Author:       Wolfgang Taphotn   <taphorn (at) de.ibm.com>
 dnl
 dnl Description:
 dnl The main function to check for the cmpi-base common header
 dnl Modifies the CPPFLAGS with the right include directory and sets
 dnl the 'have_SBLIMBASE' to either 'no' or 'yes'
dnl
AC_DEFUN([_CHECK_SBLIM_LIBRA_HEADER],
  [
      AC_MSG_CHECKING($1)
      AC_TRY_COMPILE(
      [
          #include <smt_libra_conf.h>
      ],
      [
          printf(SCRIPTS);
      ],
      [
          have_SBLIMLIBRA=yes
      ],
      [
          have_SBLIMLIBRA=no
      ])
  ]
)

AC_DEFUN([CHECK_SBLIM_LIBRA_HEADER],
  [
      AC_MSG_CHECKING(for SBLIM ToolsLibRA headers)
      SBLIMBASE_CPP_FLAGS="$CPPFLAGS"
      _CHECK_SBLIM_LIBRA_HEADER(standard)
      
      if test "$have_SBLIMLIBRA" == "yes"; then
          AC_MSG_RESULT(yes)
      else
          _DIRS_="/usr/include/sblim \
                  /usr/local/include/sblim"
          for _DIR_ in $_DIRS_ ; do
              _cppflags=$CPPFLAGS
              _include_SBLIMBASE="$_DIR_"
              CPPFLAGS="$CPPFLAGS -I$_include_SBLIMBASE"
              _CHECK_SBLIM_LIBRA_HEADER($_DIR_)
              
              if test "$have_SBLIMLIBRA" == "yes"; then
                  AC_MSG_RESULT(yes)
                  SBLIMBASE_CPP_FLAGS="$CPPFLAGS"
                  break
              fi
              CPPFLAGS=$_cppflags
          done
      fi
      CPPFLAGS=$SBLIMBASE_CPP_FLAGS
      AC_SUBST(LIBSBLIMBASE)
      
      if test "$have_SBLIMBASE" == "no"; then
          AC_MSG_ERROR(no. The required SBLIM ToolsLibRA header files are missing.)
      fi
  ]
)

