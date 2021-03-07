load('api_config.js');
load('api_gpio.js');
load('api_sys.js');
load('api_timer.js');
load('api_mqtt.js');
load('api_adc.js');
load('api_dht.js');
load('stepper.js');

// MQTT Topics
let vitalsTopic = 'iotea/vitals';

let NUMBER_OF_STEPS_PER_REV = 4096; // half step

let stepper = Object.create(Stepper);

stepper.init(NUMBER_OF_STEPS_PER_REV, 14, 27, 26, 25);
stepper.setSpeed(1);
stepper.step(NUMBER_OF_STEPS_PER_REV);

// Moisture and temperature
let mLevels = 4095;

// Pins
let dhtPin = 32;
let mPin = 33;

// Enable pins
let dht = DHT.create(dhtPin, DHT.DHT11);
ADC.enable(mPin);

Timer.set(10000, true, function() {
	let moisturePer = 100.00 - ((ADC.read(mPin) / mLevels) * 100.00);
	let message = JSON.stringify({ 
		'deviceID': Cfg.get('device.id'),
		'temperature': dht.getTemp(),
		'humidity' : dht.getHumidity(),
		'moisturePer' : moisturePer,
	});
	let ok = MQTT.pub(vitalsTopic, message, 1);
	if (!ok) print('Failed to publish to ', vitalsTopic)
	else print('Published: ', message);
}, null);
