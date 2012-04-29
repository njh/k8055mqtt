#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>

#include <libusb.h>
#include "k8055.h"

/*
  This library is based on libk8055 by Sven Lindberg and Pjetur G. Hjaltason

	Input packet format

	+---+---+---+---+---+---+---+---+
	|DIn|Sta|A1 |A2 |   C1  |   C2  |
	+---+---+---+---+---+---+---+---+
	DIn = Digital input in high nibble, except for input 3 in 0x01
	Sta = Status, Board number + 1
	A1  = Analog input 1, 0-255
	A2  = Analog input 2, 0-255
	C1  = Counter 1, 16 bits (lsb)
	C2  = Counter 2, 16 bits (lsb)

	Output packet format

	+---+---+---+---+---+---+---+---+
	|CMD|DIG|An1|An2|Rs1|Rs2|Dbv|Dbv|
	+---+---+---+---+---+---+---+---+
	CMD = Command 
	DIG = Digital output bitmask
	An1 = Analog output 1 value, 0-255
	An2 = Analog output 2 value, 0-255
	Rs1 = Reset counter 1, command 3
	Rs2 = Reset counter 3, command 4
	Dbv = Debounce value for counter 1 and 2, command 1 and 2

	Or split by commands

	Cmd 0, Reset ??
	Cmd 1, Set debounce Counter 1
	+---+---+---+---+---+---+---+---+
	|CMD|   |   |   |   |   |Dbv|   |
	+---+---+---+---+---+---+---+---+
	Cmd 2, Set debounce Counter 2
	+---+---+---+---+---+---+---+---+
	|CMD|   |   |   |   |   |   |Dbv|
	+---+---+---+---+---+---+---+---+
	Cmd 3, Reset counter 1
	+---+---+---+---+---+---+---+---+
	| 3 |   |   |   | 00|   |   |   |
	+---+---+---+---+---+---+---+---+
	Cmd 4, Reset counter 2
	+---+---+---+---+---+---+---+---+
	| 4 |   |   |   |   | 00|   |   |
	+---+---+---+---+---+---+---+---+
	cmd 5, Set analog/digital
	+---+---+---+---+---+---+---+---+
	| 5 |DIG|An1|An2|   |   |   |   |
	+---+---+---+---+---+---+---+---+

**/


#define K8055_VENDOR_ID        (0x10cf)
#define K8055_PRODUCT_ID       (0x5500)
#define K8055_INTERFACE        (0x00)
#define K8055_OUTPUT_ENDPOINT  (0x01)
#define K8055_INPUT_ENDPOINT   (0x81)
#define K8055_USB_TIMEOUT      (22)

#define K8055_CMD_RESET            (0x00)
#define K8055_CMD_SET_DEBOUNCE_1   (0x01)
#define K8055_CMD_SET_DEBOUNCE_2   (0x01)
#define K8055_CMD_RESET_COUNTER_1  (0x03)
#define K8055_CMD_RESET_COUNTER_2  (0x04)
#define K8055_CMD_SET_OUTPUT       (0x05)


struct k8055_s {
  libusb_context *usb_context;
  libusb_device_handle *usb_handle;
  uint8_t card_address;

  uint8_t digital_output;
  uint8_t digital_input;
  uint8_t analogue_outputs[2];
  uint8_t analogue_inputs[2];
  uint16_t counters[2];
  
  uint8_t write_pending;
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

static int k8055_device_write(k8055_t* dev, char data[8])
{
  int transferred = 0;
  int res = 0;

  res = libusb_interrupt_transfer(dev->usb_handle, K8055_OUTPUT_ENDPOINT, data, sizeof(data), &transferred, K8055_USB_TIMEOUT);
  if (res == 0 && transferred == sizeof(data)) {
    return 0;
  } else {
    fprintf(stderr, "k8055_device_write() failed.\n");
    return -1;
  }
}

int k8055_device_reset(k8055_t* dev)
{
  char data[8] = {K8055_CMD_RESET, 0, 0, 0, 0, 0, 0, 0};
  return k8055_device_write(dev, data);
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


int k8055_device_poll(k8055_t* dev)
{
  unsigned char data[8] = {0,0,0,0,0,0,0,0};
  int transferred = 0;
  int res;

  assert(dev != NULL);

  // Write if anything need updating
  if (dev->write_pending) {
    data[0] = K8055_CMD_SET_OUTPUT;
    data[1] = dev->digital_output;
    data[2] = dev->analogue_outputs[0];
    data[3] = dev->analogue_outputs[1];
  
    res = k8055_device_write(dev, data);
    if (res) return res;
    dev->write_pending = 0;
  }
  
  // Read in current state
  res = libusb_interrupt_transfer(dev->usb_handle, K8055_INPUT_ENDPOINT,
                                  data, sizeof(data), &transferred, K8055_USB_TIMEOUT);
  if (res == 0 && transferred == sizeof(data)) {
    dev->digital_input = (((data[0] >> 4) & 0x03) |  /* Input 1 and 2 */
                          ((data[0] << 2) & 0x04) |  /* Input 3 */
                          ((data[0] >> 3) & 0x18));  /* Input 4 and 5 */
    dev->analogue_inputs[0] = data[2];
    dev->analogue_inputs[1] = data[3];
    dev->counters[0] = ((uint16_t)data[5] << 8) | data[4];
    dev->counters[1] = ((uint16_t)data[7] << 8) | data[6];
  } else {
    fprintf(stderr, "k8055_device_poll() failed to read.\n");
    return -1;
  }
  
  return 0;
}


void k8055_digital_out_set(k8055_t* dev, int value)
{
    assert(dev != NULL);
    dev->digital_output = value;
    dev->write_pending = 1;
}


int k8055_digital_out_get(k8055_t* dev)
{
    assert(dev != NULL);
    return dev->digital_output;
}


int k8055_digital_out_get_channel(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    assert(channel > 0 && channel < 9);
    return (dev->digital_output & (0x1 << (channel-1))) ? 1 : 0;
}


void k8055_digital_out_set_channel(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    assert(channel > 0 && channel < 9);
    dev->digital_output |= (0x1 << (channel-1));
    dev->write_pending = 1;
}


void k8055_digital_out_clear_channel(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    assert(channel > 0 && channel < 9);
    dev->digital_output &= ~(0x1 << (channel-1));
    dev->write_pending = 1;
}

int k8055_digital_in_get(k8055_t* dev)
{
    return dev->digital_input;
}

int k8055_counter_get(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    if (channel == 1)
      return dev->counters[0];
    else if (channel = 2)
      return dev->counters[1];

    return -1;
}

int k8055_counter_reset(k8055_t* dev, int channel)
{
  unsigned char data[8] = {0,0,0,0,0,0,0,0};

  if (channel == 1) {
    data[0] = K8055_CMD_RESET_COUNTER_1;
    data[4] = 0x00;
  } else if (channel == 2) {
    data[0] = K8055_CMD_RESET_COUNTER_2;
    data[5] = 0x00;
  } else {
    return -1;
  }

  return k8055_device_write(dev, data);
}

void k8055_analogue_out_set(k8055_t* dev, int channel, int value)
{
    assert(dev != NULL);
    if (channel == 1)
      dev->analogue_outputs[0] = value;
    else if (channel = 2)
      dev->analogue_outputs[1] = value;
    dev->write_pending = 1;
}

int k8055_analogue_out_get(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    if (channel == 1)
      return dev->analogue_outputs[0];
    else if (channel = 2)
      return dev->analogue_outputs[1];

    return -1;
}

int k8055_analogue_in_get(k8055_t* dev, int channel)
{
    assert(dev != NULL);
    if (channel == 1)
      return dev->analogue_outputs[0];
    else if (channel = 2)
      return dev->analogue_outputs[1];

    return -1;
}
