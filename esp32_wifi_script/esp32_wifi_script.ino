// /* Connect to the network with SSID defined in wifi_creds.h */
// #include <WiFi.h>
// #include "./wifi_creds.h"

// void setup() {
//   Serial.begin(115200);
//   WiFi.mode(WIFI_STA);  // Set ESP32 to station mode to (client)
//   WiFi.disconnect();    // Disconnect from any previous connections
//   delay(100);
//   WiFi.begin(ssid, password);

//   Serial.print("Connecting\n");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print("Waiting...\n");
//   }

//   Serial.println("\nConnected!");
//   Serial.println(WiFi.localIP());
// }

// void loop() {}

/* Scan WiFi networks */
// #include <WiFi.h>

// void setup() {
//   Serial.begin(115200);
//   delay(1000); // Give serial a moment to initialize

//   // Set WiFi to station mode
//   WiFi.mode(WIFI_STA);
//   WiFi.disconnect(); // Disconnect from any previous connections
//   delay(100);

//   Serial.println("\n\n----- WiFi Scanner -----");
//   Serial.println("Scanning for networks...");
// }

// void loop() {
//   // Start WiFi scan
//   Serial.println("\nScanning available networks...");
//   int networksFound = WiFi.scanNetworks();

//   if (networksFound == 0) {
//     Serial.println("No networks found!");
//   } else {
//     Serial.print(networksFound);
//     Serial.println(" networks found:");

//     // Print details of each network
//     for (int i = 0; i < networksFound; ++i) {
//       // Print SSID and RSSI for each network
//       Serial.print(i + 1);
//       Serial.print(": ");
//       Serial.print(WiFi.SSID(i));
//       Serial.print(" (");
//       Serial.print(WiFi.RSSI(i));
//       Serial.print(" dBm) ");

//       // Print encryption type
//       switch (WiFi.encryptionType(i)) {
//         case WIFI_AUTH_OPEN:
//           Serial.print("Open");
//           break;
//         case WIFI_AUTH_WEP:
//           Serial.print("WEP");
//           break;
//         case WIFI_AUTH_WPA_PSK:
//           Serial.print("WPA");
//           break;
//         case WIFI_AUTH_WPA2_PSK:
//           Serial.print("WPA2");
//           break;
//         case WIFI_AUTH_WPA_WPA2_PSK:
//           Serial.print("WPA+WPA2");
//           break;
//         case WIFI_AUTH_WPA2_ENTERPRISE:
//           Serial.print("WPA2-Enterprise");
//           break;
//         case WIFI_AUTH_WPA3_PSK:
//           Serial.print("WPA3");
//           break;
//         case WIFI_AUTH_WPA2_WPA3_PSK:
//           Serial.print("WPA2+WPA3");
//           break;
//         default:
//           Serial.print("Unknown");
//       }

//       // Print channel
//       Serial.print(" (Ch: ");
//       Serial.print(WiFi.channel(i));
//       Serial.println(")");

//       delay(10); // Small delay between printing each network
//     }
//   }

//   // Wait before scanning again
//   Serial.println("\nScan completed. Will scan again in 5 seconds...");
//   delay(5000);
// }


#include <WiFi.h>
#include <HardwareSerial.h>

// UART Settings
HardwareSerial SerialPort(2); // Use UART2 on ESP32
const int rxPin = 16;         // RX2 pin on ESP32 - connect to TX on STM32
const int txPin = 17;         // TX2 pin on ESP32 - connect to RX on STM32
const int baudRate = 115200;  // Set this to match your STM32 UART speed

// WiFi scan interval (milliseconds)
const int scanInterval = 10000;
unsigned long lastScanTime = 0;

void setup() {
  // Initialize both serial ports
  Serial.begin(115200);        // Debug serial port (to computer)
  SerialPort.begin(baudRate, SERIAL_8N1, rxPin, txPin); // Data port to STM32
  
  // Set WiFi to station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // Make sure we're not connected
  delay(100);
  
  Serial.println("\n----- ESP32 WiFi Scanner with UART Output -----");
  SerialPort.println("\n----- ESP32 WiFi Scanner -----"); // Send startup message to STM32
}

void loop() {
  unsigned long currentTime = millis();
  
  // Run WiFi scan at regular intervals
  if (currentTime - lastScanTime >= scanInterval) {
    scanAndSendWiFiNetworks();
    lastScanTime = currentTime;
  }
}

void scanAndSendWiFiNetworks() {
  Serial.println("\nScanning for WiFi networks...");
  SerialPort.println("\nSCAN_START"); // Marker for STM32 to recognize start of scan results
  
  // Start WiFi scan
  int networksFound = WiFi.scanNetworks();
  
  if (networksFound == 0) {
    Serial.println("No networks found!");
    SerialPort.println("NO_NETWORKS_FOUND");
  } else {
    // Send number of networks found
    Serial.printf("%d networks found:\n", networksFound);
    SerialPort.printf("NETWORKS:%d\n", networksFound);
    
    // Send details for each network
    for (int i = 0; i < networksFound; i++) {
      // Format: NETWORK:id,ssid,rssi,encryption,channel
      String encType = getEncryptionType(WiFi.encryptionType(i));
      
      // Debug output to computer
      Serial.printf("%d: %s (%d dBm) %s Ch:%d\n", 
        i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), 
        encType.c_str(), WiFi.channel(i));
      
      // Output to STM32 via UART
      SerialPort.printf("NETWORK:%d,%s,%d,%s,%d\n", 
        i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i), 
        encType.c_str(), WiFi.channel(i));
      
      delay(10); // Small delay between sending each network
    }
  }
  
  // Mark end of scan data
  SerialPort.println("SCAN_END");
  WiFi.scanDelete(); // Free memory used by scan results
  
  Serial.println("Scan results sent to STM32");
}

String getEncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA+WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "WPA2-ENT";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";
    case WIFI_AUTH_WPA2_WPA3_PSK:
      return "WPA2+WPA3";
    default:
      return "Unknown";
  }
}