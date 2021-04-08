load('api_gpio.js');
load('api_timer.js');

let stepsLeft = 0;
let stepNumber = 0;
let timer = null;
let direction = 1;

let pin1, pin2, pin3, pin4;

let abs = ffi('int abs(int)');

let stepMotor = function(thisStep) {
  if (thisStep === 0) { // 0001
    GPIO.write(pin1, false);
    GPIO.write(pin2, false);
    GPIO.write(pin3, false);
    GPIO.write(pin4, true);
  } else if (thisStep === 1) { // 0011
    GPIO.write(pin1, false);
    GPIO.write(pin2, false);
    GPIO.write(pin3, true);
    GPIO.write(pin4, true);
  } else if (thisStep === 2) { // 0010
    GPIO.write(pin1, false);
    GPIO.write(pin2, false);
    GPIO.write(pin3, true);
    GPIO.write(pin4, false);
  } else if (thisStep === 3) { // 0110
    GPIO.write(pin1, false);
    GPIO.write(pin2, true);
    GPIO.write(pin3, true);
    GPIO.write(pin4, false);
  } else if (thisStep === 4) { // 0100
    GPIO.write(pin1, false);
    GPIO.write(pin2, true);
    GPIO.write(pin3, false);
    GPIO.write(pin4, false);
  } else if (thisStep === 5) { // 1100
    GPIO.write(pin1, true);
    GPIO.write(pin2, true);
    GPIO.write(pin3, false);
    GPIO.write(pin4, false);
  } else if (thisStep === 6) { // 1000
    GPIO.write(pin1, true);
    GPIO.write(pin2, false);
    GPIO.write(pin3, false);
    GPIO.write(pin4, false);
  } else if (thisStep === 7) { // 1001
    GPIO.write(pin1, true);
    GPIO.write(pin2, false);
    GPIO.write(pin3, false);
    GPIO.write(pin4, true);
  }
};

let cutPower = function() {
  GPIO.write(pin1, false);
  GPIO.write(pin2, false);
  GPIO.write(pin3, false);
  GPIO.write(pin4, false);
};

let Stepper = {
  init: function(steps, p1, p2, p3, p4) {
    this.number_of_steps = steps;
    pin1 = p1;
    pin2 = p2;
    pin3 = p3;
    pin4 = p4;

    GPIO.set_mode(pin1, GPIO.MODE_OUTPUT);
    GPIO.set_mode(pin2, GPIO.MODE_OUTPUT);
    GPIO.set_mode(pin3, GPIO.MODE_OUTPUT);
    GPIO.set_mode(pin4, GPIO.MODE_OUTPUT);
  },
  setSpeed: function(speed) {
    this.step_delay = 60 * 1000 / this.number_of_steps / speed;
  },
  step: function(stepsToMove) {
    stepsLeft = abs(stepsToMove);

    // Determine the direction
    direction = 1;
    if (stepsToMove < 0) {
      direction = 0;
    }

    if (direction === 1) {
      stepNumber = 0;
    } else if (direction === 0) {
      stepNumber = stepsLeft;
    }

    stepMotor(stepNumber % 4);

    timer = Timer.set(this.step_delay, Timer.REPEAT, function() {
      if (stepsLeft > 0) {
        // Step the motor.
        if (direction === 1) {
          stepNumber++;
        } else if (direction === 0) {
          stepNumber--;
        }
        stepMotor(stepNumber % 8);
        stepsLeft--;
      } else {
        stepNumber = 0;
        Timer.del(timer);
        cutPower();
      }
    }, null);
  },
};
