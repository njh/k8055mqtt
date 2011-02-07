#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

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
  const char page[] =
      "<html><head><title>Homepage</title></head>"
      "<body><h1>Homepage</h1>"
      "<p>This is the homepage.</p>"
      "</body></html>\n";

  redhttp_response_copy_content(response, page, sizeof(page));

  return response;
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
  int c;


  while ((c = getopt(argc, argv, "f:a:p:h")) != EOF) {
    switch (c) {
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


  server = redhttp_server_new();
  if (!server) {
    fprintf(stderr, "Failed to initialise HTTP server.\n");
    exit(EXIT_FAILURE);
  }

  redhttp_server_add_handler(server, NULL, NULL, handle_logging, NULL);
  redhttp_server_add_handler(server, "GET", "/", handle_homepage, NULL);
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

  return 0;
}
