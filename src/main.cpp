/**
 * BasicHTTPClient.ino
 *
 *  Created on: 24.05.2015
 *
 */

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include <ThingSpeak.h>

#include <OwnCredentials.h>

ESP8266WiFiMulti WiFiMulti;

// Data wire is plugged into pin 0 on the olimex mowifi test
#define ONE_WIRE_BUS 0

// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Known ds18b20 sensor addresses
uint8_t sensor[3][8] = {{ 0x28, 0x3d, 0xca, 0x04, 0x05, 0x00, 0x00, 0x1d },
                        { 0x28, 0xa2, 0xc0, 0x05, 0x05, 0x00, 0x00, 0xba },
                        // Sensor with encapsulation
                        { 0x28, 0xf3, 0x51, 0x22, 0x03, 0x00, 0x00, 0xfe }};

WiFiClient  client;

void setup() {
    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
    WiFiMulti.addAP(WLAN_SSID, WLAN_PASSWORD);

    Serial.println();
    Serial.println();
    Serial.print("Wait for WiFi... ");
    Serial.print(WiFiMulti.run());

    while(WiFiMulti.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    delay(500);
    // Start up the library
    sensors.begin();
    delay(500);

    ThingSpeak.begin(client);

    Serial.println("Found the following Dallas ds18b20 temperature sensors: ");
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
        uint8_t address[8] = { 0 };
        sensors.getAddress(address, i);
        for (int j = 0; j < 8; j++) {
            Serial.printf("0x%02x", address[j]);
            if (j < 7) {
                Serial.printf(",");
            }
        }
        Serial.println();
    }
}

int device_index(int sensor_index) {
    uint8_t address[8] = { 0 };

    sensors.getAddress(address, sensor_index);

    for (int i = 0; i < sensors.getDeviceCount(); i++) {
        if (memcmp( address, sensor[i], sizeof(address) ) == 0) {
            return i + 1;
        }
    }
    return -1;
}

void read_temperature(void)
{
    float temperature = 0;

    // call sensors.requestTemperatures() to issue a global temperature
    // request to all devices on the bus
    Serial.print("   Request temperatures");
    sensors.requestTemperatures(); // Send the command to get temperatures
    Serial.println("...DONE");
    delay(500);

    Serial.printf("Temperature for Device: ");
    for (int i = 0; i < sensors.getDeviceCount(); i++) {
        int device = device_index(i);
        temperature = sensors.getTempCByIndex(i);
        Serial.printf("%d: ", device);
        Serial.print(temperature);
        Serial.print(" ");
        ThingSpeak.setField(device , temperature);
    }

    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {
        Serial.print("   Upload to ThingSpeak ");
        ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    }

}

void loop() {
    read_temperature();
    delay(15000);
}
