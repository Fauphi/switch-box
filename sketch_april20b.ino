#include "pitches.h";
#include "images.h";

// LED data sheet
// Weiß: 3,2-3.4V; 
// Grün: 3.0-3.2V; 
// Gelb: 2.0-2.2V; 
// Blau: 3,2-3.4V; 
// Rot: 2-2.2V
// Strom: 20mA

int LED1 = 23; // quad-bottom-left
int LED2 = 25; // quad-bottom-right
int LED3 = 27; // quad-top-right
int LED4 = 29; // quad-top-left
int LED5 = 31; // rgb-red
int LED6 = 33; // rgb-green
int LED7 = 35; // rgb-blue
int LED8 = 37; // single-top
int LED9 = 39; // circle-1
int LED10 = 41; // single-big
int LED11 = 43; // circle-2
int LED12 = 45; // circle-3
int LED13 = 47; // circle-4
int LED14 = 49; // circle-5
int LED15 = 51; // circle-6
int LED16 = 53; // circle-7

const int led_all[] = {LED1,LED2,LED3,LED4,LED5,LED6,LED7,LED8,LED9,LED10,LED11,LED12,LED13,LED14,LED15,LED16};
const int led_all_size = sizeof(led_all) / sizeof(led_all[0]);

const int led_circle[] = {LED9,LED11,LED12,LED13,LED14,LED15,LED16};
const int led_circle_sounds[] = {NOTE_B2, NOTE_C3, NOTE_CS3, NOTE_D3, NOTE_DS3, NOTE_E3, NOTE_F3};
const int led_rgb[] = {LED5,LED6,LED7};
const int led_quad[] = {LED1,LED2,LED3,LED4};

const int LED_ON = HIGH;
const int LED_OFF = LOW;
const int DELAY = 60;

// SWITCHES
int SW1 = 22; // big-left
int SW2 = 24; // big-right
int SW3 = 26; // big-led-blue
int SW4 = 28; // big-led-orange
int SW5 = 30; // rgb-red
int SW6 = 32; // rgb-blue
int SW7 = 34; // on-off
int SW8 = 36; // rgb-green
int SW9 = 38; // black-left
int SW10 = 40; // black-right
int SW11 = 42; // small-middle

const int switches_all[] = {SW1,SW2,SW3,SW4,SW5,SW6,SW7,SW8,SW9,SW10,SW11};
const int switches_all_size = sizeof(switches_all) / sizeof(switches_all[0]);
boolean switch_states[] = {false,false,false,false,false,false,false,false,false,false,false};
boolean switch_sounds[] = {false,false,false,false,false,false,false,false,false,false,false};

// VOLTAGE-METER
int VM = 2;

// SPEAKER
int SPKR = 3;

// POTENTIOMETER
int POTI = A0;

// LED-MATRIX
const int row[8] = {55, 56, 60, 9, 61, 7, 6, 58};
const int col[8] = {62, 57, 11, 59, 4, 10, 5, 8};
int pixels[8][8];

int ANIMATION_FRAME = 1;
const int ANIMATION_INTERVAL = 200;

// Functions
void turn_on_leds();
void turn_off_leds();
boolean switch_state(int sw, boolean prev_state);
void read_switch_states();
void update();
void beep(int pitch);
void refreshScreen();
void play_animation();
void animate_led();

void setup() {
    Serial.begin(9600);
	Serial.println("Sketch:   ");

    // init LEDs
    for(int i=0;i<led_all_size;i++) {
        pinMode(led_all[i], OUTPUT);
    }

    // init SWITCHES
    for(int i=0;i<switches_all_size;i++) {
        pinMode(switches_all[i], INPUT_PULLUP);
    }

    // init VOLTAGE-METER
    pinMode(VM, OUTPUT);
    analogWrite(VM, 0);

    // init SPEAKER
    pinMode(SPKR, OUTPUT);
    analogWrite(SPKR, 0);

    // init POTI
    pinMode(POTI, INPUT);

    // init LED-MATRIX
    for (int thisPin = 0; thisPin < 8; thisPin++) {
        pinMode(col[thisPin], OUTPUT);
        pinMode(row[thisPin], OUTPUT);
        digitalWrite(col[thisPin], LOW);
        digitalWrite(row[thisPin], HIGH);
    }
    // init PIXEL FIELD
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            pixels[x][y] = HIGH;
        }
    }
    // set PIXELs to black
    for(int x=0;x<8;x++) {
        for(int y=0;y<8;y++) {
            if(IMG_BLACK[8 * y + x] == 1) {
                pixels[x][y] = LOW;
            } else {
                pixels[x][y] = HIGH;
            }
        }
    }

    // init led test
    turn_on_leds();
    turn_off_leds();

    // read initial switch states @INFO: does this work?
    read_switch_states();
}

// timing vars
unsigned long animationPreviousMillis = 0;
unsigned long potiPreviousMillis = 0;
unsigned long currentMillis;
int prev_step = 0;

void loop() {
    currentMillis = millis();

    // read poti
    if (currentMillis - potiPreviousMillis >= 100) {
        potiPreviousMillis = currentMillis;
        
        int poti_val = analogRead(POTI) + 6;
        int step_size = 1024 / 7;
        int step = ((poti_val / step_size) - 7) * -1;
        int vm_step_size = 255 / 7;

        if(step != prev_step) {
            Serial.print("Step: ");
            Serial.println(step);

            Serial.print("Poti: ");
            Serial.println(poti_val);
            prev_step = step;

            Serial.println(step*vm_step_size);

            analogWrite(VM, step*vm_step_size);

            tone(SPKR, led_circle_sounds[step-1], 1000 / 16);

            for(int i=1;i<=7;i++) {
                if(i>step) {
                    digitalWrite(led_circle[i-1], LED_OFF);
                } else if(i<=step) {
                    digitalWrite(led_circle[i-1], LED_ON);  
                } 
            }
        }
    }

    // draw LED-MATRIX:
    refreshScreen();

    // read switches
    for(int i=0;i<switches_all_size;i++) {
        boolean new_state = switch_state(switches_all[i], switch_states[i]);
        switch_states[i] = new_state;
    }
    
    // updates LEDs based on switch states
    update();
}

void update() {
    // RGB
    if(switch_states[4]) { // red
        digitalWrite(led_rgb[0], LED_ON);

        for(int x=0;x<8;x++) {
            for(int y=0;y<8;y++) {
                if(IMG_HEART[8 * y + x] == 1) {
                    pixels[x][y] = LOW;
                } else {
                    pixels[x][y] = HIGH;
                }
            }
        }

        if(!switch_sounds[4]) {
            beep(NOTE_C4);
            switch_sounds[4] = true;
        }
    } else {
        digitalWrite(led_rgb[0], LED_OFF);
        switch_sounds[4] = false;
    }

    if(switch_states[7]) { // green
        digitalWrite(led_rgb[1], LED_ON);
        if(!switch_sounds[7]) {
            beep(NOTE_D4);
            switch_sounds[7] = true;
        }
    } else {
        digitalWrite(led_rgb[1], LED_OFF);
        switch_sounds[7] = false;
    }

    if(switch_states[5]) { // blue
        digitalWrite(led_rgb[2], LED_ON);
        if(!switch_sounds[5]) {
            beep(NOTE_E4);
            switch_sounds[5] = true;
        }
    } else {
        digitalWrite(led_rgb[2], LED_OFF);
        switch_sounds[5] = false;
    }

    // red & green
    if(switch_states[4] && switch_states[7]) {

    }

    // green & blue
    if(switch_states[5] && switch_states[7]) {

    }

    // red & blue
    if(switch_states[5] && switch_states[4]) {

    }
    
    // red & green & blue
    if(switch_states[4] && switch_states[5] && switch_states[7]) {

    }

    // BIG SWITCH
    if(switch_states[0]) { // left
        digitalWrite(led_quad[0], LED_ON);
        digitalWrite(led_quad[1], LED_ON);
    } else {
        digitalWrite(led_quad[0], LED_OFF);
        digitalWrite(led_quad[1], LED_OFF);
    }
    if(switch_states[1]) { // right
        digitalWrite(led_quad[2], LED_ON);
        digitalWrite(led_quad[3], LED_ON);
    } else {
        digitalWrite(led_quad[2], LED_OFF);
        digitalWrite(led_quad[3], LED_OFF);
    }

    // SMALL SWITCH
    if(switch_states[10]) {
        digitalWrite(led_all[7], LED_ON);
    } else {
        digitalWrite(led_all[7], LED_OFF);
    }

    // ON-OFF
    if(switch_states[6]) {
        digitalWrite(led_all[9], LED_ON);
        // analogWrite(VM, 255);
        play_animation();
    } else {
        digitalWrite(led_all[9], LED_OFF);
        // analogWrite(VM, 0);
    }
}

void refreshScreen() {
    // iterate over the rows (anodes):
    for (int thisRow = 0; thisRow < 8; thisRow++) {
        // take the row pin (anode) high:
        digitalWrite(col[thisRow], HIGH);
        // iterate over the cols (cathodes):
        for (int thisCol = 0; thisCol < 8; thisCol++) {
            // get the state of the current pixel;
            int thisPixel = pixels[thisRow][thisCol];
            // when the row is HIGH and the col is LOW,
            // the LED where they meet turns on:
            digitalWrite(row[thisCol], thisPixel);
            // turn the pixel off:
            if (thisPixel == LOW) {
                digitalWrite(row[thisCol], HIGH);
            }
        }
        // take the row pin low to turn off the whole row:
        digitalWrite(col[thisRow], LOW);
    }
}

void play_animation() {
    if (currentMillis - animationPreviousMillis >= ANIMATION_INTERVAL) {
        // save the last time you blinked the LED
        animationPreviousMillis = currentMillis;

        animate_led();
    }
}

void animate_led() {
    int newFrame[64];
    memcpy(newFrame, IMG_ANIMATION_1, sizeof newFrame);
    
    if(ANIMATION_FRAME == 1) {
        ANIMATION_FRAME = 2;
    } else if(ANIMATION_FRAME == 2) {
        memcpy(newFrame, IMG_ANIMATION_2, sizeof IMG_ANIMATION_2);
        ANIMATION_FRAME = 3;
    } else if(ANIMATION_FRAME == 3) {
        memcpy(newFrame, IMG_ANIMATION_3, sizeof IMG_ANIMATION_3);
        ANIMATION_FRAME = 4;
    } else if(ANIMATION_FRAME == 4) {
        memcpy(newFrame, IMG_ANIMATION_4, sizeof IMG_ANIMATION_4);
        ANIMATION_FRAME = 1;
    }

    for(int x=0;x<8;x++) {
        for(int y=0;y<8;y++) {
            if(newFrame[8 * y + x] == 1) {
                pixels[x][y] = LOW;
            } else {
                pixels[x][y] = HIGH;
            }
        }
    }
}

void beep(int pitch) {
    if(!pitch) {
        pitch = NOTE_C4;
    }
    tone(SPKR, pitch, 1000 / 16);
}

void read_switch_states() {
    for(int i=0;i<switches_all_size;i++) {
        switch_states[i] = !digitalRead(switches_all[i]);
    }
}

boolean switch_state(int sw, boolean prev_state) {
    boolean state = !digitalRead(sw);  //read the voltage on PIN2
 
    if(state != prev_state){        //check if the button state has changed since last time
        Serial.print("Switch State changed (");
        Serial.print(sw);
        Serial.print("): ");
        Serial.println(state);      //print to serial if it has
        prev_state = state;         //save the button state
    }

    return state;
}

void turn_on_leds() {
    Serial.println(led_all_size);
    for(int i=0;i<led_all_size;i++) {
        digitalWrite(led_all[i], LED_ON);
        delay(DELAY);
    }
}

void turn_off_leds() {
    for(int i=0;i<led_all_size;i++) {
        digitalWrite(led_all[i], LED_OFF);
        delay(DELAY);
    }
}