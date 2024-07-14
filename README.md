# UnoModem

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Arduino Uno Wifi 2 emulate SIM7000A LTE modem.

Copyright (c) 2024 Daxbot

Spawns a REPL on serial port 0, allowing you to interact with the modem using AT commands.
Communicates with the client on serial port 1.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Installation

Burn onto a Arduino Uno Wifi 2 board.  Connect Serial 1 to the device that is expecting a SIM7000A LTE modem.  If desired, connect the USB port to a serial console to interact with the modem.  Apply power.

Device TX pin -> Arduino RX (pin 0)
Device RX pin -> Arduino TX (pin 1)
Device GND -> Arduino GND (pin 14)


## Usage

You will need to set the SSID and password for your wifi network by connecting to one of the Arduino serial ports.  Once set and saved, the board will connect to the network on boot and every 30 seconds thereafter if disconnected.

1. Connect to the primary Arduino serial port (9600 baud).
2. Send the following commands:
    ```
    ssid your_ssid  
    pass your_password
    save
    reboot
    ```
3. Alternatively, you can use the following AT commands over Serial1 (115200 baud):
    ```
    AT+CWJAP="your_ssid","your_password"
    AT+SAVE
    AT+RST
    ```
4. Once connected to the network, the LED will change from a slow blink to steady on and you will now be able to use `AT+CIPSTART`, `AT+CIPCLOSE`, etc to conduct your business.  Since it is emulating a SIM7000A, you can use the same AT commands you would with a real SIM7000A.  LTE is not supported, but it will play along with some LTE style commands (`AT+CIICR`, etc) without complaint.

## Contributing
Hit me up with what you want to do.  I'm open to suggestions.

## License
 
This project is licensed under the [MIT License](LICENSE).
