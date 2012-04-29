#ifndef K8055_H
#define K8055_H

typedef struct k8055_s k8055_t;

k8055_t* k8055_device_open(int card_address);
int k8055_device_reset(k8055_t* dev);
int k8055_device_poll(k8055_t* dev);
int k8055_device_close(k8055_t* dev);

void k8055_digital_out_set(k8055_t* dev, int value);
int k8055_digital_out_get(k8055_t* dev);
int k8055_digital_out_get_channel(k8055_t* dev, int channel);
void k8055_digital_out_set_channel(k8055_t* dev, int channel);
void k8055_digital_out_clear_channel(k8055_t* dev, int channel);

int k8055_digital_in_get(k8055_t* dev);
int k8055_counter_get(k8055_t* dev, int channel);
int k8055_counter_reset(k8055_t* dev, int channel);

void k8055_analogue_out_set(k8055_t* dev, int channel, int value);
int k8055_analogue_out_get(k8055_t* dev, int channel);

int k8055_analogue_in_get(k8055_t* dev, int channel);


#endif
