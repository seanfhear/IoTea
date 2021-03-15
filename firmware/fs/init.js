load('api_config.js');
load('api_gpio.js');
load('api_sys.js');
load('api_timer.js');
load('api_mqtt.js');
load('api_adc.js');
load('api_dht.js');
load('stepper.js');
load('api_rpc.js');
load('api_events.js');
load('api_aws.js');

// Pins
let sPin1 = 14, sPin2 = 27, sPin3 = 26, sPin4 = 25;
let pumpPin = 13;
let dhtPin = 32;
let mPin = 33;

// MQTT Topics
let vitalsTopic = 'iotea/vitals';

// Stepper.
let NUMBER_OF_STEPS_PER_REV = 4096; // half step

let stepper = Object.create(Stepper);
stepper.init(NUMBER_OF_STEPS_PER_REV, sPin1, sPin2, sPin3, sPin4);
stepper.setSpeed(1);

RPC.addHandler('Turn', function(args) {
	if (typeof(args) === 'object' && typeof(args.steps) === 'number') {
		stepper.step(args.steps);
		return true;
	} else {
		return {error: -1, message: 'Bad request. Expected: {"steps": N1}'};
	}
});

// Water Pump.
GPIO.set_mode(pumpPin, GPIO.MODE_OUTPUT);
GPIO.write(pumpPin, false);
RPC.addHandler('Pump', function(args) {
	if (typeof(args) === 'object' && typeof(args.time) === 'number') {
		GPIO.write(pumpPin, true);
		Timer.set(args.time * 1000, 0, function() {
			GPIO.write(pumpPin, false);
		}, null);
		return true;
	} else {
		return {error: -1, message: 'Bad request. Expected: {"time": N1}'};
	}
});

// Moisture and temperature
let mLevels = 4095;
let dht = DHT.create(dhtPin, DHT.DHT11);
ADC.enable(mPin);

if(AWS.isConnected()){
	print("AWS connected")
} else{
	print("AWS not connected")
}

Event.on(Event.CLOUD_CONNECTED, function() {
	print("Connected to the cloud")
}, null);

Event.on(Event.CLOUD_DISCONNECTED, function() {
  print("Not connected to the cloud")
}, null);

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
