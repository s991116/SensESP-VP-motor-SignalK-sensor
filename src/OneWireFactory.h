#pragma once

#include "sensesp_onewire/onewire_temperature.h"

using namespace sensesp;
using namespace sensesp::onewire;

class OneWireFactory {
 public:
  OneWireFactory(int one_wire_pin);

  void createTemperatureSensor(
      const char* onewire_config_path,
      const char* linear_config_path,
      const char* sk_path,
      const char* sk_config_path);

 private:
  DallasTemperatureSensors* dts_;
};
