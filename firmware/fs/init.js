load('api_config.js');
load('api_gpio.js');
load('api_sys.js');
load('api_timer.js');
load('api_mqtt.js');
load('api_adc.js');
load('api_dht.js');
load('stepper.js');
load('api_rpc.js');

// Pins.
let sPin1 = 14, sPin2 = 27, sPin3 = 26, sPin4 = 25;
let pumpPin = 13;
let dhtPin = 32;
let mPin = 33;

// Moisture and temperature.
let mLevels = 4095;
let dht = DHT.create(dhtPin, DHT.DHT11);
ADC.enable(mPin);

// Water Pump.
GPIO.set_mode(pumpPin, GPIO.MODE_OUTPUT);
GPIO.write(pumpPin, false);

// Stepper.
let NUMBER_OF_STEPS_PER_REV = 4096; // half step

let stepper = Object.create(Stepper);
stepper.init(NUMBER_OF_STEPS_PER_REV, sPin1, sPin2, sPin3, sPin4);
stepper.setSpeed(1);

// Plant ID Verification.
let plantID = Cfg.get('app.plant_id');
if (plantID === '') {
	console.log('Error: app.plant_id is empty, update mos.yml');
	return false
}

// MQTT Topics based on Plant ID.
let registerTopic = 'IoTea/register';
let vitalsTopic = 'IoTea/'+plantID +'/vitals';
let triggersTopic = 'IoTea/'+plantID+'/triggers';

// Registers plant identifier on AWS IoT.
function registerPlant(plantID) {
	if (plantID === '') {
		console.log('Error: Failed to register as app.plant_id is empty, update mos.yml');
		return false
	}
	let message = JSON.stringify({
		'plantID': plantID,
	});
	let ok = MQTT.pub(registerTopic, message, 1);
	if (!ok) {
		print('Failed to publish to ', registerTopic)
		return false
	}
	print('To ', registerTopic, ' Published: ', message);
	return true
}

// Sends IoTea plant vitals to the plant's vitals topic.
function sendVitals() {
	let moisturePer = 100.00 - ((ADC.read(mPin) / mLevels) * 100.00);
	let message = JSON.stringify({
		'plantID': plantID,
		'temperature': dht.getTemp(),
		'humidity': dht.getHumidity(),
		'moisturePer': moisturePer,
	});
	let ok = MQTT.pub(vitalsTopic, message, 1);
	if (!ok) {
		return {error: -1, message: 'Error. Failed to publish to '+ vitalsTopic};
	}
	return true
}

// Run water pump.
function pump(val){
	if (typeof(val) === 'number') {
		GPIO.write(pumpPin, true);
		Timer.set(val * 1000, 0, function() {
			GPIO.write(pumpPin, false);
		}, null);
		return true;
	} else {
		return {error: -1, message: 'Bad request. Expected pump value to be a number'};
	}
}

// Turn stepper motor below the plant.
function turn(val){
	if (typeof(val) === 'number') {
		stepper.step(val);
		return true;
	} else {
		return {error: -1, message: 'Bad request. Expected turn value to be a number'};
	}
}

if (!registerPlant(plantID)){
	die('Device failed to register Plant ID to AWS IoT Core')
}

MQTT.sub(triggersTopic, function(conn, topic, trigger){
	trigger = JSON.parse(trigger);
	let output = null;
	if (trigger['category'] === 'vitals'){
		output = sendVitals();
	}
	else if (trigger['category'] === 'pump'){
		output = pump(trigger['value']);
	}
	else if (trigger['category'] === 'turn'){
		output = turn(trigger['value']);
	}

	if (output !== true){
		print(output['message']);
	}
}, null)

