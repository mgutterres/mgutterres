#ifndef TIMEOUTCPP_H
#define TIMEOUTCPP_H

#include "HardwareSerial.h"

class TimeoutCpp {
 private:
  unsigned long startTime;
  unsigned long interval;
  bool isStarted;
  HardwareSerial* printer;

 public:
  TimeoutCpp();
  TimeoutCpp(const TimeoutCpp &other);
  ~TimeoutCpp();

  void SetSerialtoPrint(HardwareSerial&);
  bool IsStarted();
  bool Start(unsigned long);
  void Stop();
  unsigned long GetElapsed();
  bool Check();
  bool CheckAndRestart(unsigned long);
  void Delay(unsigned long);
  unsigned long GetInterval();
};

#endif  // TIMEOUTCPP_H
