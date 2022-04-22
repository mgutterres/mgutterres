
#include <Timeout.h>
#include <Arduino.h>

TimeoutCpp::TimeoutCpp() : interval(0), isStarted(false), startTime(0) {}

TimeoutCpp::TimeoutCpp(const TimeoutCpp &other) {
  this->interval = other.interval;
  this->isStarted = other.isStarted;
  this->startTime = other.startTime;
}

TimeoutCpp::~TimeoutCpp() {}

void TimeoutCpp::SetSerialtoPrint(HardwareSerial &print) {
	printer = &print; //operate on the address of print
}

bool TimeoutCpp::IsStarted() { return this->isStarted; }

bool TimeoutCpp::Start(unsigned long interval) {
  this->startTime = millis();

  this->interval = interval;
  this->isStarted = true;
  return true;
}

unsigned long TimeoutCpp::GetElapsed() {
  if (!this->isStarted) {
    return 0;
  }

  unsigned long tmpTime = millis();
  tmpTime -= this->startTime;  
  return tmpTime;
}

void TimeoutCpp::Stop() { this->isStarted = false; }

bool TimeoutCpp::Check() {
  if (!this->isStarted) {
    return false;
  }

  unsigned long elapsed = GetElapsed();
  bool isElapsed = (elapsed > this->interval);

  if (isElapsed) {
	if (printer) {
	  printer->println("End of time!");
	}
    this->isStarted = false;
  }

  return isElapsed;
}

bool TimeoutCpp::CheckAndRestart(unsigned long interval) {
  if (Check()) {
    Start(interval);
    return true;
  }

  return false;
}

void TimeoutCpp::Delay(unsigned long interval) {
  TimeoutCpp delay;

  delay.Start(interval);
  while (!delay.Check())
    ;
}

unsigned long TimeoutCpp::GetInterval() { return this->interval; }
