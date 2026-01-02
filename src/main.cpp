#include "Arduino.h"

#include "sensesp.h"
#include "sensesp_app_builder.h"

#include "OneWireFactory.h"
#include "EngineRpmFactory.h"
#include "BilgeMonitorFactory.h"

using namespace sensesp;
using namespace sensesp::onewire;

#define RPM_COUNTER_PIN   (4)
#define ONE_WIRE_BUS_PIN (16)
#define BILGE_SWITCH_PIN (25)

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

  RpmSensorConfig engine_rpm_config(
    RPM_COUNTER_PIN,
    0.075275648,
    "/Engine RPM/calibrate",
    "/Engine RPM/movingAVG",
    "propulsion.engine.revolutions",
    "/Engine RPM/sk_path"
  );

  EngineRpmFactory engineRpm(engine_rpm_config);
  engineRpm.create();


  BilgeMonitorFactory bilgeFactory(BILGE_SWITCH_PIN);
  bilgeFactory.create();

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }