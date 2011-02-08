#!/bin/sh
# Run this to set up the build system: configure, makefiles, etc.

package="k8055httpd"

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

cd "$srcdir"
DIE=0
LIBTOOLIZE=libtoolize

# Check to see if we need to use the Mac OS X libtool version
(glibtoolize --version) < /dev/null > /dev/null 2>&1 && {
    LIBTOOLIZE=glibtoolize
}

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have autoconf installed to compile $package."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/autoconf"
    DIE=1
}
(automake --version) < /dev/null > /dev/null 2>&1 || {
    echo
    echo "You must have automake installed to compile $package."
    echo "Download the appropriate package for your distribution,"
    echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/automake"
    DIE=1
}

if test "$DIE" -eq 1; then
    exit 1
fi

# Do some cleanup
rm -f config.cache config.log config.guess config.sub configure

# Because GIT doesn't support empty directories
if [ ! -d "$srcdir/build-scripts" ]; then
    mkdir "$srcdir/build-scripts"
fi

echo "Generating configuration files for $package, please wait...."

echo "  aclocal $ACLOCAL_FLAGS"
aclocal $ACLOCAL_FLAGS
echo "  autoheader"
autoheader
echo "  $LIBTOOLIZE --automake --force"
$LIBTOOLIZE --force --copy --automake
echo "  automake --add-missing"
automake --add-missing --force --copy
echo "  autoconf"
autoconf

./configure
