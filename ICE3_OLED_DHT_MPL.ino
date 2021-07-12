// Adafruit IO Temperature & Humidity Example
// Tutorial Link: https://learn.adafruit.com/adafruit-io-basics-temperature-and-humidity
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016-2017 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.

#include "config.h" // you will need your own config.h file to run this code

// Include for the LED display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Include for DHT22
#include <DHT.h>
#include <DHT_U.h>
#define DATA_PIN 32   // pin connected to DH22 data line for ESP32
// create DHT22 instance
DHT_Unified dht(DATA_PIN, DHT22);

// Include for MPL115A2
#include <Wire.h>
#include <Adafruit_MPL115A2.h>
Adafruit_MPL115A2 mpl115a2;

// DEBUG mode: if 1 we are not logging to Adafruit, if 0 we are posting data!
#define DEBUG 0

// set up the 'temperature' and 'humidity' feeds
AdafruitIO_Feed *temperature = io.feed("temperature");
AdafruitIO_Feed *humidity = io.feed("humidity");
AdafruitIO_Feed *temperatureMPL = io.feed("temperaturempl");
AdafruitIO_Feed *pressure = io.feed("pressure");
// set up the 'digital' feed for software push button
AdafruitIO_Feed *digital = io.feed("digital");

float current_time = 0; // Current time to cause less over loading of the IO
String button_state = ""; // Variable for printing the button state

void setup() {

  // start the serial connection and show the filename
  Serial.begin(115200);
  Serial.print("This board is running: ");
  Serial.println(F(__FILE__));
  Serial.print("Compiled: ");
  Serial.println(F(__DATE__ " " __TIME__));
  delay(2000);

  // Check if the screen is functioning
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Check if the MPL sensor is connected
  Serial.println("Getting barometric pressure ...");
  if (! mpl115a2.begin()) {
    Serial.println("Sensor not found! Check wiring");
    while (1);
  }

  // Setup the OLED display
  // Clear the buffer
  display.clearDisplay();
  // Draw a single pixel in white - just to make sure this thing works
  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();
  delay(2000);
  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  
  // initialize dht22
  dht.begin();

  if (!DEBUG){
  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // set up a message handler for the 'digital' feed.
  digital->onMessage(handleMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
 
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
 }

 // Receive signal from digital IO
 digital->get();
  
}


void loop() {
  if (!DEBUG){
    io.run(); // Run the IO. it keeps the client connected to io.adafruit.com, and processes any incoming data.
  }
  
  // DHT Get temperature and humidity
  sensors_event_t event; // Setup events
  dht.temperature().getEvent(&event);
  float celsius = event.temperature;
  float fahrenheit = (celsius * 1.8) + 32;
  dht.humidity().getEvent(&event);
  float humidityDHT = event.relative_humidity;

  // MPL115A2 Get Readings
  float pressureKPA = 0, temperatureC = 0;    
  mpl115a2.getPT(&pressureKPA,&temperatureC);
  pressureKPA = mpl115a2.getPressure();  
  temperatureC = mpl115a2.getTemperature();  

  // Below is the code to print all of the lines to the OLED display
  // The file name, time since startup, temperature, humidity, and pressure will be present
  // Additionally, the virtual button state will be printed
  display.clearDisplay();
  display.setCursor(0, 0);            // Start at top-left corner - xpos, ypos
  display.println("Compiled: ");
  display.println(F(__DATE__ ", " __TIME__));
  display.println("Time since startup: ");
  display.print(millis());
  display.println(" milliseconds");
  display.print("DHT: ");
  display.print(celsius);
  display.print(" C  ");
  display.print(humidityDHT);
  display.println(" %");
  display.print("MPL: ");
  display.print(temperatureC);
  display.print(" C  ");
  display.print(pressureKPA);
  display.println(" kPa");
  display.println(button_state);
  display.display();
  delay(10);
  Serial.println(millis());

  // Below will only send the output to the IO every 10 seconds
  // This is meant to reduce throttling of the dashboard without slowing down display printing
  if (millis() > current_time + 10000){
    temperature->save(celsius);
    humidity->save(humidityDHT);
    temperatureMPL->save(temperatureC);
    pressure->save(pressureKPA);
    current_time = millis();
  } 
}

void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->toPinLevel()); // Print the incoming data
  
  //Prints the incoming data to the serial port
  // Sets the button state to "HIGH" is received 1 and vice versa
  if(data->toPinLevel() == HIGH){ 
    Serial.println("HIGH");
    button_state = "HIGH";
  }
  else {
    Serial.println("LOW");
    button_state = "LOW";
  }
  
}
