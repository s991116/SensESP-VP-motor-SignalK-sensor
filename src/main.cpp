#include <Arduino.h>
#include <FastLED.h>

#include "sensesp.h"
#include "sensesp_app_builder.h"

// --- Engine / Sensors ---
#include "OneWireFactory.h"
#include "EngineRpmFactory.h"
#include "BilgeMonitorFactory.h"

// --- LED strip ---
#include "LedStrip.h"
#include "LedStripFactory.h"

// ------------------------

using namespace sensesp;

// ---------- Pins ----------
#define RPM_COUNTER_PIN     4
#define ONE_WIRE_BUS_PIN   16
#define BILGE_SWITCH_PIN   25
#define LED_DATA_PIN       26

// ---------- LED config ----------
#define NUM_LEDS 5
CRGB leds[NUM_LEDS];

LedStripFactory* ledFactory = nullptr;

// ------------------------------

void setup() {
  SetupLogging(ESP_LOG_DEBUG);

  SensESPAppBuilder builder;
  sensesp_app = builder
      .set_hostname("vp-engine-led-controller")
      ->enable_uptime_sensor()
      ->get_app();

  // =======================
  // Engine sensors
  // =======================

  OneWireFactory oneWireFactory(ONE_WIRE_BUS_PIN);

  oneWireFactory.createTemperatureSensor(
      "/coolantTemperature/oneWire",
      "/coolantTemperature/linear",
      "propulsion.mainEngine.coolantTemperature",
      "/coolantTemperature/skPath");

  oneWireFactory.createTemperatureSensor(
      "/exhaustTemperature/oneWire",
      "/exhaustTemperature/linear",
      "propulsion.mainEngine.exhaustTemperature",
      "/exhaustTemperature/skPath");

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
      "/Engine RPM/sk_path");

  EngineRpmFactory engineRpm(engine_rpm_config);
  engineRpm.create();

  BilgeMonitorFactory bilgeFactory(BILGE_SWITCH_PIN);
  bilgeFactory.create();

  // =======================
  // LED strip
  // =======================

  FastLED.addLeds<WS2812B, LED_DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.clear(true);

  ledFactory = new LedStripFactory(leds, NUM_LEDS);

  ledFactory->addSegment(
      0, 3,
      "electrical.switches.lightStairs.state",
      "electrical.switches.lightStairs.level",
      "electrical.switches.nightMode");

  ledFactory->addSegment(
      3, 2,
      "electrical.switches.lightOutside.state",
      "electrical.switches.lightOutside.level",
      "electrical.switches.nightMode");

  event_loop()->onRepeat(1000, []() {
    static bool emitted = false;
    if (!emitted) {
      ledFactory->emitInitialState();
      ESP_LOGI("Main", "Initial LED state sent to Signal K");
      emitted = true;
    }
  });

  sensesp_app->start();
}

void loop() {
  event_loop()->tick();

  if (ledFactory != nullptr) {
    ledFactory->updateAll();
  }
}
