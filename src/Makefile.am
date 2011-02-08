AM_CFLAGS = -Iredhttp $(LIBUSB_CFLAGS)

bin_PROGRAMS = k8055httpd test_k8055
k8055httpd_LDADD = $(LIBUSB_LIBS) redhttp/libredhttp.la
k8055httpd_SOURCES = k8055httpd.c k8055.c k8055.h

test_k8055_LDADD = $(LIBUSB_LIBS)
test_k8055_SOURCES = test_k8055.c k8055.c k8055.h

SUBDIRS = redhttp
