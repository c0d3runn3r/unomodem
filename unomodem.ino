#include <WiFiNINA.h>
#include <EEPROM.h>
#include <SPI.h>

const int MAX_SSID_LENGTH = 32;
const int MAX_PASSWORD_LENGTH = 32;
const unsigned long WIFI_RECONNECT_INTERVAL = 30000;

char ssid[MAX_SSID_LENGTH + 1] = "";
char password[MAX_PASSWORD_LENGTH + 1] = "";

WiFiClient wifiClient;

void connectToWiFi();
void handleATCommand(String command);
void rebootDevice();


  void rebootDevice() {
    Serial.println(F("Rebooting..."));
    asm volatile ("  jmp 0");
  }


// REPL class to handle input and commands
class REPL {
public:
  void begin() {
    Serial.println(F("*********************************"));
    Serial.println(F("*           UnoModem            *"));
	Serial.println(F("*                               *"));
    Serial.println(F("*      Author: c0d3runn3r		  *"));
    Serial.println(F("* Copyright (c) 2024 DaxBot Inc *"));
    Serial.println(F("*********************************"));
    Serial.println();
    Serial.println(F("Welcome to the UnoModem REPL."));
    Serial.println(F("Type 'help' for a list of commands."));
  }

  void handleInput() {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n') {
        commandBuffer[commandIndex] = '\0'; // Null-terminate the string
        processCommand();
        commandIndex = 0; // Reset buffer index
      } else if (commandIndex < sizeof(commandBuffer) - 1) {
        commandBuffer[commandIndex++] = c; // Add char to buffer
      }
    }
  }

private:
  char commandBuffer[64];
  int commandIndex = 0;

  void processCommand() {
    Serial.print(F("> "));
    Serial.println(commandBuffer);

    String command = String(commandBuffer);
    if (command == "?" || command == "help") {
      printHelp();
    } else if (command.startsWith("ssid")) {
      handleSSIDCommand(command);
    } else if (command.startsWith("password")) {
      handlePasswordCommand(command);
    } else if (command == "save") {
      saveConfig();
      Serial.println(F("Configuration saved."));
    } else if (command == "config") {
      printConfig();
    } else if (command == "reboot") {
      rebootDevice();
	} else if (command == "connect") {

		// Connect to WiFi
		connectToWiFi();		

	} else if (command == "status") {

		// Print WiFi status
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println(F("WiFi connected"));

		} else {
			Serial.println(F("WiFi not connected"));
		}


    } else {
      Serial.println(F("Unknown command."));
    }
  }

  void printHelp() {
    Serial.println(F("Available commands:"));
    Serial.println(F("  ? or help       - Show this help message"));
    Serial.println(F("  ssid <value>    - Set or get the WiFi SSID"));
    Serial.println(F("  password <value>- Set or get the WiFi password"));
    Serial.println(F("  save            - Save the configuration"));
    Serial.println(F("  config          - Show current configuration"));
    Serial.println(F("  reboot          - Reboot the device"));
	Serial.println(F("  connect         - Connect to WiFi"));
	Serial.println(F("  status          - Show WiFi status"));
  }

  void handleSSIDCommand(String command) {
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex > 0) {
      String newSSID = command.substring(spaceIndex + 1);
      newSSID.toCharArray(ssid, MAX_SSID_LENGTH + 1);
      Serial.print(F("SSID set to: "));
      Serial.println(ssid);
    } else {
      Serial.print(F("Current SSID: "));
      Serial.println(ssid);
    }
  }

  void handlePasswordCommand(String command) {
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex > 0) {
      String newPassword = command.substring(spaceIndex + 1);
      newPassword.toCharArray(password, MAX_PASSWORD_LENGTH + 1);
      Serial.print(F("Password set to: "));
      Serial.println(password);
    } else {
      Serial.print(F("Current Password: "));
      Serial.println(password);
    }
  }

  void saveConfig() {
    EEPROM.put(0, ssid);
    EEPROM.put(MAX_SSID_LENGTH + 1, password);
	Serial.println(F("Configuration saved to EEPROM"));
    //EEPROM.commit(); // Not needed for this board
  }

  void printConfig() {
    Serial.print(F("SSID: "));
    Serial.println(ssid);
    Serial.print(F("Password: "));
    Serial.println(password);
  }

//   void rebootDevice() {
//     Serial.println(F("Rebooting..."));
//     asm volatile ("  jmp 0");
//   }
};

REPL repl;

void setup() {

	// Set up serial ports
	Serial.begin(9600);
	Serial1.begin(115200);

	// Initialize config from EEPROM
	EEPROM.get(0, ssid);
	EEPROM.get(MAX_SSID_LENGTH + 1, password);

	// If we don't have a configuration, print a message
	if (ssid[0] == '\0' || password[0] == '\0') {
		
		Serial.println(F("No configuration found in EEPROM"));
	
	} else {

		// We do have a config. Print it out
		Serial.println(F("Configuration loaded from EEPROM"));
		Serial.print(F("SSID: "));
		Serial.println(ssid);
		Serial.print(F("Password: "));
		Serial.println(password);

		// Initialize WiFi
		connectToWiFi();
	}

  repl.begin();

}

void loop() {

	// Check elapsed time and reconnect to WiFi if necessary
	static unsigned long previousMillis = 0;
	unsigned long elapsedMillis = millis() - previousMillis;

	if (elapsedMillis >= WIFI_RECONNECT_INTERVAL) {
		previousMillis = millis();
		if (WiFi.status() != WL_CONNECTED) {
			Serial.println(F("WiFi connection lost. Reconnecting..."));
			connectToWiFi();
		}
	}

  repl.handleInput();

  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();
    Serial.print(F("Received AT command: "));
    Serial.println(command);
    handleATCommand(command);
  }

  if (wifiClient.connected() && wifiClient.available()) {
    while (wifiClient.available()) {
      char c = wifiClient.read();
      Serial1.write(c);
    }
  }





}

void connectToWiFi() {

	// If we have no SSID, we can't connect
	if (ssid[0] == '\0' || password[0] == '\0') {
		Serial.println(F("No SSID or password set. Cannot connect to WiFi"));
		Serial1.println(F("ERROR"));
		return;
	}

	// Print connecting message
	Serial.print(F("Connecting to WiFi (10s timeout, SSID '"));
	Serial.print(ssid);
	Serial.print(F("', password '"));
	Serial.print(password);
	Serial.println(F("')..."));

	WiFi.setTimeout(10000);
  if (WiFi.begin(ssid, password) != WL_CONNECTED) {
    
	Serial.println(F("WiFi connection failed"));
    Serial1.println(F("ERROR"));
  
  } else {

	Serial.println(F("Connected to WiFi"));
	Serial1.println(F("OK"));
	Serial.println(F("Sent response: OK"));
  }

}

void handleATCommand(String command) {
  if (command == "AT") {
    Serial1.println(F("OK"));
    Serial.println(F("Sent response: OK"));

	// AT+CWJAP sets the SSID and password
  } else if (command.startsWith("AT+CWJAP") {

	// Parse the SSID and password parameters
	int firstQuote = command.indexOf('"');
	int secondQuote = command.indexOf('"', firstQuote + 1);
	String newSSID = command.substring(firstQuote + 1, secondQuote);
	int thirdQuote = command.indexOf('"', secondQuote + 1);
	int fourthQuote = command.indexOf('"', thirdQuote + 1);
	String newPassword = command.substring(thirdQuote + 1, fourthQuote);

	// Set the new SSID and password
	newSSID.toCharArray(ssid, MAX_SSID_LENGTH + 1);
	newPassword.toCharArray(password, MAX_PASSWORD_LENGTH + 1);

	Serial1.println(F("OK"));
	Serial.println(F("Sent response: OK"));

  } else if (command == "AT+SAVE") {

	// Save the configuration to EEPROM
	saveConfig();
	Serial1.println(F("OK"));
	Serial.println(F("Sent response: OK"));
 
  } else if (command == "AT+RST") {
    Serial1.println(F("OK"));
    Serial.println(F("Sent response: OK"));
    rebootDevice();
  } else if (command == "AT+CSQ") {
    int rssi = WiFi.RSSI();
    Serial1.print(F("+CSQ: "));
    Serial1.println(rssi);
    Serial.print(F("Sent response: +CSQ: "));
    Serial.println(rssi);
  } else if (command == "AT+CGATT") {
    Serial1.println(F("OK"));
    Serial.println(F("Sent response: OK"));
  } else if (command == "AT+SAPBR") {
    Serial1.println(F("OK"));
    Serial.println(F("Sent response: OK"));
  } else if (command == "AT+CGACT") {
    Serial1.println(F("OK"));
    Serial.println(F("Sent response: OK"));
  } else if (command == "AT+CIPSTATUS") {
    if (WiFi.status() == WL_CONNECTED) {
      Serial1.println(F("STATE: CONNECT OK"));
    } else {
      Serial1.println(F("STATE: IP INITIAL"));
    }
    Serial.println(F("Sent response: CIPSTATUS"));
  } else if (command == "AT+CIICR") {
    Serial1.println(F("OK"));
    Serial.println(F("Sent response: OK"));
  } else if (command == "AT+CIPCLOSE") {
    wifiClient.stop();
    Serial1.println(F("CLOSE OK"));
    Serial.println(F("Sent response: CLOSE OK"));
  } else if (command == "AT+CIFSR") {
    Serial1.println(WiFi.localIP());
    Serial.print(F("Sent response: "));
    Serial.println(WiFi.localIP());
  } else if (command.startsWith("AT+CIPSTART")) {
    int firstComma = command.indexOf(',');
    int secondComma = command.indexOf(',', firstComma + 1);
    String protocol = command.substring(firstComma + 2, firstComma + 5);
    String server = command.substring(firstComma + 6, secondComma - 1);
    int port = command.substring(secondComma + 1).toInt();

    Serial.print(F("Attempting to connect to "));
    Serial.print(server);
    Serial.print(F(":"));
    Serial.println(port);

    if (wifiClient.connect(server.c_str(), port)) {
      Serial1.println(F("OK"));
      Serial.println(F("Sent response: OK"));
    } else {
      Serial1.println(F("ERROR"));
      Serial.println(F("Sent response: ERROR"));
    }
  } else if (command.startsWith("AT+CIPSEND")) {
    Serial1.println(F(">"));
    Serial.println(F("Sent response: >"));
    while (!Serial1.available());
    String data = Serial1.readStringUntil('\n');
    data.trim();
    wifiClient.print(data);
    Serial1.println(F("SEND OK"));
    Serial.println(F("Sent response: SEND OK"));
  } else if (command.startsWith("AT+CIPCLOSE")) {
    wifiClient.stop();
    Serial1.println(F("CLOSE OK"));
    Serial.println(F("Sent response: CLOSE OK"));
  } else {
    Serial1.println(F("ERROR"));
    Serial.println(F("Sent response: ERROR"));
  }
}
