load('api_config.js');
load('api_timer.js');
load('api_sys.js');
load('api_gpio.js');
load('api_dht.js');
load('api_adc.js');
load('stepper.js');

let NUMBER_OF_STEPS_PER_REV = 2038;

let stepper = Object.create(Stepper);

stepper.init(NUMBER_OF_STEPS_PER_REV, 14, 27, 26, 25);
stepper.setSpeed(1);
stepper.step(NUMBER_OF_STEPS_PER_REV);

// Moisture and temperature.
let mLevels = 4095;

// Pins
let dhtPin = 32;
let mPin = 33;

// Enable pins
let dht = DHT.create(dhtPin, DHT.DHT11);
ADC.enable(mPin);

Timer.set(10000, true, function() {
	let moisturePer = 100.00 - ((ADC.read(mPin) / mLevels) * 100.00);
	print('Temperature: ', dht.getTemp(), ' Humidity: ', dht.getHumidity(), ' Moisture: ', moisturePer);
}, null);
