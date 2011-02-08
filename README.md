k8055httpd
==========

Self contained HTTP server for controlling the the Velleman K8055 / VM110 Board.


Requirements
------------

  libusb version 1.0.8 or higher
  

Examples
--------

Starting the server on port 8055:

    ./src/k8055httpd -p 8055

Turn on digital output 1:
  
    curl -d d1=on http://127.0.0.1:8055/outputs
    
Turn off all digital outputs:
  
    curl -d d=0 http://127.0.0.1:8055/outputs

