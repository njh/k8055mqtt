k8055mqtt
=========

Self contained MQTT daemon for controlling the the [Velleman] [K8055] / [VM110] Board.


Requirements
------------

  [libusb] version 1.0.8 or higher
  [mosquitto] library version 0.14 or higher


Usage
-----


Examples
--------

Starting the daemon:

    ./src/k8055mqtt -h test.mosquitto.org

Turn on digital output 1:

    mosquitto_pub -h test.mosquitto.org -t k8055/digital/out/1 -m 1

Turn off all digital outputs:

    mosquitto_pub -h test.mosquitto.org -t k8055/digital/out -m 0x00

Set analogue output channel 1 to 50%:

    mosquitto_pub -h test.mosquitto.org -t k8055/analogue/out/1 -m 127


Installation
------------
k8055mqtt uses a standard automake build process:

    ./configure
    make
    make install


Credits
-------

k8055mqtt is based on [k8055 for Mac OS X and Linux] by Julien Etelain and Edward Nys.


License
-------

This program is free software: you can redistribute it and/or modify
it under the terms of the [GNU General Public License] as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


[Velleman]:                       http://www.velleman.eu/
[K8055]:                          http://www.velleman.eu/products/view/?id=351346
[VM110]:                          http://www.velleman.eu/products/view/?id=351980
[libusb]:                         http://libusb.org/
[mosquitto]:                      http://mosquitto.org/
[k8055 for Mac OS X and Linux]:   http://soft.pmad.net/k8055/
[GNU General Public License]:     http://www.gnu.org/licenses/gpl.html
