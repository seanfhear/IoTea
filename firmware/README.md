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

### Stepper Motor

The stepper connects to a driver that allows for a microcontroller to send a high or low signal to each of the four stators to control it.

### Water Pump

The water pump cannot be directly controlled by a microcontroller as it only has a power input. To control it, we have a relay that can be controlled by a microcontroller.

### Libraries

Libraries can be added to the `mos.yml` file at:

```
libs:
  ...
  - origin: https://github.com/mongoose-os-libs/dht
```

This program uses:

- `aws`: Enables Amazon AWS IoT integration with Mongoose OS
- `mjs`: Enables the use of javascript files
- `mqtt`: Enables MQTT protocol support with Mongoose OS
- `dht`: Enables support for the DHT temperature/humidity sensors
- `adc`: Enables support for analog-to-digital conversion on pins

Full list of libraries: [https://github.com/mongoose-os-libs](https://github.com/mongoose-os-libs/)

## Connecting the hardware

### Breadboard

Connect the provided power supply to the breadboard power rails and ensure that the 5V and VCC pins are selected to provide 5V to the rail.

### ESP32

Connect the __GND__ pin to the (-) rail on the breadboard.

### DHT Sensor

| Sensor Pin | ESP32 Pin | Breadboard |
|------------|-----------|------------|
| +          | -         | (+) rail   |
| -          | -         | (-) rail   |
| out        | 32        | -          |

### Soil Moisture Sensor

| Sensor Pin | ESP32 Pin | Breadboard |
|------------|-----------|------------|
| VCC+       | -         | (+) rail   |
| GND        | -         | (-) rail   |
| A0         | 33        | -          |

### Stepper Motor

| Driver Pin | ESP32 Pin | Breadboard |
|------------|-----------|------------|
| IN1        | 14        | -          |
| IN2        | 27        | -          |
| IN3        | 26        | -          |
| IN4        | 25        | -          |
| (+)        | -         | (+) rail   |
| (-)        | -         | (-) rail   |

### Water Pump

| Relay Pin | ESP32 Pin | Breadboard | Water Pump |
|-----------|-----------|------------|------------|
| DC+       | -         | (+) rail   | -          |
| DC-       | -         | (-) rail   | -          |
| IN        | 13        | -          | -          |
| NO        | -         | -          | negative (black) |
| COM       | -         | (+) rail   | (-) rail on breadboard to positive (red) |

## AWS CLI Setup

1. Install [AWS CLI](https://docs.aws.amazon.com/cli/latest/userguide/install-cliv2.html)

2. Configure CLI with appropriate provided credentials:
```bash
aws configure
```

3. Verify correct setup being able to run:
```bash
aws iot list-things
```

## Device Setup

1. Set your Plant ID

Edit mos.yml with your plant ID.

2. Build:
```bash
mos build --platform esp32
```

3. Flash:
```bash
mos flash
```

4. Wifi Setup:
```bash
mos wifi <WIFI-SSID> <WIFI-PASSWORD>
```

5. AWS IoT Setup:
```bash
mos aws-iot-setup
```