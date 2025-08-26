#include <Adafruit_BMP280.h>

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

using namespace sensesp;
using namespace sensesp::onewire;

#define RPM_COUNTER_PIN   (4)
#define ONE_WIRE_BUS_PIN (16)
#define BILGE_SWITCH_PIN (25)
//SDA GPIO 21
//SCL GPIO 22

void ExhaustOneWireTemperatureSetup(sensesp::onewire::DallasTemperatureSensors *dts);
void EngineBlockOneWireTemperatureSetup(sensesp::onewire::DallasTemperatureSensors *dts);
void EngineCoolantOneWireTemperatureSetup(sensesp::onewire::DallasTemperatureSensors *dts);
void EngingRPMCounterSetup();
void EngineBilgeMonitorSetup();
void EngineRoomTempSetup();

Adafruit_BMP280 bmp280; // I2C
float read_temp_callback() { return (bmp280.readTemperature());}
float read_pressure_callback() { return (bmp280.readPressure());}

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

  EngineRoomTempSetup();

  DallasTemperatureSensors* dts = new DallasTemperatureSensors(ONE_WIRE_BUS_PIN);
  ExhaustOneWireTemperatureSetup(dts);
  EngineBlockOneWireTemperatureSetup(dts);
  EngineCoolantOneWireTemperatureSetup(dts);
  
  EngingRPMCounterSetup();
  EngineBilgeMonitorSetup();

  // To avoid garbage collecting all shared pointers created in setup(),
  // loop from here.
  while (true) {
    loop();
  }
}

void loop() { event_loop()->tick(); }

void EngineRoomTempSetup() {
  /// Engine Room Temp Sensor ////
  bmp280.begin(0x76);

  // Create a RepeatSensor with float output that reads the temperature
  // using the function defined above.
  auto* engine_room_temp = new RepeatSensor<float>(5000, read_temp_callback);

  auto* engine_room_pressure =
      new RepeatSensor<float>(60000, read_pressure_callback);

  // Send the temperature to the Signal K server as a Float
  engine_room_temp->connect_to(new SKOutputFloat(
      "propulsion.engineRoom.temperature", "/Engineroom Temperature/sk_path"));

  engine_room_pressure->connect_to(new SKOutputFloat(
      "propulsion.engineRoom.pressure", "/Engineroom Preasure/sk_path"));
}

void ExhaustOneWireTemperatureSetup(sensesp::onewire::DallasTemperatureSensors* dts) {
  auto* exhaust_temp =
      new OneWireTemperature(dts, 1000, "/Exhaust Temperature/oneWire");

  exhaust_temp->connect_to(new Linear(1.0, 0.0, "/Exhaust Temperature/linear"))
      ->connect_to(new SKOutputFloat("propulsion.engine.1.exhaustTemperature",
                                     "/Exhaust Temperature/sk_path"));
}

void EngineBlockOneWireTemperatureSetup(sensesp::onewire::DallasTemperatureSensors* dts) {
  auto* enginge_block_temp =
      new OneWireTemperature(dts, 1000, "/Engine block Temperature/oneWire");

  enginge_block_temp->connect_to(new Linear(1.0, 0.0, "/Engine block Temperature/linear"))
      ->connect_to( 
          new SKOutputFloat("propulsion.engine.1.engineBlockTemperature","/Engine Block Temperature/sk_path"));
}

void EngineCoolantOneWireTemperatureSetup(sensesp::onewire::DallasTemperatureSensors* dts) {
  auto* enginge_coolant_temp =
      new OneWireTemperature(dts, 1000, "/Engine coolant Temperature/oneWire");

  enginge_coolant_temp->connect_to(new Linear(1.0, 0.0, "/Engine coolant Temperature/linear"))
      ->connect_to( 
          new SKOutputFloat("propulsion.engine.1.engineCoolantTemperature","/Engine Coolant Temperature/sk_path"));
}

void EngingRPMCounterSetup() {
  const char* config_path_calibrate = "/Engine RPM/calibrate";
  const char* config_path_skpath = "/Engine RPM/sk_path";
  const float multiplier = 1.0;

  auto* sensor =
      new DigitalInputCounter(RPM_COUNTER_PIN, INPUT_PULLUP, RISING, 500);

  sensor
      ->connect_to(new Frequency(multiplier, config_path_calibrate))
      // connect the output of sensor to the input of Frequency()
      ->connect_to(new MovingAverage(2, 1.0, "/Engine RPM/movingAVG"))
      ->connect_to(new SKOutputFloat("propulsion.engine.revolutions",
                                     config_path_skpath));
  // connect the output of Frequency() to a Signal K Output as a number

  // sensor->connect_to(new Frequency(6))
  //  times by 6 to go from Hz to RPM
  //         ->connect_to(new MovingAverage(4, 1.0,"/Engine Fuel/movingAVG"))
  //         ->connect_to(new FuelInterpreter("/Engine Fuel/curve"))
  //         ->connect_to(new SKOutputFloat("propulsion.engine.fuel.rate",
  //         "/Engine Fuel/sk_path"));
}

void EngineBilgeMonitorSetup() {
  auto* bilge = new DigitalInputState(25, INPUT_PULLUP, 5000);
  bilge->connect_to(new SKOutputBool("propulsion.engine.bilge",
                                     "/Engine Bilge filled/sk_path"));
}
