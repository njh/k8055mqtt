#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "k8055.h"
#include "redhttp.h"


#define DEFAULT_PORT      "8055"

int running = 1;


static void print_help(char *pname)
{
  fprintf(stderr, "%s [option]\n"
          " -f [46]: specify family\n"
          " -a : specify bind address\n"
          " -p : specify port (default 9999)\n" " -h: help\n", pname);
}


static redhttp_response_t *handle_homepage(redhttp_request_t * request, void *user_data)
{
  redhttp_response_t *response = redhttp_response_new_with_type(REDHTTP_OK, NULL, "text/html");
  FILE *socket = redhttp_request_get_socket(request);
  k8055_t *dev = (k8055_t*)user_data;
  int i;
  
  redhttp_response_send(response, request);

  fprintf(socket, "<html><head><title>K8055 Browser Interface</title></head>\n");
  fprintf(socket, "<body><h1>K8055 Browser Interface</h1>\n");
  fprintf(socket, "<form action=\"/outputs\" method=\"post\">\n");
  fprintf(socket, "<table border=\"1\" cellpadding=\"4\">\n");
  for(i=1; i<=8; i++) {
    fprintf(socket, "<tr><th>Digital Output %d</th>", i);
    fprintf(socket, "  <td>%d</td>\n", k8055_digital_get_channel(dev, i));
    fprintf(socket, "  <td><input type=\"submit\" name=\"d%d\" value=\"On\" /></td>\n", i);
    fprintf(socket, "  <td><input type=\"submit\" name=\"d%d\" value=\"Off\" /></td>\n", i);
    fprintf(socket, "</tr>\n");
  }
  fprintf(socket, "</table>\n");
  fprintf(socket, "</form>\n");
  fprintf(socket, "</body></html>");

  return response;
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

static redhttp_response_t *handle_get_digital_outputs(redhttp_request_t * request, void *user_data)
{
  redhttp_response_t *response = redhttp_response_new_with_type(REDHTTP_OK, NULL, "text/plain");
  k8055_t *dev = (k8055_t*)user_data;
  char data[10];
  
  snprintf(data, sizeof(data), "%d", k8055_digital_get(dev));  
  redhttp_response_copy_content(response, data, strlen(data));

  return response;
}

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


static redhttp_response_t *handle_logging(redhttp_request_t * request, void *user_data)
{
  printf("[%s:%s] %s: %s\n",
         redhttp_request_get_remote_addr(request),
         redhttp_request_get_remote_port(request),
         redhttp_request_get_method(request), redhttp_request_get_path(request)
      );
  return NULL;
}



int main(int argc, char **argv)
{
  sa_family_t sopt_family = PF_UNSPEC;  // PF_UNSPEC, PF_INET, PF_INET6
  char *sopt_host = NULL;       // nodename for getaddrinfo(3)
  char *sopt_service = DEFAULT_PORT;  // service name: "pop", "110"
  redhttp_server_t *server;
  int verbose = 0;
  k8055_t *dev = NULL;
  int c;


  while ((c = getopt(argc, argv, "vf:a:p:h")) != EOF) {
    switch (c) {
    case 'v':
      verbose = 1;
    break;
    case 'f':
      if (!strncmp("4", optarg, 1)) {
        sopt_family = PF_INET;
      } else if (!strncmp("6", optarg, 1)) {
        sopt_family = PF_INET6;
      } else {
        print_help(argv[0]);
        exit(EXIT_FAILURE);
      }
      break;
    case 'a':
      sopt_host = optarg;
      break;
    case 'p':
      sopt_service = optarg;
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

  server = redhttp_server_new();
  if (!server) {
    fprintf(stderr, "Failed to initialise HTTP server.\n");
    exit(EXIT_FAILURE);
  }

  redhttp_server_add_handler(server, NULL, NULL, handle_logging, NULL);
  redhttp_server_add_handler(server, "GET", "/", handle_homepage, dev);
  redhttp_server_add_handler(server, "GET", "/outputs", handle_get_digital_outputs, dev);
  redhttp_server_add_handler(server, "POST", "/outputs", handle_post_digital_outputs, dev);
  redhttp_server_set_signature(server, "k8055httpd/0.1");

  if (redhttp_server_listen(server, sopt_host, sopt_service, sopt_family)) {
    fprintf(stderr, "Failed to create HTTP socket.\n");
    exit(EXIT_FAILURE);
  }

  printf("Listening on: %s\n", sopt_service);

  while (running) {
    redhttp_server_run(server);
  }

  redhttp_server_free(server);
  
  k8055_device_close(dev);

  return 0;
}
