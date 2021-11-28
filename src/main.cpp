
#include <Arduino.h>
#include <CheapStepper.h>

#include <Ds1302.h>

// DS1302 RTC instance

#define pausePin 37
#define thirdIncrease 38
#define secondIncrease 39
#define firstIncrease 40
#define ledPin 41

#define clockCLK 32
#define clockDAT 33
#define clockRST 34
Ds1302::DateTime dateTime;

const int numberCounterPin[3] = {18, 2, 20};
const int zeroCounterPin[3] = {19, 3, 21};

boolean zeroPosition[3] = {false, false, false};
int numberDisplayed = 0;
bool pauseState = false;

byte displayedNumber[3] = {0, 0, 0};

Ds1302 rtc(clockRST, clockCLK, clockDAT);
unsigned long timerCounter = 0;
unsigned long timerPause = 0;
long timerInterval = 72000; // 72000 = 72 secs

CheapStepper stepper[3]{{46, 47, 48, 49}, {42, 43, 44, 45}, {50, 51, 52, 53}};

// let's create a boolean variable to save the direction of our rotation
boolean moveClockwise = false;
boolean numberChanged[3] = {false, false, false};

void zeroCount(int digit);
void countNumber(int digit);
void gotoZeroPosition(int digit);
void increaseNumber(int digit);
void pauseCounter();

const int numberThreshold = 160; //Gap between hall sensor and number position

const int numberThresh[3] = {180, 150, 180}; //Gap between hall sensor and number position

int secondDigit = 0;
int thirdDigit = 0;

void zeroCount0()
{
  zeroCount(0);
}
void zeroCount1()
{
  zeroCount(1);
}
void zeroCount2()
{
  zeroCount(2);
}

void countNumber0()
{
  countNumber(0);
}

void countNumber1()
{
  countNumber(1);
}

void countNumber2()
{
  countNumber(2);
}

void setup()
{
  Serial.begin(115200);

  rtc.init();
  // dateTime.year = 21;
  // dateTime.month = 11;
  // dateTime.day = 24;
  // dateTime.hour = 21;
  // dateTime.minute = 11;
  // dateTime.second = 0;
  // rtc.setDateTime(&dateTime);
  rtc.getDateTime(&dateTime);
  Serial.print(dateTime.year);
  Serial.print("/");
  Serial.print(dateTime.month);
  Serial.print("/");
  Serial.print(dateTime.day);
  Serial.print(" - ");
  Serial.print(dateTime.hour);
  Serial.print(":");
  Serial.print(dateTime.minute);
  Serial.print(":");
  Serial.print(dateTime.second);
  Serial.println();

  pinMode(pausePin, INPUT_PULLUP);
  pinMode(thirdIncrease, INPUT_PULLUP);
  pinMode(secondIncrease, INPUT_PULLUP);
  pinMode(firstIncrease, INPUT_PULLUP);

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  for (int i = 0; i < 3; i++)
  {
    stepper[i].setRpm(10);
    pinMode(zeroCounterPin[i], INPUT_PULLUP);
    pinMode(numberCounterPin[i], INPUT_PULLUP);
  }
  attachInterrupt(digitalPinToInterrupt(zeroCounterPin[0]), zeroCount0, RISING);
  attachInterrupt(digitalPinToInterrupt(numberCounterPin[0]), countNumber0, CHANGE);
  attachInterrupt(digitalPinToInterrupt(zeroCounterPin[1]), zeroCount1, RISING);
  attachInterrupt(digitalPinToInterrupt(numberCounterPin[1]), countNumber1, CHANGE);
  attachInterrupt(digitalPinToInterrupt(zeroCounterPin[2]), zeroCount2, RISING);
  attachInterrupt(digitalPinToInterrupt(numberCounterPin[2]), countNumber2, CHANGE);
  /* Note: CheapStepper library assumes you are powering your 28BYJ-48 stepper
   * using an external 5V power supply (>100mA) for RPM calculations
   * -- don't try to power the stepper directly from the Arduino
   * 
   * accepted RPM range: 6RPM (may overheat) - 24RPM (may skip)
   * ideal range: 10RPM (safe, high torque) - 22RPM (fast, low torque)
   */

  // Serial.print(stepper[0].getRpm()); // get the RPM of the stepper
  // Serial.print(" rpm = delay of ");
  // Serial.print(stepper[0].getDelay()); // get delay between steps for set RPM
  // Serial.print(" microseconds between steps");
  // Serial.println();
  zeroPosition[2] = false;
  zeroPosition[1] = false;
  zeroPosition[0] = false;

  gotoZeroPosition(0);
  numberChanged[0] = false;
  gotoZeroPosition(1);
  numberChanged[1] = false;
  gotoZeroPosition(2);
  numberChanged[2] = false;
  Serial.print("Displayed Number = 000 @ ");
  rtc.getDateTime(&dateTime);
  Serial.print(dateTime.year);
  Serial.print("/");
  Serial.print(dateTime.month);
  Serial.print("/");
  Serial.print(dateTime.day);
  Serial.print(" - ");
  Serial.print(dateTime.hour);
  Serial.print(":");
  Serial.print(dateTime.minute);
  Serial.print(":");
  Serial.print(dateTime.second);
  Serial.println();
  timerCounter = millis();
  for (int i = 0; i < 3; i++)
    stepper[i].off();
}

void loop()
{

  if (digitalRead(pausePin) == HIGH && (millis() >= timerPause + 1000))
  {
    pauseState = true;
    timerPause = millis();
  }

  if (pauseState)
    pauseCounter();

  if (millis() - timerCounter > timerInterval)
  {

    increaseNumber(0);

    numberDisplayed++;
    uint8_t third, second, first;
    third = numberDisplayed / 100;
    second = (numberDisplayed - third * 100) / 10;
    first = numberDisplayed % 10;
    Serial.print("Displayed Number = ");
    Serial.print(third);
    Serial.print("-");
    Serial.print(second);
    Serial.print("-");
    Serial.print(first);
    Serial.print(" @ ");
    rtc.getDateTime(&dateTime);
    Serial.print(dateTime.year);
    Serial.print("/");
    Serial.print(dateTime.month);
    Serial.print("/");
    Serial.print(dateTime.day);
    Serial.print(" - ");
    Serial.print(dateTime.hour);
    Serial.print(":");
    Serial.print(dateTime.minute);
    Serial.print(":");
    Serial.print(dateTime.second);
    Serial.println();

    if ((numberDisplayed % 100) / 10 > displayedNumber[1])
      increaseNumber(1);

    if (numberDisplayed / 100 > displayedNumber[2])
    {
      increaseNumber(1);
      increaseNumber(2);
    }
    if (numberDisplayed == 400)
    {
      pauseState = true;
      pauseCounter();
    }
    Serial.println("Stepper off");
    timerCounter = millis();
    for (int i = 0; i < 3; i++)
      stepper[i].off();
  }
}

void increaseNumber(int digit)
{
  // Serial.print("increase for ");
  // Serial.println(digit);
  while (!numberChanged[digit])
  {
    stepper[digit].move(moveClockwise, 1);
  }
  Serial.print("Move threshold = ");
  Serial.println(numberThresh[digit]);
  stepper[digit].move(moveClockwise, numberThresh[digit]);
  numberChanged[digit] = false;
}

void pauseCounter()
{
  digitalWrite(ledPin, HIGH);
  Serial.println("pause counter");
  for (int i = 0; i < 3; i++)
    stepper[i].off();

  while (pauseState)
  {
    if (digitalRead(pausePin) == HIGH && (millis() >= timerPause + 1000))
    {
      pauseState = false;
      timerPause = millis();
    }

    if (digitalRead(firstIncrease) == HIGH)
      increaseNumber(0);

    if (digitalRead(secondIncrease) == HIGH)
      increaseNumber(1);
    if (digitalRead(thirdIncrease) == HIGH)
      increaseNumber(2);
    for (int i = 0; i < 3; i++)
      stepper[i].off();
  }
  numberDisplayed = displayedNumber[2] * 100 + displayedNumber[1] * 10 + displayedNumber[0];
  digitalWrite(ledPin, LOW);
  Serial.println("resume counter");
  timerCounter = millis();
}

void zeroCount(int digit)
{
  stepper[digit].stop();
  displayedNumber[digit] = 0;
  zeroPosition[digit] = true;
}

void countNumber(int digit)
{
  displayedNumber[digit]++;
  if (zeroPosition[digit])
    displayedNumber[digit] = 0;

  // Serial.print("Number ");
  // Serial.print(digit);
  // Serial.print(" = ");
  // Serial.println(displayedNumber[digit]);

  numberChanged[digit] = true;
  zeroPosition[digit] = false;
}

void gotoZeroPosition(int digit)
{
  while (!zeroPosition[digit])
  {
    stepper[digit].move(moveClockwise, 1);
  }
  // Serial.print("Zero Position Digit : ");
  // Serial.println(digit);

  stepper[digit].move(moveClockwise, numberThresh[digit]);
}