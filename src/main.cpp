#include <memory>

#include "Arduino.h"
#include "sensesp.h"
#include "sensesp/sensors/analog_input.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp/transforms/frequency.h"
#include "sensesp/transforms/linear.h"
#include "sensesp/transforms/moving_average.h"
#include "sensesp_app_builder.h"
#include "sensesp_onewire/onewire_temperature.h"
#include "OneWireFactory.h"

using namespace sensesp;
using namespace sensesp::onewire;

#define RPM_COUNTER_PIN   (4)
#define ONE_WIRE_BUS_PIN (16)
#define BILGE_SWITCH_PIN (25)

void EngingRPMCounterSetup();
void EngineBilgeMonitorSetup();

// The setup function performs one-time application initialization.
void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    // Set a custom hostname for the app.
                    ->set_hostname("vp-motor-sensor")
                    // Optionally, hard-code the WiFi and Signal K server
                    // settings. This is normally not needed.
                    //->set_wifi_client("Trevangsvej_197", "xx")
                    //->set_wifi_access_point("vp-motor-sensor", "xx")
                    //->set_sk_server("192.168.1.180", 80)
                    //->set_sk_server("openplotter.local", 80)
                    ->enable_uptime_sensor()
                    ->get_app();

  OneWireFactory oneWireFactory(ONE_WIRE_BUS_PIN);

  // A
  oneWireFactory.createTemperatureSensor(
    "/coolantTemperature/oneWire",
    "/coolantTemperature/linear",
    "propulsion.mainEngine.coolantTemperature",
    "/coolantTemperature/skPath");

  // B
  oneWireFactory.createTemperatureSensor(
    "/exhaustTemperature/oneWire",
    "/exhaustTemperature/linear",
    "propulsion.mainEngine.exhaustTemperature",
    "/exhaustTemperature/skPath");

  // C
  oneWireFactory.createTemperatureSensor(
    "/engineRoomTemperature/oneWire",
    "/engineRoomTemperature/linear",
    "environment.inside.engineRoom.temperature",
    "/engineRoomTemperature/skPath");

  EngingRPMCounterSetup();
  EngineBilgeMonitorSetup();

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }

void EngingRPMCounterSetup() {
  const char* config_path_calibrate = "/Engine RPM/calibrate";
  const char* config_path_skpath = "/Engine RPM/sk_path";
  const float multiplier = 0.075275648;

  auto* sensor =
      new DigitalInputCounter(RPM_COUNTER_PIN, INPUT_PULLUP, RISING, 500);

  sensor
      ->connect_to(new Frequency(multiplier, config_path_calibrate))
      // connect the output of sensor to the input of Frequency()
      ->connect_to(new MovingAverage(2, 1.0, "/Engine RPM/movingAVG"))
      ->connect_to(new SKOutputFloat("propulsion.engine.revolutions",
                                     config_path_skpath));
}

auto bilge_switch_invert_function = [](int input) -> bool {
  return input == 0;
};

void EngineBilgeMonitorSetup() {
  auto* bilge = new DigitalInputState(BILGE_SWITCH_PIN, INPUT_PULLDOWN, 5000);
  auto* bilge_switch_invert = new LambdaTransform<int, bool>(bilge_switch_invert_function);
  
  bilge->connect_to(bilge_switch_invert)->connect_to(new SKOutputBool("propulsion.engine.bilge",
                                     "/Engine Bilge filled/sk_path"));
}
