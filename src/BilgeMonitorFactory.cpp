#include "BilgeMonitorFactory.h"

#include "Arduino.h"

namespace {
bool invert_switch(int input) {
  return input == 0;
}
}

BilgeMonitorFactory::BilgeMonitorFactory(int pin)
    : pin_(pin) {}

void BilgeMonitorFactory::create() {
  auto* bilge =
      new DigitalInputState(pin_, INPUT_PULLDOWN, read_delay_ms_);

  auto* invert =
      new LambdaTransform<int, bool>(invert_switch);

  bilge
      ->connect_to(invert)
      ->connect_to(new SKOutputBool(
          sk_path_,
          sk_config_path_));
}
