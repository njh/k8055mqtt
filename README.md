k8055mqtt
=========

Self contained MQTT daemon for controlling the the Velleman K8055 / VM110 Board.


Requirements
------------

  libusb version 1.0.8 or higher
  

Examples
--------

Starting the daemon:

    ./src/k8055mqtt -h test.mosquitto.org

Turn on digital output 1:
  
    mosqutto_pub -h test.mosquitto.org -t 'k8055/digital/1' -m 1
    
Turn off all digital outputs:
  
    mosqutto_pub -h test.mosquitto.org -t 'k8055/digital' -m 0
