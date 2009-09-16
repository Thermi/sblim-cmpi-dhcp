dnl
 dnl sblim-testsuite.m4
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
 dnl               Wolfgang Taphotn   <taphorn (at)de.ibm.com>
 dnl
 dnl Description:
 dnl The check for the SBLIM test suite
 dnl Sets the TESTSUITEDIR variable and the TESTSUITE conditional
dnl

AC_DEFUN([CHECK_SBLIM_TESTSUITE],
  [
      AC_MSG_CHECKING(for SBLIM TestSuite)
      
      if test x"$TESTSUITEDIR" != x ; then
          AC_MSG_CHECKING($TESTSUITEDIR)
          if test -x `eval echo $TESTSUITEDIR`/run.sh ; then
              AC_MSG_RESULT(yes)
              AC_SUBST(TESTSUITEDIR)
          else
              AC_MSG_RESULT(no)
              AC_MSG_WARN(Continuing with not existend user defined SBLIM TestSuite path $TESTSUITEDIR.)
          fi
      else
          _DIRS="$datadir/sblim-testsuite"
          save_exec_prefix=${exec_prefix}
          save_prefix=${prefix}
          
          if test xNONE == x${prefix}; then
              prefix=/usr/local
          fi
          
          if test xNONE == x${exec_prefix}; then
              exec_prefix=$prefix
          fi
          
          for _name in $_DIRS ; do
              AC_MSG_CHECKING( $_name )
              _xname=`eval echo $_name`
              
              if test -x $_xname/run.sh ; then
                  dnl Found it
                  AC_MSG_RESULT(yes)
                  
                  if test x"$TESTSUITEDIR" == x; then
                      TESTSUITEDIR=$_name
                  fi
                  
                  AC_SUBST(TESTSUITEDIR)
                  break;
              fi
          done
          
          if test x"$TESTSUITEDIR" == x ; then
              AC_MSG_RESULT(no)
          fi
          
          exec_prefix=$save_exec_prefix
          prefix=$save_prefix
      fi
      AM_CONDITIONAL(SBLIM_TESTSUITE,[test $TESTSUITEDIR])
  ]
)

