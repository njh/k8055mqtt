#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "k8055.h"


#define DEFAULT_MQTT_PORT      "1883"

int running = 1;


static void print_help(char *pname)
{
  fprintf(stderr, "%s [option]\n"
          " -s : specify the MQTT server to connect to\n"
          " -p : specify MQTT port (default %s)\n"
          " -h : help\n", pname, DEFAULT_MQTT_PORT);
}



/*
static int get_channel_number(redhttp_request_t * request)
{
    const char* glob = redhttp_request_get_path_glob(request);
    int result, channel = 0;
    
    if (!glob || strlen(glob) != 1)
      return -1;

    result = sscanf(glob, "%d", &channel);
    if (result != 1)
      return -1;
    else if (channel < 1 || channel > 8)
      return -1;
    else
      return channel;
}
*/

static int is_true(const char* str)
{
  if (str && (
        strcmp(str, "On") == 0 ||
        strcmp(str, "on") == 0 ||
        strcmp(str, "true") == 0 ||
        strcmp(str, "True") == 0 ||
        strcmp(str, "1") == 0
     ))
    return 1;
  else
    return 0;
}

/*
static redhttp_response_t *handle_get_digital_outputs(redhttp_request_t * request, void *user_data)
{
  redhttp_response_t *response = redhttp_response_new_with_type(REDHTTP_OK, NULL, "text/plain");
  k8055_t *dev = (k8055_t*)user_data;
  char data[10];
  
  snprintf(data, sizeof(data), "%d", k8055_digital_get(dev));  
  redhttp_response_copy_content(response, data, strlen(data));

  return response;
}
*/

/*
static redhttp_response_t *handle_post_digital_outputs(redhttp_request_t * request, void *user_data)
{
  k8055_t *dev = (k8055_t*)user_data;
  const char *key, *value;
  int i;
  
  for (i=0; 1; i++) {
    if (!redhttp_request_get_argument_index(request, i, &key, &value))
      break;
    
    if (key != NULL && strlen(key) == 2 && key[0] == 'd') {
      int channel = atoi(&key[1]);
      if (is_true(value)) {
        printf("Turning On : %d\n", channel);
        k8055_digital_set_channel(dev, channel);
      } else {
        printf("Turning Off : %d\n", channel);
        k8055_digital_clear_channel(dev, channel);
      }
    } else if (key != NULL && strlen(key) == 1 && key[0] == 'd') {
      int v = atoi(value);
      printf("Setting digital output : %d\n", v);
      k8055_digital_set(dev, v);
    } else {
      printf("Unknown key: %s\n", key);
    }
  }

  return redhttp_response_new_redirect("/", REDHTTP_SEE_OTHER);
}
*/


int main(int argc, char **argv)
{
  const char* mqtt_server = NULL;
  const char* mmqtt_port = DEFAULT_MQTT_PORT;
  k8055_t *dev = NULL;
  int c;


  while ((c = getopt(argc, argv, "f:a:p:h")) != EOF) {
    switch (c) {
    case 's':
      mqtt_server = optarg;
      break;
    case 'p':
      mqtt_port = optarg;
      break;
    case 'h':
    default:
      print_help(argv[0]);
      exit(EXIT_SUCCESS);
      break;
    }
  }


  
  dev = k8055_device_open(0);
  if (!dev) {
    fprintf(stderr, "Failed to initialise K8055 device.\n");
    exit(EXIT_FAILURE);
  }

  printf("Connecting to: %s\n", mqtt_server);

  while (running) {

  }

  k8055_device_close(dev);

  return 0;
}
