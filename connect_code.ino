// currently does not work, the accelerometer service is not found


#include <bluefruit.h>
#include <string>

BLEClientBas clientBas;  // battery client
BLEClientDis clientDis;  // device information client
BLEClientUart clientUart; // bleuart client

uint8_t ACCEL_SERVICE_UUID[] = {0xE9,0x5D,0x07,0x53,0x25,0x1D,0x47,0x0A,0xA0,0x62,0xFA,0x19,0x22,0xDF,0xA9,0xA8};
uint8_t ACCEL_CHARACTERISTIC_UUID[] = {0xE9,0x5D,0xCA,0x4B,0x25,0x1D,0x47,0x0A,0xA0,0x62,0xFA,0x19,0x22,0xDF,0xA9,0xA8};

BLEClientService accel_service = BLEClientService(ACCEL_SERVICE_UUID);
BLEClientCharacteristic accel_characteristic = BLEClientCharacteristic(ACCEL_CHARACTERISTIC_UUID);

void setup() 
{
  Serial.begin(115200);
  while (!Serial) delay(10);   // for nrf52840 with native usb

  Serial.println("Bluefruit52 Central ADV Scan Example");
  Serial.println("------------------------------------\n");

  // Initialize Bluefruit with maximum connections as Peripheral = 0, Central = 1
  Bluefruit.begin(0, 1);
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values

  Bluefruit.setName("Bluefruit52");
  Bluefruit.setConnLedInterval(250);

  accel_service.begin();
  accel_characteristic.setNotifyCallback(accel_notify_callback);
  accel_characteristic.begin();

  // Init BLE Central Uart Service
  clientUart.begin();
  clientUart.setRxCallback(bleuart_rx_callback);

  // Callbacks for Central
  Bluefruit.Central.setConnectCallback(connect_callback);
  Bluefruit.Central.setDisconnectCallback(disconnect_callback);

  // Start Central Scanning
  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.filterRssi(-80);
  Bluefruit.Scanner.setInterval(160, 80);       // in units of 0.625 ms
  Bluefruit.Scanner.useActiveScan(true);        // Request scan response data
  Bluefruit.Scanner.start(0);                   // 0 = Don't stop scanning after n seconds

  Serial.println("Scanning ...");
}

void loop() 
{
  if (Bluefruit.Central.connected()) {
    // Not discovered yet
    if (clientUart.discovered()) {
      // Discovered means in working state
      // Get Serial input and send to Peripheral
      if (Serial.available()) {
        delay(2); // delay a bit for all characters to arrive
        char str[20+1] = { 0 };
        Serial.readBytes(str, 20);
        clientUart.print(str);
        Serial.printf("Sent: %s\n", str);
      }
    }
  }
}

void scan_callback(ble_gap_evt_adv_report_t* report)
{
  Serial.print("Found device: ");
  Serial.printBufferReverse(report->peer_addr.addr, 6, ':');
  Serial.println();

  // Check if the device has the expected name
  uint8_t buffer[32] = {0};
  if (Bluefruit.Scanner.parseReportByType(report, BLE_GAP_AD_TYPE_COMPLETE_LOCAL_NAME, buffer, sizeof(buffer))) {
    Serial.printf("Device name: %s\n", buffer);
    if (strncmp((const char*)buffer, "BBC micro:bit", 13) == 0) {
      Serial.println("Found micro:bit, connecting...");
      Bluefruit.Central.connect(report);
      Bluefruit.Scanner.stop();
    }
  }

  // For Softdevice v6: after received a report, scanner will be paused
  // We need to call Scanner resume() to continue scanning
  Bluefruit.Scanner.resume();
}

void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");

  if (clientDis.discover(conn_handle)) {
    Serial.println("Device Information Service found");

    char buffer[32+1] = {0};
    if (clientDis.getManufacturer(buffer, sizeof(buffer))) {
      Serial.print("Manufacturer: ");
      Serial.println(buffer);
    }

    if (clientDis.getModel(buffer, sizeof(buffer))) {
      Serial.print("Model: ");
      Serial.println(buffer);
    }
  } else {
    Serial.println("Device Information Service not found");
  }

  if (clientBas.discover(conn_handle)) {
    Serial.print("Battery level: ");
    Serial.print(clientBas.read());
    Serial.println("%");
  } else {
    Serial.println("Battery Service not found");
  }

  if (clientUart.discover(conn_handle)) {
    Serial.println("UART Service found");
    clientUart.enableTXD();
  } else {
    Serial.println("UART Service not found");
  }

  Serial.print("Discovering Accelerometer Service ... ");
  if (accel_service.discover(conn_handle)) {
    Serial.println("Service Found");

    Serial.print("Discovering Accelerometer Characteristic ... ");
    if (accel_characteristic.discover()) {
      Serial.println("Characteristic Found");

      Serial.print("Enabling Notify on the Characteristic ... ");
      if (accel_characteristic.enableNotify()) {
        Serial.println("Notify enabled, ready to receive data");
      } else {
        Serial.println("Failed to enable notify for Characteristic");
      }
    } else {
      Serial.println("Failed to discover Characteristic");
    }
  } else {
    Serial.println("Failed to discover Accelerometer Service");
  }
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  Bluefruit.Scanner.start(0); // Restart scanning
}

void accel_notify_callback(BLEClientCharacteristic* chr, uint8_t* data, uint16_t len)
{
  Serial.print("Accelerometer data: "); 
  for (uint16_t i = 0; i < len; i++) {
    Serial.printf("%02X ", data[i]);
  }
  Serial.println();
}

void bleuart_rx_callback(BLEClientUart& uart_svc)
{
  Serial.println("Received data from UART:");
  while (uart_svc.available()) {
    Serial.print((char)uart_svc.read());
  }
  Serial.println();
}
