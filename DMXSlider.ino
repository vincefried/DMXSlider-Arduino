#include <DMXSerial.h>

#define BAUD 115200
#define BLE_BUFFER_SIZE 20

/// Setup
void setup() {
  // Start serial connection to HM-10 Modul, connected to TX3 and RX3
  Serial3.begin(BAUD);

  // Initialize DMX Controller
  DMXSerial.init(DMXController);
  DMXSerial.write(3, 90);    //Color
  DMXSerial.write(4, 43);     //Gobo /muster
  DMXSerial.write(5, 8);     //Shutter Strobe
  DMXSerial.write(6, 255);   
  DMXSerial.write(7, 0);
  DMXSerial.write(8, 0);
}

// Buffer
char buff[BLE_BUFFER_SIZE] = {0};
// Counter
int buffCounter = 0;

struct ControlXYCommand {
  int x;
  int y;
};

/// Frees the buffer and resets the flag
void freeBuffer() {
  for (int i = 0; i < buffCounter; i++) {
    buff[i] = 0;
  }

  buffCounter = 0;
}

/// Reads from the serial input if available
bool readSerial() {
  if (Serial3.available()) {
    // Get single char from HM-10
    char c = Serial3.read();
    // Activate flag if line feed found
    // Add char to buffer
    buff[buffCounter] = c;
    buffCounter++;
    return c == '\n';
  }

  return false;
}

/// Parses the buffer into a command struct
struct ControlXYCommand parseBuffer(char buff[], int buffSize) {
  ControlXYCommand command = { 0, 0 };

  // Buffer for temp values
  char tmp[buffSize] = {0};
  int tmpCounter = 0;

  // Iterate through buffer
  for (int i = 0; i < buffSize; i++) {
    // If x found, reset tmp buffer
    if (buff[i] == 'x') {
      for (int i = 0; i < tmpCounter; i++) {
        tmp[i] = 0;
      }
      tmpCounter = 0;
    }
    // If y found, set buffered x, and reset tmp buffer 
    else if (buff[i] == 'y') {
      command.x = atoi(tmp);
      for (int i = 0; i < tmpCounter; i++) {
        tmp[i] = 0;
      }
      tmpCounter = 0;
    }
    // If line feed found, set buffered y, and reset tmp buffer 
    else if (buff[i] == '\n') {
      command.y = atoi(tmp);
      for (int i = 0; i < tmpCounter; i++) {
        tmp[i] = 0;
      }
      tmpCounter = 0;
    }
    // If nothing special found, fill tmp buffer
    else if ((buff[i] != 'x') && (buff[i] != 'y')) {
      tmp[tmpCounter] = buff[i];
      tmpCounter++;
    }
  }
  
  return command;
}

/// Handles a control xy command and controls the DMX
void handleCommand(ControlXYCommand command) {  
  DMXSerial.write(1, command.x);
  DMXSerial.write(2, command.y);
}

/// Main loop
void loop() {  
  // If reading finished
  if (readSerial()) {
    // Parse buffer into command struct
    ControlXYCommand command = parseBuffer(buff, buffCounter);
    handleCommand(command);

    // Free buffer
    freeBuffer();
  }
}
