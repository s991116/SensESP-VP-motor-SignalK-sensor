#include "OneWireFactory.h"

#include "sensesp/signalk/signalk_output.h"
#include "sensesp/transforms/linear.h"

OneWireFactory::OneWireFactory(int one_wire_pin) {
  dts_ = new DallasTemperatureSensors(one_wire_pin);
}

void OneWireFactory::createTemperatureSensor(
    const char* onewire_config_path,
    const char* linear_config_path,
    const char* sk_path,
    const char* sk_config_path) {

  auto* oneWireTemperature =
      new OneWireTemperature(dts_, 1000, onewire_config_path);
  auto* calibration = new Linear(1.0, 0.0, linear_config_path);
  auto* output = new SKOutputFloat(sk_path, sk_config_path);

  oneWireTemperature
      ->connect_to(calibration)
      ->connect_to(output);
}
