# IoTea Mongoose OS app

## Overview

This program reads the DHT sensor from __pin 32__ and the soil moisture sensor from
__pin 33__.

### DHT Sensor

Contains a digital circuit that provides a digital output for temperature and humidity.

### Soil Moisture Sensor

Provides an analog and digital output but in our case we're only interested in the analog
output.

The analog output for this sensor is 0-4095 where 4095 is no moisture.

### Libraries

Libraries can be added to the `mos.yml` file at:

```
libs:
  ...
  - origin: https://github.com/mongoose-os-libs/dht
```

This program uses:

- `mjs`: Enables the use of javascript files
- `dht`: Enables support for the DHT temperature/humidity sensors
- `adc`: Enables support for analog-to-digital conversion on pins

Full list of libraries: [https://github.com/mongoose-os-libs](https://github.com/mongoose-os-libs/)

## Connecting the hardware

### DHT Sensor

| Sensor Pin | ESP32 Pin |
|------------|-----------|
| +          | 3V3       |
| -          | GND       |
| out        | 32        |

### Soil Moisture Sensor

| Sensor Pin | ESP32 Pin |
|------------|-----------|
| VCC+       | 3V3       |
| GND        | GND       |
| AO         | 33        |

On the breadboard, you can connect the `3V3` pin on the ESP32 to the 
(+) rail and the `GND` pin to the (-) rail and connect each of the 
sensors to those rails for power.
