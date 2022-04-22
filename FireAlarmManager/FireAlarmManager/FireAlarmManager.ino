
#include <Timeout.h>

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 2;  // the number of the button pin
const int relayPin = 10;  // the number of the relay pin
const int buzzerPin = 11; // the number of the buzzer pin
const int ledPin = 13;    // the number of the LED pin

#define SEC_TO_MS(x) (unsigned long)((x) * 1000)
#define MS_TO_SEC(x) (unsigned long)((x) / 1000)
#define MIN_TO_SEC(x) (unsigned long)((x) * 60)
#define SEC_TO_MIN(x) (unsigned long)((x) / 60)

typedef enum {
  FIRE_ALARM_IDLE = 0,
  FIRE_ALARM_TURNING_OFF,
} FireAlarmState;

typedef enum {
  BUZZER_IDLE = 0,
  BUZZER_ON,
  BUZZER_OFF,
} BuzzerState;

const unsigned long MILLIS_200 = 200;
const unsigned long MILLIS_500 = 500;
const unsigned long MASTER_TIME = SEC_TO_MS(MIN_TO_SEC(15));
const long BAUDRATE = 38400;

// variables will change:
bool fireAlarm = false;
bool fireAlarm_TurningOff = false;
unsigned char buzzer_Times = 0;
unsigned long buzzer_TimeOn = 0;
unsigned long buzzer_TimeOff = 0;
TimeoutCpp masterTimer;


void setup() {
  Serial.begin(BAUDRATE);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  digitalWrite(relayPin, LOW);
  digitalWrite(buzzerPin, LOW);

  Buzzer_ConfigurationBeep(2, 250, 250);
}

void Buzzer_TurnOn() {
  digitalWrite(buzzerPin, HIGH);
}

void Buzzer_TurnOff() {
  digitalWrite(buzzerPin, LOW);
}

void Buzzer_ConfigurationBeep(unsigned char times, unsigned long timeOn, unsigned long timeOff) {
  buzzer_Times = times;
  buzzer_TimeOn = timeOn;
  buzzer_TimeOff = timeOff;
}

static void Beep() {
  static TimeoutCpp timerBuzzer;
  static BuzzerState buzzer_State = BUZZER_IDLE;

  switch (buzzer_State) {
    case BUZZER_IDLE: {
        if (buzzer_Times) {
          Buzzer_TurnOn();
          timerBuzzer.Start(buzzer_TimeOn);
          buzzer_State = BUZZER_ON;
        }
        break;
      }
    case BUZZER_ON: {
        if (timerBuzzer.CheckAndRestart(buzzer_TimeOff)) {
          Buzzer_TurnOff();
          buzzer_State = BUZZER_OFF;
        }
        break;
      }
    case BUZZER_OFF: {
        if (timerBuzzer.Check()) {
          if (buzzer_Times && buzzer_Times != UINT8_MAX) {
            buzzer_Times--;
          }
          buzzer_State = BUZZER_IDLE;
        }
        break;
      }
    default:
      buzzer_State = BUZZER_IDLE;
      break;
  }
}

void Buzzer_Update() {
  Beep();
}

void FireAlarm_Update() {
  static FireAlarmState fireAlarm_State = FIRE_ALARM_IDLE;
  static TimeoutCpp fireAlarm_TurningOffTimer;

  switch (fireAlarm_State) {
    case FIRE_ALARM_IDLE:
      if (fireAlarm_TurningOff) {
        static const unsigned char timesBeep = 5;
        Buzzer_ConfigurationBeep(timesBeep, MILLIS_500, MILLIS_500);
        fireAlarm_TurningOffTimer.Start(timesBeep * (MILLIS_500 * 2));
        fireAlarm_State = FIRE_ALARM_TURNING_OFF;
      }
      break;
    case FIRE_ALARM_TURNING_OFF:
      if (fireAlarm_TurningOffTimer.Check()) {
        digitalWrite(relayPin, LOW);
        fireAlarm = false;
        fireAlarm_TurningOff = false;
        fireAlarm_State = FIRE_ALARM_IDLE;
      }
      break;
  }
}

bool FireAlarm_IsEnabled() {
  return fireAlarm;
}

void FireAlarm_TurnOn() {
  //Buzzer_ConfigurationBeep(1, MILLIS_200, 0);
  digitalWrite(relayPin, HIGH);
  fireAlarm = true;
  fireAlarm_TurningOff = false;
  masterTimer.Start(MASTER_TIME);
}

void FireAlarm_TurnOff(bool slowTurnOff) {
  if (slowTurnOff) {
    fireAlarm_TurningOff = true;
  } else {
    digitalWrite(relayPin, LOW);
    fireAlarm = false;
  }
}

void Button_Update() {
  static bool releasedButton = true;
  static int buttonState = HIGH;
  static int lastButtonState = HIGH;
  static const unsigned long DEBOUNCE_TIME = 50, PUSHED_TIME = 100, HELDED_TIME = 2000;
  static TimeoutCpp timerDebounce;
  static TimeoutCpp timerPush;
  static TimeoutCpp timerHeld;

  int reading = digitalRead(buttonPin);

  if (lastButtonState != reading) {
    timerDebounce.Start(DEBOUNCE_TIME);
    timerPush.Start(PUSHED_TIME);
    timerHeld.Start(HELDED_TIME);
  }

  if (timerDebounce.IsStarted() && timerDebounce.CheckAndRestart(DEBOUNCE_TIME)) {
    if (buttonState != lastButtonState) {
      buttonState = lastButtonState;
    }

    if (buttonState == LOW) {
      if (!FireAlarm_IsEnabled() && releasedButton && timerPush.Check()) {
        FireAlarm_TurnOn();
        releasedButton = false;
      } else if (FireAlarm_IsEnabled() && releasedButton && timerHeld.Check()) {
        FireAlarm_TurnOff(false);
        releasedButton = false;
      }
    } else {
      releasedButton = true;
    }
  }

  lastButtonState = reading;
}

void Led_Update() {
  static const unsigned long BLINK_LED = MILLIS_200;
  static TimeoutCpp toggleLed;
  if (toggleLed.IsStarted() && toggleLed.CheckAndRestart(BLINK_LED)) {
    digitalWrite(ledPin, !digitalRead(ledPin));
  } else if (!toggleLed.IsStarted()) {
    //toggleLed.SetSerialtoPrint(Serial);
    toggleLed.Start(BLINK_LED);
  }
}

void loop() {
  Led_Update();
  Button_Update();
  Buzzer_Update();
  FireAlarm_Update();

  if (FireAlarm_IsEnabled() && masterTimer.IsStarted() && masterTimer.Check()) {
    FireAlarm_TurnOff(true);
  }
}
