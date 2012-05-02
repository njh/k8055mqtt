#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>

#include <mosquitto.h>
#include "k8055.h"
#include "config.h"

#define MAX_TOPIC_LEN         (127)
#define DEFAULT_MQTT_PORT     (1883)
#define DEFAULT_KEEP_ALIVE    (60)
#define DEFAULT_PREFIX        "k8055"
#define DEFAULT_CLIENT_ID     "k8055"

struct mosquitto *mosq = NULL;
char *mqtt_prefix = DEFAULT_PREFIX;
char *mqtt_client_id = DEFAULT_CLIENT_ID;
int mqtt_connected = 0;
int keep_running = 1;
int exit_code = EXIT_SUCCESS;
k8055_t *dev = NULL;



static void connect_callback(void *obj, int result)
{
  if(!result){
    printf("Connected to MQTT server.\n");
    mqtt_connected = 1;
  } else {
    switch(result) {
      case 0x01:
        fprintf(stderr, "Connection Refused: unacceptable protocol version\n");
        break;
      case 0x02:
        fprintf(stderr, "Connection Refused: identifier rejected\n");
        break;
      case 0x03:
        // FIXME: if broker is unavailable, sleep and try connecting again
        fprintf(stderr, "Connection Refused: broker unavailable\n");
        break;
      case 0x04:
        fprintf(stderr, "Connection Refused: bad user name or password\n");
        break;
      case 0x05:
        fprintf(stderr, "Connection Refused: not authorised\n");
        break;
      default:
        fprintf(stderr, "Connection Refused: unknown reason\n");
        break;
    }

    mqtt_connected = 0;
  }
}


static void disconnect_callback(void *obj)
{
  mqtt_connected = 0;

  // FIXME: re-establish the connection
  // FIXME: keep count of re-connects
}

static void message_callback(void *userdata, const struct mosquitto_message *mesg)
{
  size_t prefix_len = strlen(mqtt_prefix);
  size_t topic_len = 0;
  char* topic = NULL;
  int value = 0;

  // Check that the topic prefix matches
  if (strncmp(mesg->topic, mqtt_prefix, prefix_len) != 0) {
    printf("Topic didn't match: %s\n", mesg->topic);
    return;
  } else {
    topic = mesg->topic + prefix_len;
    topic_len = strlen(topic);
  }

  // Check the topic name
  if (topic_len == 12 && strncmp(topic, "/digital/out", topic_len) == 0) {
    if (sscanf((char*)mesg->payload, "%i", &value) == 1) {
      printf("Setting digital to: 0x%2.2x\n", value);
      k8055_digital_out_set(dev, value);
    }
  } else if (topic_len == 14 && strncmp(topic, "/digital/out/", 13) == 0) {
    if (sscanf((char*)mesg->payload, "%i", &value) == 1) {
      int channel = atoi(&topic[13]);
      printf("Setting digital channel %d to: %d\n", channel, value);
      if (value) {
        k8055_digital_out_set_channel(dev, channel);
      } else {
        k8055_digital_out_clear_channel(dev, channel);
      }
    }
  } else if (topic_len == 15 && strncmp(topic, "/analogue/out/", 14) == 0) {
    if (sscanf((char*)mesg->payload, "%i", &value) == 1) {
      int channel = atoi(&topic[14]);
      printf("Setting analogue channel %d to: %d\n", channel, value);
      k8055_analogue_out_set(dev, channel, value);
    }
  } else if (topic_len == 10 && strncmp(topic, "/counter/", 9) == 0) {
    if (strncmp((char*)mesg->payload, "reset", 5)==0) {
      int channel = atoi(&topic[9]);
      printf("Resettting counter %d\n", channel);
      k8055_counter_reset(dev, channel);
    }
  } else {
    printf("Unhandled message to %s: %s\n", topic, (char*)mesg->payload);
  }
}


static mqtt_subscribe(struct mosquitto *mosq, const char* topic)
{
  uint16_t mid;
  int res;

  res = mosquitto_subscribe(mosq, &mid, topic, 0);
  // FIXME: check for success
  // FIXME: wait for acknowledgement
}

static struct mosquitto * mqtt_initialise(const char* host, int port)
{
  struct mosquitto *mosq = NULL;
  char subscribe_path[MAX_TOPIC_LEN+1];
  int res = 0;

  // Create a new MQTT client
  mosq = mosquitto_new(mqtt_client_id, NULL);
  if (!mosq) {
    fprintf(stderr, "Failed to initialise MQTT client.\n");
    return NULL;
  }

  // Configure logging
  mosquitto_log_init(mosq, MOSQ_LOG_INFO | MOSQ_LOG_WARNING | MOSQ_LOG_ERR, MOSQ_LOG_STDOUT);

  // FIXME: add support for username and password

  mosquitto_connect_callback_set(mosq, connect_callback);
  mosquitto_disconnect_callback_set(mosq, disconnect_callback);
  mosquitto_message_callback_set(mosq, message_callback);

  printf("Connecting to %s:%d...\n", host, port);
  res = mosquitto_connect(mosq, host, port, DEFAULT_KEEP_ALIVE, 1);
  if (res) {
    fprintf(stderr, "Unable to connect (%d).\n", res);
    mosquitto_destroy(mosq);
    return NULL;
  }

  // Wait until connected
  while (!mqtt_connected) {
    // FIXME: add timeout
    int res = mosquitto_loop(mosq, 500);
    if (res != MOSQ_ERR_SUCCESS) exit(EXIT_FAILURE);
  }

  // Subscribe to every message starting with prefix
  snprintf(subscribe_path, MAX_TOPIC_LEN, "%s/#", mqtt_prefix);
  mqtt_subscribe(mosq, subscribe_path);
  // FIXME: check for error

  return mosq;
}

static void termination_handler(int signum)
{
  switch(signum) {
    case SIGHUP:  printf("Received SIGHUP, exiting.\n"); break;
    case SIGTERM: printf("Received SIGTERM, exiting.\n"); break;
    case SIGINT:  printf("Received SIGINT, exiting.\n"); break;
  }

  keep_running = 0;
}

static void usage()
{
  printf("%s version %s\n\n", PACKAGE_NAME, PACKAGE_VERSION);
  printf("Usage: %s [options]\n", PACKAGE_TARNAME);
  printf("   -h <host>       the MQTT server to connect to (required)\n");
  printf("   -p <port>       the MQTT port to connect to (default %d)\n", DEFAULT_MQTT_PORT);
  printf("   -t <prefix>     the MQTT topic prefix (default %s)\n", DEFAULT_PREFIX);
  printf("   -i <clientid>   the MQTT Client ID (default %s)\n", DEFAULT_CLIENT_ID);
  printf("   -?              This help message.\n");
}


int main(int argc, char **argv)
{
  const char* mqtt_host = NULL;
  int mqtt_port = DEFAULT_MQTT_PORT;
  int res;
  int c;

  // Make stdout unbuffered for logging/debugging
  setbuf(stdout, NULL);

  // Parse Switches
  while ((c = getopt(argc, argv, "h:pt:i:?")) != EOF) {
    switch (c) {
      case 'h':
        mqtt_host = optarg;
        break;
      case 'p':
        mqtt_port = atoi(optarg);
        break;
      case 't':
        mqtt_prefix = optarg;
        break;
      case 'i':
        mqtt_client_id = optarg;
        break;
      case '?':
      default:
        usage();
        exit(EXIT_SUCCESS);
        break;
    }
  }
  
  if (mqtt_host == NULL) {
    usage();
    exit(EXIT_FAILURE);
  }

  // Setup signal handlers - so we exit cleanly
  signal(SIGTERM, termination_handler);
  signal(SIGINT, termination_handler);
  signal(SIGHUP, termination_handler);

  // Connect to the USB device
  dev = k8055_device_open(0);
  if (!dev) {
    fprintf(stderr, "Failed to initialise K8055 device.\n");
    exit(EXIT_FAILURE);
  }

  // Create MQTT client
  mosq = mqtt_initialise(mqtt_host, mqtt_port);
  if (!mosq) exit(EXIT_FAILURE);

  while (keep_running) {
    // Wait for network packets for a maximum of 0.5s
    res = mosquitto_loop(mosq, 500);
    
    // Write output changes and poll for inputs
    k8055_device_poll(dev);
  }

  // Clean up
  if (mosq) mosquitto_destroy(mosq);
  k8055_device_close(dev);

  // exit_code is non-zero if something went wrong
  return exit_code;
}