#include <HardwareSerial.h>

// UART Settings
HardwareSerial SerialPort(2); // Use UART2 on ESP32
const int rxPin = 16;         // RX2 pin on ESP32 - connect to TX on STM32
const int txPin = 17;         // TX2 pin on ESP32 - connect to RX on STM32
const int baudRate = 115200;  // Match STM32 UART baudrate

// Message variables
unsigned long lastSendTime = 0;
const int sendInterval = 2000; // Send message every 2 seconds
int messageCount = 0;

void setup() {
  // Initialize both serial ports
  Serial.begin(115200);        // Debug serial port (to computer)
  SerialPort.begin(baudRate, SERIAL_8N1, rxPin, txPin); // Data port to STM32
  
  Serial.println("ESP32 UART Communication Test");
  
  // Initial handshake message
  SerialPort.println("ESP32_READY");
}

void loop() {
  // Check for incoming messages from STM32
  if (SerialPort.available()) {
    String message = SerialPort.readStringUntil('\n');
    message.trim(); // Remove any whitespace or newline characters
    
    // Print to debug serial
    Serial.print("Received from STM32: ");
    Serial.println(message);
    
    // Respond to handshake
    if (message == "STM32_READY") {
      SerialPort.println("ESP32_ACK");
    }
    
    // Echo any other message back with a prefix
    else {
      SerialPort.print("ESP32_ECHO: ");
      SerialPort.println(message);
    }
  }
  
  // Send periodic messages
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    messageCount++;
    
    // Send a simple message with counter
    String message = "ESP32_MSG_" + String(messageCount);
    SerialPort.println(message);
    Serial.print("Sent to STM32: ");
    Serial.println(message);
    
    lastSendTime = currentTime;
  }
}