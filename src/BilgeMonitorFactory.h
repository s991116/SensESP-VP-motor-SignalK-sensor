#pragma once

#include "sensesp/sensors/digital_input.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/moving_average.h"
#include "sensesp/signalk/signalk_output.h"

using namespace sensesp;

class BilgeMonitorFactory {
 public:
  BilgeMonitorFactory(int pin);

  void create();

 private:
  int pin_;

  // Konfiguration (kan senere gøres parametrisk)
  const char* sk_path_ = "propulsion.engine.bilge";
  const char* sk_config_path_ = "/Engine Bilge filled/sk_path";
  int read_delay_ms_ = 5000;
};
