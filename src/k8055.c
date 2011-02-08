#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>

#include <libusb.h>
#include "k8055.h"


struct k8055_s {
  libusb_context *usb_context;
  libusb_device_handle *usb_handle;
  
  unsigned char card_address;
  unsigned char digital_output;
  unsigned char analogue_outputs[2];
  
};


k8055_t* k8055_device_open(int card_address)
{
  k8055_t* dev = NULL;
  int product_id = K8055_PRODUCT_ID + card_address;

  if (card_address < 0 || card_address > 4)
    return NULL;
  
  dev = malloc(sizeof(k8055_t));
  if (!dev)
    return NULL;
  
  // Initialise memory structure
  memset(dev, 0, sizeof(k8055_t));

  // Initialise libusb
  libusb_init(&dev->usb_context);
  libusb_set_debug(dev->usb_context, 1);

  // Find the device
  dev->card_address = card_address;
  dev->usb_handle = libusb_open_device_with_vid_pid(dev->usb_context, K8055_VENDOR_ID, product_id);
  if (!dev->usb_handle) {
		fprintf(stderr, "libusb_open_device_with_vid_pid() error.\n");
    k8055_device_close(dev);
    return NULL;
  }
  
  if (libusb_set_configuration(dev->usb_handle, 1)) {
		fprintf(stderr, "libusb_set_configuration() error.\n");
    k8055_device_close(dev);
    return NULL;
  }

  if (libusb_kernel_driver_active(dev->usb_handle, K8055_INTERFACE)) {
		fprintf(stderr, "Kernel driver is active for device, attempting to detatch.\n");
    if (libusb_detach_kernel_driver(dev->usb_handle, K8055_INTERFACE)) {
      fprintf(stderr, "libusb_detach_kernel_driver() error.\n");
      k8055_device_close(dev);
      return NULL;
    }
  }
  
	if (libusb_claim_interface(dev->usb_handle, K8055_INTERFACE)) {
		fprintf(stderr, "libusb_claim_interface() error.\n");
    k8055_device_close(dev);
    return NULL;
	}
  
  return dev;
}


int k8055_device_init(k8055_t* dev)
{
  unsigned char command = K8055_CMD_INIT;
  int transfered = 0;

  assert(dev != NULL);
  if (libusb_interrupt_transfer(dev->usb_handle, 0x1, &command, sizeof(command), &transfered, 20)) {
    printf("k8055_device_init() failed.\n");
  }
  
  return transfered;
}


int k8055_device_close(k8055_t* dev)
{
  if (dev) {
    if (dev->usb_handle)
      libusb_release_interface(dev->usb_handle, 0);
    if (dev->usb_handle)
	    libusb_close(dev->usb_handle);
    if (dev->usb_context)
      libusb_exit(dev->usb_context);
    free(dev);
  }
}


static int k8055_output_sync(k8055_t* dev)
{
  unsigned char data[4];
  int transfered = 0;

  assert(dev != NULL);

  data[0] = K8055_CMD_SETOUTPUT;
  data[1] = dev->digital_output;
  data[2] = dev->analogue_outputs[0];
  data[3] = dev->analogue_outputs[1];

  if (libusb_interrupt_transfer(dev->usb_handle, 0x1, data, sizeof(data), &transfered, 20)) {
    printf("k8055_sync_output() failed.\n");
  }
  
  return transfered;
}


int k8055_digital_set(k8055_t* dev, int value)
{
    assert(dev != NULL);
    dev->digital_output = value;
    return k8055_output_sync(dev);
}


int k8055_digital_get(k8055_t* dev)
{
    assert(dev != NULL);
    return dev->digital_output;
}


int k8055_digital_get_channel(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    assert(channel > 0 && channel < 9);
    return (dev->digital_output & (0x1 << (channel-1))) ? 1 : 0;
}


int k8055_digital_set_channel(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    assert(channel > 0 && channel < 9);
    dev->digital_output |= (0x1 << (channel-1));
    return k8055_output_sync(dev);
}


int k8055_digital_clear_channel(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    assert(channel > 0 && channel < 9);
    dev->digital_output &= ~(0x1 << (channel-1));
    return k8055_output_sync(dev);
}
