#include <ArduinoJson.h>
#include <DMXSerial.h>

#define BAUD 115200
#define JSON_BUFFER 200

/// Setup
void setup() {
  // Start serial connection to HM-10 Modul, connected to TX3 and RX3
  Serial3.begin(BAUD);

  // Initialize DMX Controller
  DMXSerial.init(DMXController);
  DMXSerial.write(3, 45);    //Color
  DMXSerial.write(4, 8);     //Gobo /muster
  DMXSerial.write(5, 8);     //Shutter Strobe
  DMXSerial.write(6, 255);   
  DMXSerial.write(7, 0);
  DMXSerial.write(8, 0);
}

// Json buffer
char json[JSON_BUFFER] = {0};
// Json counter
int jsonCounter = 0;
// Flag for flagging finished reading
bool readJson = false;

/// Frees the json buffer and resets the flag
void freeJson() {
  for (int i = 0; i < jsonCounter; i++) {
    json[i] = 0;
  }

  jsonCounter = 0;
  readJson = false;
}

/// Reads from the serial input if available
void readSerial() {
  if (Serial3.available()) {
    // Get single char from HM-10
    char c = Serial3.read();
    // Activate flag if line feed found
    readJson = c == '\n';
    // Add char to buffer
    json[jsonCounter] = c;
    jsonCounter++;
  }
}

bool parseJson(JsonDocument *doc, char buff[]) {
    // Deserialize the JSON document
    DeserializationError error = deserializeJson(*doc, buff);
    
    // Test if parsing succeeds and reset buffer if failed
    if (error) {
      freeJson();
      return false;
    } else {
      return true;
    }
}

/// Handles a json document according to its content
void handleJson(JsonDocument doc) {
    if (strcmp(doc["c"], "x") == 0) {
      // Check led state in command
      DMXSerial.write(1, doc["x"]);     // 0 = rechts - 255 = links
    } else if (strcmp(doc["c"], "y") == 0) {
      // Check led state in command
      DMXSerial.write(1, doc["y"]);
    } 
}

/// Main loop
void loop() {
  // Read input from HM-10 if available
  readSerial();

  // If reading finished
  if (readJson) {
    // Parse json
    StaticJsonDocument<JSON_BUFFER> doc;
    bool success = parseJson(&doc, json);

    // Handle doc if success
    if (success) {
      handleJson(doc);
    }

    // Free buffer
    freeJson();
  }
}
