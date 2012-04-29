
#include <stdio.h>

#include "k8055.h"


int main(int argc, char *params[])
{
  k8055_t *dev = k8055_device_open(0);
  int i;
  
  if (dev) {
	  printf("Successfully opened K8055 device.\n");
  }
  
  k8055_device_reset(dev);
  k8055_counter_reset(dev, 1);
  k8055_counter_reset(dev, 2);

  for(i=1; i<=8; i++) {
    k8055_digital_out_set_channel(dev, i);
    k8055_device_poll(dev);
    sleep(1);
    k8055_digital_out_clear_channel(dev, i);
    k8055_device_poll(dev);

    printf("Digital: 0x%2.2x\n", k8055_digital_in_get(dev));
    printf("Analogue: %d, %d\n", k8055_analogue_in_get(dev, 1), k8055_analogue_in_get(dev, 2));
    printf("Counters: %d, %d\n", k8055_counter_get(dev, 1), k8055_counter_get(dev, 2));
  }

  k8055_digital_out_set(dev, 0x00);
  k8055_device_poll(dev);

  k8055_device_close(dev);
}
