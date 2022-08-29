/*
 This example connects to an unencrypted Wifi network.
 Then it prints the  MAC address of the Wifi shield,
 the IP address obtained, and other network details.
 */
#include <SPI.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <secrets.h>

char ssid[] = WIFI_SSID;     //  your network SSID (name)
char pass[] = WIFI_PASSWORD;  // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClient client;

char server[] = "api.openweathermap.org";   // domain for request

void printWifiData() {
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    Serial.println(ip);

    // print your MAC address:
    byte mac[6];
    WiFi.macAddress(mac);
    Serial.print("MAC address: ");
    Serial.print(mac[5], HEX);
    Serial.print(":");
    Serial.print(mac[4], HEX);
    Serial.print(":");
    Serial.print(mac[3], HEX);
    Serial.print(":");
    Serial.print(mac[2], HEX);
    Serial.print(":");
    Serial.print(mac[1], HEX);
    Serial.print(":");
    Serial.println(mac[0], HEX);
}

void printCurrentNet() {
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print the MAC address of the router you're attached to:
    byte bssid[6];
    WiFi.BSSID(bssid);
    Serial.print("BSSID: ");
    Serial.print(bssid[5], HEX);
    Serial.print(":");
    Serial.print(bssid[4], HEX);
    Serial.print(":");
    Serial.print(bssid[3], HEX);
    Serial.print(":");
    Serial.print(bssid[2], HEX);
    Serial.print(":");
    Serial.print(bssid[1], HEX);
    Serial.print(":");
    Serial.println(bssid[0], HEX);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.println(rssi);

    // print the encryption type:
    byte encryption = WiFi.encryptionType();
    Serial.print("Encryption Type:");
    Serial.println(encryption, HEX);
    Serial.println();
}

void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(9600);

    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // check for the presence of the shield:
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true);
    }

    String fv = WiFi.firmwareVersion();
    if (fv != WIFI_FIRMWARE_LATEST_VERSION) {
        Serial.println("Please upgrade the firmware");
    }

    // attempt to connect to Wifi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        // Connect to WPA/WPA2 network:
        status = WiFi.begin(ssid, pass);
        // wait 10 seconds for connection:
        delay(10000);
    }

    // you're connected now, so print out the data:
    Serial.print("You're connected to the network");
    printCurrentNet();
    printWifiData();

    Serial.println("\nStarting connection to server...");
    // if you get a connection, report back via serial:
    if (client.connect(server, 80)) {
        Serial.println("connected to server");
        // Make a HTTP request:
        client.println("GET /data/2.5/weather?lat=40.7608&lon=-111.8910&units=imperial&cnt=1&appid=" OW_KEY " HTTP/1.1");
        client.println("Host: api.openweathermap.org");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
    }
}

void loop() {
    client.setTimeout(3000); // exit definitely after 3 second no data
    // try to read until the end of the HTTP headers are reached
    while(true) {
        int data = client.read();
        if (data <= 0) {
            continue;
        }
        char c = (char) data;
        //trigger on \r
        if( c == '\r') {
            // read the next 3 bytes when available
            while(client.available() < 3) {}
            char nextData[3] = {0};
            client.read((uint8_t*) nextData, sizeof(nextData));
            if(nextData[0] == '\n' && nextData[1] == '\r' && nextData[2] == '\n') {
                // end of HTTP headers reached!
                Serial.println("HTTP headers read.\n");
                break;
            }
        }
    }
    // the next data is all JSON response data
    static StaticJsonDocument<768> doc;
    DeserializationError err;
    Serial.println("Start deserializing from string now.");
    err = deserializeJson(doc, client);
    if(err != DeserializationError::Ok) {
        Serial.println("Deserialize error:");
        Serial.println(err.f_str());
    } else {
        Serial.println("DESERIALIZATION OKAY!!");
        Serial.print("Used memory: ");
        Serial.println(doc.memoryUsage());
        String buf;
        serializeJson(doc, buf);
        Serial.println("JSON data as string:");
        Serial.println(buf);
        // JsonObject root = doc.as<JsonObject>();
        // for (JsonPair kv : root) {
        //     Serial.println(kv.key().c_str());
        //     Serial.println(kv.value().as<char*>());
        // }
    }
    // block forever
    client.stop();
    while (true);
}
