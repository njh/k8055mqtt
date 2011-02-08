#ifndef _K8055_H_
#define _K8055_H_


#define K8055_VENDOR_ID       (0x10cf)
#define K8055_PRODUCT_ID      (0x5500)
#define K8055_INTERFACE       (0x00)

#define K8055_CMD_INIT        (0x00)
#define K8055_CMD_SETDBT1     (0x01)
#define K8055_CMD_SETDBT2     (0x02)
#define K8055_CMD_RESETCNT1   (0x03)
#define K8055_CMD_RESETCNT2   (0x04)
#define K8055_CMD_SETOUTPUT   (0x05)


typedef struct k8055_s k8055_t;

k8055_t* k8055_device_open(int card_address);
int k8055_device_init(k8055_t* dev);
int k8055_device_close(k8055_t* dev);

int k8055_digital_set(k8055_t* dev, int value);
int k8055_digital_get(k8055_t* dev);
int k8055_digital_get_channel(k8055_t* dev, int channel);
int k8055_digital_set_channel(k8055_t* dev, int channel);
int k8055_digital_clear_channel(k8055_t* dev, int channel);


#endif
