AM_CFLAGS = $(LIBUSB_CFLAGS)

bin_PROGRAMS = k8055mqtt
k8055mqtt_LDADD = $(LIBUSB_LIBS) -lmosquitto
k8055mqtt_SOURCES = k8055mqtt.c k8055.c k8055.h

noinst_PROGRAMS = test_k8055
test_k8055_LDADD = $(LIBUSB_LIBS)
test_k8055_SOURCES = test_k8055.c k8055.c k8055.h
