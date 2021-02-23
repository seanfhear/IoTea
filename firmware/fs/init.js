load('api_config.js');
load('api_timer.js');
load('api_sys.js');
load('api_gpio.js');
load('api_dht.js');
load('api_adc.js');

let mLevels = 4095;

// Pins
let dhtPin = 32;
let mPin = 33;

// Enable pins
let dht = DHT.create(dhtPin, DHT.DHT11);
ADC.enable(mhPin);

Timer.set(1000, true, function() {
	let moisturePer = 100.00 - ((ADC.read(mPin) / mLevels) * 100.00);
	print('Temperature: ', dht.getTemp(), ' Humidity: ', dht.getHumidity(), ' Moisture: ', moisturePer);
}, null);

