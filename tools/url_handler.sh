#!/bin/sh
#
# This script is an example of how to call the appropriate viewer based upon
# the URL method
#
# Created by:  Michael Elkins <me@cs.hmc.edu> on March 10, 1997
# Modified by: Liviu Daia <daia@stoilow.imar.ro>
# Last Edited: May 26, 1997
#

url=$1
method=`echo $1 | sed 's;\(^[^:]*\):.*;\1;'`

case $method in
    ftp)
	target=`echo $url | sed 's;^.*://\([^/]*\)/*\(.*\);\1:/\2;'`
	ncftp $target
	;;

    http)   
	if test x$DISPLAY = x; then
	    lynx $url
	else
	    netscape -remote "openURL($url)" || netscape $url
	fi
	;;

    mailto)
	mutt `echo $url | sed 's;^[^:]*:\(.*\);\1;'`
	;;

    *)
	method=`echo $url | sed 's;\(^...\).*;\1;'`
	case $method in
	    ftp)
		target=`echo $url | sed 's;^\([^/]*\)/*\(.*\);\1:/\2;'`
		ncftp $target
		;;

	    www)
		target="http://"$url
		if test x$DISPLAY = x; then
		    lynx $target
		else
		    netscape -remote "openURL($target)" || netscape $target
		fi
		;;

	    *)
		mutt $url
		;;
	esac
	;;
esac
