dnl
 dnl cimom.m4
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
 dnl The "check" for the CIM server type in PATH and
 dnl the sbin directories.
 dnl Sets the CIMSERVER variable.
dnl

AC_DEFUN([CHECK_CIMSERVER],
  [
      AC_MSG_CHECKING(for CIMOM servers)
      PATH=/usr/sbin:/usr/local/sbin:$PATH

      if test x"$CIMSERVER" != x ; then
          AC_MSG_CHECKING($CIMSERVER)
          for _path in `echo $PATH | sed "s/:/ /g"` ; do
              CIMSERVER_EXE=$CIMSERVER
              case $CIMSERVER in
                  (sfcb) CIMSERVER_EXE=sfcbd;;
                  (pegasus) CIMSERVER_EXE=cimserver;;
                  (openwbem) CIMSERVER_EXE=owcimomd;;
              esac

              if test -f $_path/$CIMSERVER_EXE ; then
                  AC_MSG_RESULT(yes)
                  case $CIMSERVER_EXE in
                      (sfcbd) CIMSERVER=sfcb;;
                      (cimserver) CIMSERVER=pegasus;;
                      (owcimomd) CIMSERVER=openwbem;;
                  esac
                  have_found_cimserver=true
                  break;
              fi
          done
          if test "$have_found_cimserver" != "true" ; then
              AC_MSG_RESULT(no)
              AC_MSG_ERROR(The defined CIMOM server "$CIMSERVER" is unknown!)
          fi
      else
          _SERVERS="sfcbd cimserver owcimomd"
          _SAVE_PATH=$PATH
          for _name in $_SERVERS ; do
              AC_MSG_CHECKING( $_name )
              for _path in `echo $PATH | sed "s/:/ /g"` ; do
                  if test -f $_path/$_name ; then
                      dnl Found it
                      AC_MSG_RESULT(yes)
                      if test x"$CIMSERVER" == x ; then
                          case $_name in
                              (sfcbd) CIMSERVER=sfcb;;
                              (cimserver) CIMSERVER=pegasus;;
                              (owcimomd) CIMSERVER=openwbem;;
                          esac
                      fi
                      break;
                  fi
              done
              if test x"$CIMSERVER" != x; then
                  break;
              fi
          done
          PATH=$_SAVE_PATH
          if test x"$CIMSERVER" == x ; then
              CIMSERVER=sfcb
              AC_MSG_RESULT(implied: $CIMSERVER)
          fi
      fi
      # Cross platform only needed for sfcb currently
      if test $CIMSERVER = sfcb ; then
          AC_REQUIRE([AC_CANONICAL_HOST])
          AC_CHECK_SIZEOF(int)
          case "$build_cpu" in
              i*86) case "$host_cpu" in
                        powerpc*) if test $ac_cv_sizeof_int == 4 ; then
                                      REGISTER_FLAGS="-X P32"
                                  fi ;;
                    esac ;;
          esac
          AC_SUBST(REGISTER_FLAGS)
      fi
  ]
)

dnl
dnl The check for the CMPI provider directory
dnl Sets the PROVIDERDIR  variable.
dnl

AC_DEFUN([CHECK_CIMSERVER_PROVIDERDIR],
  [
      AC_MSG_CHECKING(for CMPI provider directory)
      
      if test x"$PROVIDERDIR" != x ; then
          AC_MSG_CHECKING($PROVIDERDIR)
          if test -d $PROVIDERDIR ; then
              AC_MSG_RESULT(yes)
          else
              AC_MSG_RESULT(no)
              AC_MSG_WARN(Continuing with not existend user defined CMPI provider directory $PROVIDERDIR.)
          fi
      else
          _DIRS="$libdir/cmpi"
          save_exec_prefix=${exec_prefix}
          save_prefix=${prefix}
          
          if test xNONE == x${prefix} ; then
              prefix=/usr/local
          fi
          if test xNONE == x${exec_prefix} ; then
              exec_prefix=$prefix
          fi
          for _dir in $_DIRS ; do
              _xdir=`eval echo $_dir`
              AC_MSG_CHECKING( $_dir )
              if test -d $_xdir ; then
                  dnl Found it
                  AC_MSG_RESULT(yes)
                  if test x"$PROVIDERDIR" == x ; then
                      PROVIDERDIR=$_dir
                  fi
                  break
              fi
          done
          if test x"$PROVIDERDIR" == x ; then
              PROVIDERDIR="$libdir"/cmpi
              AC_MSG_RESULT(implied: $PROVIDERDIR)
          fi
          exec_prefix=$save_exec_prefix
          prefix=$save_prefix
      fi
  ]
)
