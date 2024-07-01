/*
  SensorTag Button

  This example scans for Bluetooth® Low Energy peripherals until a TI SensorTag is discovered.
  It then connects to it, discovers the attributes of the 0xffe0 service,
  subscribes to the Simple Key Characteristic (UUID 0xffe1). When a button is
  pressed on the SensorTag a notification is received and the button state is
  outputted to the Serial Monitor when one is pressed.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.
  - TI SensorTag

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("Bluetooth® Low Energy Central - SensorTag button");
  Serial.println("Make sure to turn on the device.");

  // start scanning for peripheral
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    // Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName().substring(0, 13));
    // Serial.print("' ");
    // Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // Check if the peripheral is a SensorTag, the local name will be:
    // "CC2650 SensorTag"
    if (peripheral.localName().substring(0, 13) == "BBC micro:bit") {
      // stop scanning
      BLE.stopScan();

      monitorSensorTagButtons(peripheral);

      // peripheral disconnected, start scanning again
      //BLE.scan();
    }
  }
}

void monitorSensorTagButtons(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");
  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  Serial.println(peripheral.discoverAttributes());
  // discover peripheral attributes
  Serial.println("Discovering service 0xffe0 ...");
  Serial.println("Services found:");
        
    // Iterate through each service
    for (int i = 0; i < peripheral.serviceCount(); i++) {
        BLEService service = peripheral.service(i);
        Serial.print("- Service: ");
        Serial.println(service.uuid());
    }
    //- Service: e95d0753-251d-470a-a062-fa1922dfa9a8

  if (peripheral.discoverService("e95d0753-251d-470a-a062-fa1922dfa9a8")) {
    Serial.println("Service discovered");
  } else {
    Serial.println("Attribute discovery failed.");
    // peripheral.disconnect();

    // while (1);
    // return;
  }
  BLEService accelService = peripheral.service("e95d0753-251d-470a-a062-fa1922dfa9a8");
  BLECharacteristic accelCharacteristic = peripheral.characteristic("e95dca4b-251d-470a-a062-fa1922dfa9a8");
  if (accelCharacteristic){
    Serial.println("got accelCharacteristic");
    exploreCharacteristic(accelCharacteristic);
  } else{
    Serial.println("unable to get accelCharacteristic");
  }
}

void exploreCharacteristic(BLECharacteristic characteristic) {
  while (characteristic.canRead()) { //true if characteristic is readable
    characteristic.read(); //
    if (characteristic.valueLength() > 0) { //the size of the value in bytes

      short dataHex[3] = {}; // data is 3 signed 16 bit values
      characteristic.readValue(dataHex, 6); // so we read 6 bytes at a time

      Serial.print("X: ");
      Serial.println(dataHex[0]);
      Serial.print("Y: ");
      Serial.println(dataHex[1]);
      Serial.print("Z: ");
      Serial.println(dataHex[2]);

      Serial.println();
      delay(1000);
    }  
  }
}


