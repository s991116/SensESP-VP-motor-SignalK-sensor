#include "EngineRpmFactory.h"
#include "Arduino.h"

EngineRpmFactory::EngineRpmFactory(const RpmSensorConfig& config)
    : config_(config) {}

void EngineRpmFactory::create() {
  auto* sensor = new DigitalInputCounter(
      config_.pin,
      INPUT_PULLUP,
      RISING,
      config_.debounce_ms);

  sensor
      ->connect_to(new Frequency(
          config_.multiplier,
          config_.calibrate_config_path))
      ->connect_to(new MovingAverage(
          config_.moving_average_samples,
          config_.moving_average_scale,
          config_.moving_avg_config_path))
      ->connect_to(new SKOutputFloat(
          config_.sk_path,
          config_.sk_config_path));
}
