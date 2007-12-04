dnl
 dnl sblim-indication.m4
dnl
 dnl 
 dnl © Copyright IBM Corp. 2007
 dnl
 dnl THIS FILE IS PROVIDED UNDER THE TERMS OF THE COMMON PUBLIC LICENSE
 dnl ("AGREEMENT"). ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS FILE
 dnl CONSTITUTES RECIPIENTS ACCEPTANCE OF THE AGREEMENT.
 dnl
 dnl You can obtain a current copy of the Common Public License from
 dnl  http://www.opensource.org/licenses/cpl1.0.php
 dnl
 dnl Author:       Konrad Rzeszutek   <konradr (at) us.ibm.com>
 dnl Contributors: Viktor Mihajlovski <mihajlov (at) de.ibm.com>
 dnl               Wolfgang Taphotn   <taphorn (at) de.ibm.com>
 dnl
 dnl Description:
 dnl The check for the SBLIM indication support
dnl
AC_DEFUN([_CHECK_INDHELP_HEADER],
  [
      AC_MSG_CHECKING($1)
      AC_TRY_COMPILE(
        [
            #include <stdio.h>
            #include <ind_helper.h>
        ],
        [
            CMPISelectExp *filter = NULL;
            ind_set_select("/root/cimv2",NULL,filter);
        ],
        [
            have_INDHELP=yes
            dnl AC_MSG_RESULT(yes)
        ],
        [
            have_INDHELP=no
            dnl AC_MSG_RESULT(no)
        ])
  ]
)

AC_DEFUN([CHECK_INDHELP_HEADER],
  [
    INDHELP_CPP_FLAGS="$CPPFLAGS"
    AC_MSG_CHECKING(for indication helper header)

    dnl Check just with the standard include paths
    _CHECK_INDHELP_HEADER(standard)
    if test "$have_INDHELP" == "yes"; then
        dnl The standard include paths worked.
        AC_MSG_RESULT(yes)
    else
        _DIRS_="/usr/include/sblim \
                /usr/local/include/sblim"
        for _DIR_ in $_DIRS_ 
        do
            _cppflags=$CPPFLAGS
            _include_INDHELP="$_DIR_"
            CPPFLAGS="$CPPFLAGS -I$_include_INDHELP"
            _CHECK_INDHELP_HEADER($_DIR_)
            if test "$have_INDHELP" == "yes"; then
                dnl Found it
                AC_MSG_RESULT(yes)
                dnl Save the new -I parameter
                INDHELP_CPP_FLAGS="$CPPFLAGS"
                break
            fi
            CPPFLAGS=$_cppflags
        done
    fi

    CPPFLAGS="$INDHELP_CPP_FLAGS"

    if test "$have_INDHELP" == "no"; then
        AC_MSG_RESULT(no)
    fi
  ]
)

