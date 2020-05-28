#include "DHT.h"
#define DHTPIN 4    // modify to the pin we connected
#define DHTTYPE DHT21   // AM2301 
#define ESPSIGNAL 2
#define RED 3
#define GREEN 5
#define BLUE 6
#define BUZZ 7
#define FORWARD 9
#define REVERSE 10
#define STOP 0
#define STRAIGHT 0
#define RIGHT 11
#define LEFT 12
#define LIGHT A0
#define BATTERY A1
#define SOUND A2
#define GAS A4
#define MAINTAIN 100
#define REFRESH_MEASURE_TIME 10000 //not ms

#define Stop 1
#define Forward 2
#define Reverse 3
#define Straight 4
#define Right 5
#define Left 6
#define BuzzOn 7
#define BuzzOff 8
#define TakeMeasure 9
#define Run 10

#define Lon 0
#define Lbat 1
#define Lgas 2
#define Lmeasuring 3
#define Ltemphum 4
#define Lready 5
#define Lrunning 6
#define Loff 7

#define Low 0
#define Dead 1

#define Bbeep 0
#define Bready 1
#define Bdanger 2
#define Boff 3

DHT dht(DHTPIN, DHTTYPE);


byte light;
byte sound;
byte gas;
byte humidity;
byte temperature;
byte battery;
byte red;
byte green;
byte blue;
byte buzz;
int data;
unsigned long currentTime;
unsigned long previousTime = 0;
byte ledMeasureState = 0;
const byte matrixLength = 20;
byte matrix_temp[matrixLength];
byte matrix_hum[matrixLength];
byte matrix_sound[matrixLength];
byte matrix_light[matrixLength];
byte matrix_gas[matrixLength];
byte readyState;


void measureAll();
void printAll();
void drive(byte direct); //FORWARD, REVERSE, STOP
void turn(byte direct); //RIGHT, LEFT, STRAIGHT
void led(byte state);
void BMS(byte state);
void zeroMatrix();
byte updateMatrix(byte init);
void measureBattery();
void buzzer(byte state);



void setup()
{
  TCCR2B = TCCR2B & B11111000 | B00000001; // for PWM frequency of 31372.55 Hz
  TCCR0B = TCCR0B & B11111000 | B00000010; // for PWM frequency of 7812.50 Hz
  TCCR1B = TCCR1B & B11111000 | B00000001; // set timer 1 divisor to 1 for PWM frequency of 31372.55 Hz
  pinMode(ESPSIGNAL, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  pinMode(FORWARD, OUTPUT);
  pinMode(REVERSE, OUTPUT);
  pinMode(RIGHT, OUTPUT);
  pinMode(LEFT, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(BLUE, HIGH);
  digitalWrite(BUZZ, HIGH);

  Serial.begin(115200);
  dht.begin();
  zeroMatrix();

}


void loop()
{
  if (Serial.available()) {
    data = (int)(Serial.read());
    switch (data) {
      case Stop:
        drive(STOP);
        led(Lon);
        break;
      case Forward:
        drive(FORWARD);
        led(Lrunning);
        break;
      case Reverse:
        drive(REVERSE);
        led(Lrunning);
        break;
      case Straight:
        turn(STRAIGHT);
        break;
      case Right:
        turn(RIGHT);
        break;
      case Left:
        turn(LEFT);
        break;
      case BuzzOff:
        buzzer(Boff);
        break;
      case BuzzOn:
        buzzer(Bbeep);
        break;
      case TakeMeasure:
        Serial.write(temperature);
        Serial.write(humidity);
        Serial.write(light);
        Serial.write(sound);
        Serial.write(gas);
        Serial.write(battery);
        if (ledMeasureState == 0) {
          led(Lmeasuring);
        }
        break;
      case Run:
        ledMeasureState = 0;
        updateMatrix(1);
        led(Lon);
        readyState = 0;
        break;
      case -1:
        break;
      default:
        turn(STRAIGHT);
        drive(STOP);
        digitalWrite(BUZZ, HIGH);
    }
  }
  else if (currentTime - previousTime > REFRESH_MEASURE_TIME) {
    previousTime = currentTime;
    measureBattery();
    if (battery == 0) {
      BMS(Dead);
    }
    else if (battery < 50) {
      BMS(Low);
    }
    if (data == TakeMeasure) {
      measureAll();
      if (readyState == 0) {
        readyState = updateMatrix(0);
        digitalWrite(ESPSIGNAL, LOW);
        led(Lmeasuring);
      }
      else if (readyState == 1) {
        readyState = 2;
        updateMatrix(0);
        digitalWrite(ESPSIGNAL, HIGH);
        ledMeasureState = 1;
        if (gas > 51) { //20%
          led(Lgas);
          buzzer(Bdanger);
        }
        else if (temperature > 125 || temperature < 75 || humidity > 204 || humidity < 51) { //15-25 βαθμοί, 20-80% υγρασία
          led(Ltemphum);
          buzzer(Bready);
        }
        else {
          led(Lready);
          buzzer(Bready);
        }
      }
      else if (readyState == 2) {
        updateMatrix(0);
      }
    }
  }
  turn(MAINTAIN);
  led(MAINTAIN);
  buzzer(MAINTAIN);
}


void measureAll() {
  int s;
  int sp = 1024;
  float h;
  float t;
  unsigned long previousTime = millis();

  currentTime = millis();
  while (millis() < previousTime + 2000) {
    led(MAINTAIN);
    buzzer(MAINTAIN);
    s = analogRead(SOUND);
    if (s < sp) {
      sp = s;
    }
  }
  sp -= 425; //425-680
  if (sp < 0) {
    sp = 0;
  }
  else if (sp > 255) {
    sp = 255;
  }
  sound = sp;
  gas = analogRead(GAS) / 4;
  light = analogRead(LIGHT) / 4;
  h = dht.readHumidity();
  led(MAINTAIN);
  buzzer(MAINTAIN);
  t = dht.readTemperature();
  h *= 2.55;
  humidity = h;
  if (t < 0) {
    t = 0;
  }
  else if (t > 50) {
    t = 50;
  }
  t *= 5;
  temperature = t;
}


void measureBattery() {
  float b;
  static float prev_b = 255;
  b = analogRead(BATTERY);
  b -= 310;
  b *= 2.55;
  if (b < 0) {
    b = 0;
  }
  else if (b > 255) {
    b = 255;
  }
  if (b < prev_b) {
    prev_b = b;
  }
  battery = (byte)(prev_b);
}


void printAll() {
  Serial.print("Sound is: ");
  Serial.print(sound);
  Serial.print(", Gas is: ");
  Serial.print(gas);
  Serial.print(", Light is: ");
  Serial.print(light);
  Serial.print(", Humidity is: ");
  Serial.print(humidity);
  Serial.print(", Temperature is: ");
  Serial.println(temperature);
}


void drive(byte direct)
{
  static byte speeds = 150;
  switch (direct) {
    case FORWARD:
      digitalWrite(REVERSE, LOW);
      analogWrite(FORWARD, speeds);
      break;
    case REVERSE:
      digitalWrite(FORWARD, LOW);
      analogWrite(REVERSE, speeds);
      break;
    case STOP:
      digitalWrite(FORWARD, LOW);
      digitalWrite(REVERSE, LOW);
      break;
  }
}


void turn(byte direct)
{
  static unsigned long previousTime = 0;
  static const byte strong = 220;
  static const byte weak = 100;
  static byte flag = 0;
  static int TimePass = 5000;
  static const byte res = 100;
  static byte LUT[res];
  static byte start = 1;
  static byte j = 0;

  currentTime = millis();
  if (start) {
    start = 0;
    for (byte i = 0; i < res; i++) {
      LUT[i] = weak + (strong - weak) * exp(-5 * (float)(i) / res);
    }
  }
  switch (direct) {
    case RIGHT:
      previousTime = currentTime;
      flag = RIGHT;
      digitalWrite(LEFT, LOW);
      analogWrite(RIGHT, strong);
      j = 0;
      break;
    case LEFT:
      previousTime = currentTime;
      flag = LEFT;
      digitalWrite(LEFT, HIGH);
      analogWrite(RIGHT, 255 - strong);
      j = 0;
      break;
    case STRAIGHT:
      flag = STRAIGHT;
      digitalWrite(RIGHT, LOW);
      digitalWrite(LEFT, LOW);
      break;
    case MAINTAIN:
      if (flag == LEFT || flag == RIGHT) {
        if (currentTime - previousTime >= TimePass / res) {
          previousTime = currentTime;
          if (j != res) {
            if (flag == RIGHT) {
              analogWrite(RIGHT, LUT[j]);
            }
            else if (flag == LEFT) {
              analogWrite(RIGHT, 255 - LUT[j]);
            }
            j++;
          }
        }
      }
      break;
  }
}

void led(byte state) {
  static unsigned long previousTime = 0;
  static const byte on = 150;
  static const byte off = 255;
  static int inc = -1;
  static byte i = 0;
  static byte r = off;
  static byte g = off;
  static byte b = off;
  static int t;
  static byte flag = 0;

  //  Demonstrate
  //  static unsigned long pv = 0;
  //  if (currentTime - pv > 50000) {
  //    pv = currentTime;
  //    i++;
  //    if (i == 8) {
  //      i = 0;
  //    }
  //  }

  currentTime = millis();
  if (state != MAINTAIN) {
    i = state;
  }

  switch (i) {
    case Lon: //ON
      analogWrite(RED, off);
      analogWrite(BLUE, off);
      t = 80;
      if (currentTime - previousTime > t) {
        previousTime = currentTime;
        if (g >= 255) {
          inc = -1;
        }
        else if (g <= 150) {
          inc = 1;
        }
        g += inc;
        analogWrite(GREEN, g);
      }
      break;
    case Lbat: //LOW BATTERY
      analogWrite(GREEN, off);
      analogWrite(BLUE, off);
      if (currentTime - previousTime > t) {
        previousTime = currentTime;
        if (t > 600) {
          analogWrite(RED, on);
          t = 500;
        }
        else {
          analogWrite(RED, off);
          t = 10000;
        }
      }
      break;
    case Lgas: //GAS
      t = 3000;
      analogWrite(GREEN, off);
      if (currentTime - previousTime > t) {
        previousTime = currentTime;
        if (flag) {
          analogWrite(RED, on);
          analogWrite(BLUE, off);
          flag = 0;
        }
        else {
          analogWrite(RED, off);
          analogWrite(BLUE, on);
          flag = 1;
        }
      }
      break;
    case Lmeasuring: //MEASURING
      analogWrite(GREEN, off);
      t = 80;
      if (currentTime - previousTime > t) {
        previousTime = currentTime;
        if (b >= 255) {
          inc = -1;
        }
        else if (b <= 150) {
          inc = 1;
        }
        b += inc;
        r = 255 + 150 - b;
        analogWrite(RED, r);
        analogWrite(BLUE, b);
      }
      break;
    case Ltemphum: //TEMPERATURE/HUMIDITY
      analogWrite(GREEN, on);
      analogWrite(BLUE, off);
      t = 10;
      if (currentTime - previousTime > t) {
        previousTime = currentTime;
        if (r >= 255) {
          inc = -1;
        }
        else if (r <= 0) {
          inc = 1;
        }
        r += inc;
        analogWrite(RED, r);
      }
      break;
    case Lready: //READY
      analogWrite(RED, off);
      t = 40;
      if (currentTime - previousTime > t) {
        previousTime = currentTime;
        if (g >= 255) {
          inc = -1;
        }
        else if (g <= 150) {
          inc = 1;
        }
        g += inc;
        b = 255 + 300 - 2 * g;
        analogWrite(BLUE, b);
        analogWrite(GREEN, g);
      }
      break;
    case Lrunning: //RUNNING
      analogWrite(BLUE, off);
      t = 30;
      if (currentTime - previousTime > t) {
        previousTime = currentTime;
        if (g >= 255) {
          inc = -1;
        }
        else if (g <= 150) {
          inc = 1;
        }
        g += inc;
        r = 255 + 300 - 2 * g;
        analogWrite(RED, r);
        analogWrite(GREEN, g);
      }
      break;
    case Loff: //OFF
      analogWrite(RED, off);
      analogWrite(GREEN, off);
      analogWrite(BLUE, off);
      break;
  }
}


void buzzer(byte state) {
  static unsigned long previousTime = 0;
  static byte flag = 0;
  static byte i = Loff;
  static byte counter = 0;
  static byte ch = 0;

  currentTime = millis();
  if (state != MAINTAIN) {
    i = state;
    flag = 0;
    counter = 0;
    ch = 0;
    previousTime = currentTime;
    digitalWrite(BUZZ, HIGH);
  }
  switch (i) {
    case Bbeep:
      digitalWrite(BUZZ, LOW);
      break;
    case Bready:
      if (currentTime - previousTime > 500 && flag < 6) {
        previousTime = currentTime;
        if (ch == 0) {
          digitalWrite(BUZZ, LOW);
          ch = 1;
        }
        else {
          digitalWrite(BUZZ, HIGH);
          ch = 0;
        }
        flag++;
      }
      break;
    case Bdanger:
      if (currentTime - previousTime > 3000 && flag < 6) {
        previousTime = currentTime;
        if (ch == 0) {
          digitalWrite(BUZZ, LOW);
          ch = 1;
        }
        else {
          digitalWrite(BUZZ, HIGH);
          ch = 0;
        }
        flag++;
      }
      break;
    case Boff:
      digitalWrite(BUZZ, HIGH);
      break;
  }
}


void BMS(byte state) {
  if (state == Low) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (state == Dead) {
    digitalWrite(FORWARD, LOW);
    digitalWrite(REVERSE, LOW);
    digitalWrite(RIGHT, LOW);
    digitalWrite(LEFT, LOW);
    digitalWrite(BUZZ, HIGH);
    digitalWrite(ESPSIGNAL, LOW);
    digitalWrite(LED_BUILTIN, HIGH);
    led(Lbat);
    while (1) {
      if (Serial.available()) {
        data = (int)(Serial.read());
        if (data == TakeMeasure) {
          Serial.write(temperature);
          Serial.write(humidity);
          Serial.write(light);
          Serial.write(sound);
          Serial.write(gas);
          Serial.write(0);
        }
      }
      led(MAINTAIN);
    }
  }
}


void zeroMatrix() {
  for (byte i = 0; i < matrixLength; i++) {
    matrix_temp[i] = 0;
    matrix_hum[i] = 0;
    matrix_sound[i] = 0;
    matrix_light[i] = 0;
    matrix_gas[i] = 0;
  }
}


byte updateMatrix(byte init) {
  static byte i = 0;
  static byte flag = 0;
  static byte state = 0;
  int diffTemp;
  int diffHum;
  if (init) {
    i = 0;
    flag = 0;
    state = 0;
    return (0);
  }
  matrix_temp[i] = temperature;
  matrix_hum[i] = humidity;
  matrix_sound[i] = sound;
  matrix_light[i] = light;
  matrix_gas[i] = gas;
  i++;
  if (i >= matrixLength) {
    i = 0;
    flag = 1;
  }
  if (i == matrixLength - 1) {
    diffTemp = (int)(matrix_temp[i]) - (int)(matrix_temp[0]);
    diffHum = (int)(matrix_hum[i]) - (int)(matrix_hum[0]);
  }
  else {
    diffTemp = (int)(matrix_temp[i]) - (int)(matrix_temp[i + 1]);
    diffHum = (int)(matrix_hum[i]) - (int)(matrix_hum[i + 1]);
  }
  diffTemp = abs(diffTemp);
  diffHum = abs(diffHum);
  if (diffTemp < 2 && diffHum < 2 && flag == 1) {
    state = 1;
  }
  return (state);
}
