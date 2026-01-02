#pragma once

#include "sensesp/sensors/digital_input.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp/transforms/moving_average.h"
#include "sensesp/signalk/signalk_output.h"

using namespace sensesp;

struct RpmSensorConfig {
  int pin;
  float multiplier;

  int debounce_ms;
  int moving_average_samples;
  float moving_average_scale;

  const char* calibrate_config_path;
  const char* moving_avg_config_path;
  const char* sk_path;
  const char* sk_config_path;

  RpmSensorConfig(
      int pin_,
      float multiplier_,
      const char* calibrate_path,
      const char* moving_avg_path,
      const char* sk_path_,
      const char* sk_config_path_,
      int debounce_ms_ = 500,
      int moving_avg_samples_ = 2,
      float moving_avg_scale_ = 1.0)
      : pin(pin_),
        multiplier(multiplier_),
        debounce_ms(debounce_ms_),
        moving_average_samples(moving_avg_samples_),
        moving_average_scale(moving_avg_scale_),
        calibrate_config_path(calibrate_path),
        moving_avg_config_path(moving_avg_path),
        sk_path(sk_path_),
        sk_config_path(sk_config_path_) {}
};

class EngineRpmFactory {
 public:
  EngineRpmFactory(const RpmSensorConfig& config);

  void create();

 private:
  RpmSensorConfig config_;
};
