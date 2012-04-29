
#include <stdio.h>

#include "k8055.h"


int main(int argc, char *params[])
{
  k8055_t *dev = k8055_device_open(0);
  int i;
  
  if (dev) {
	  printf("Successfully opened K8055 device.\n");
  }
  
  k8055_device_init(dev);
  
  for(i=1; i<=8; i++) {
    k8055_digital_out_set_channel(dev, i);
    k8055_device_poll(dev);
    sleep(1);
    k8055_digital_out_clear_channel(dev, i);
    k8055_device_poll(dev);
  }

  k8055_digital_out_set(dev, 0x00);
  k8055_device_poll(dev);

  k8055_device_close(dev);
}
