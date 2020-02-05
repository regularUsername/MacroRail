#include <Arduino.h>
#include <AccelStepper.h>
// #include <ClickEncoder.h>
// #include <Adafruit_PCD8544.h>

#include <Config.h>


#define SHUTTER_PIN 13
#define STEPPER_PIN1 2
#define STEPPER_PIN2 3
#define STEPPER_PIN3 4
#define STEPPER_PIN4 5


// Adafruit_PCD8544 display = Adafruit_PCD8544(5,4,3);


AccelStepper stepper(AccelStepper::FULL4WIRE,STEPPER_PIN1,STEPPER_PIN2,STEPPER_PIN3,STEPPER_PIN4);
// ClickEncoder encoder(A1,A0,A2);



void takePhoto(int exposure_time) {
    delay(200);
    digitalWrite(SHUTTER_PIN, HIGH);
    delay(200);
    digitalWrite(SHUTTER_PIN, LOW);
    delay(exposure_time);
}

enum State {
    READY,
    BRACKETING,
    HOMING,
    PREVIEW,
};

int targetPos;
int i;
State state;
Config config;

void setup() {
    config = {};
    config.stepsPerMM = 183; // 183 bei fullstepping auf novoflex
    config.totalDistance = 5*config.stepsPerMM;
    config.interval = 0.5*config.stepsPerMM; 
    config.backwards = false;
    config.exposureTime = 1000;
    if (config.backwards){
        config.interval = -config.interval;
    }

    targetPos = 0;
    i = 0;
    state = READY;

    Serial.begin(9600);
    Serial.setTimeout(5000);

    stepper.setMaxSpeed(500);
    stepper.setAcceleration(200);

    pinMode(SHUTTER_PIN,OUTPUT);
}

int parseConfigString(String *str){
    String subs = str->substring(str->lastIndexOf("=")+1);
    // Serial.print("substring: "); Serial.println(subs);
    return subs.toInt();
}
float parseConfigStringF32(String *str){
    String subs = str->substring(str->lastIndexOf("=")+1);
    // Serial.print("substring: "); Serial.println(subs);
    return subs.toFloat();
}

void loop() {
    switch (state) {
    case READY:
        if (Serial.available()>0){
            String str = Serial.readStringUntil('\n');
            // Serial.println(str);
            if(str.startsWith("start")){
                state = BRACKETING;
                targetPos = 0;
                if (!config.backwards){
                    config.interval = abs(config.interval);
                } else {
                    config.interval = -abs(config.interval);
                }
                stepper.moveTo(targetPos);
                delay(2000);
            } else if (str.startsWith("preview")) {
                state = PREVIEW;
                stepper.moveTo(!config.backwards ? config.totalDistance: -config.totalDistance); //-totalDistance für rückwärts
            } else if (str.startsWith("dist")){
                int x = parseConfigString(&str);
                config.totalDistance = x*config.stepsPerMM;
                Serial.print("Setting totalDistance to "); Serial.print(x); Serial.println("mm");
            } else if (str.startsWith("interval")){
                float x =parseConfigStringF32(&str);
                config.interval = x*config.stepsPerMM;
                Serial.print("Setting interval to ");
                Serial.print(x);
                Serial.print("mm (");
                Serial.print(config.interval);
                Serial.println(" steps)");
            } else if (str.startsWith("dir")){
                if (parseConfigString(&str)>0){
                    config.backwards = false;
                    Serial.println("Setting direction to forwards");
                } else {
                    config.backwards = true;
                    Serial.println("Setting direction to backwards");
                }
            } else if (str.startsWith("exp")) {
                config.exposureTime = parseConfigString(&str);
                Serial.print("Setting exposure time to "); Serial.print(config.exposureTime); Serial.println("ms");
            } else if(str.startsWith("print")){
                Serial.print("distance="); Serial.print(config.totalDistance/config.stepsPerMM); Serial.println("mm");
                Serial.print("interval="); Serial.print((float)abs(config.interval)/(float)config.stepsPerMM); Serial.println("mm");
                Serial.print("exposure="); Serial.print(config.exposureTime); Serial.println("ms");
                Serial.println(!config.backwards ? "direction=fowards":"Direction=backwards");
                Serial.print(config.totalDistance/config.interval+2); Serial.println(" photos will be taken");
            } else if (str.startsWith("mov")){
                float dist = parseConfigStringF32(&str);
                Serial.print("Moving "); Serial.print(dist); Serial.println("mm");
                stepper.setCurrentPosition(-(dist*config.stepsPerMM));
                state=HOMING;
                stepper.moveTo(0);
            } else if (str.startsWith("speed")){
                stepper.setMaxSpeed(parseConfigString(&str));
                Serial.print("Setting speed to "); Serial.println(stepper.maxSpeed());
            } else if (str.startsWith("accel")){
                int accel = parseConfigString(&str);
                stepper.setAcceleration(accel);
                Serial.print("Setting acceleration to "); Serial.println(accel);
            }
            
        }
        break;
    case BRACKETING:
        stepper.run();
        if (stepper.currentPosition() == targetPos) {
            i++;
            Serial.print("taking picture "); Serial.print(i); Serial.print(" of "); Serial.println(config.totalDistance/config.interval+2);
            takePhoto(config.exposureTime);
            if (abs(targetPos)>config.totalDistance){
                state = HOMING;
                targetPos = 0;
                i = 0;
            } else {
                targetPos+=config.interval;
            }
            stepper.moveTo(targetPos);
        }
        break;
    case PREVIEW:
        stepper.run();
        if(abs(stepper.currentPosition())>=config.totalDistance){
            state = HOMING;
            stepper.moveTo(0);
        }

        break;
    case HOMING:
        stepper.run();
        if(stepper.currentPosition() == 0){
            state = READY;
            Serial.println("action complete");
            stepper.disableOutputs();
        }
        break;
    }
}