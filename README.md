# esp8266_iftttAlarm

Arduino Sketch to send the status of GPIOs via the webhooks of Ifttt service.

## Getting Started

The Sketch can be tested with a LoLin NodeMCU V3.

### Prerequisites

To use this sketch the Arduino IDE (at least Ver. 1.8.9) must be installed. In addition, at least the following libraries must be downloaded and available.
- WiFi Built-In by Arduino (1.2.7)
- WiFiManager by tzapu (version 0.14.0)

### Test

After switching on, the ESP tries to log on to a known WLAN network. If this does not succeed, a soft access point with the name AlarmConnect_AP (password: AlarmConnect_AP) is started. Every three minutes a new connection to the WLAN network is attempted.
You can connect from any device to the Soft-Access Point of the ESP. If you are not automatically forwarded to the configuration page after the connection, you must enter the IP address of the ESP in a browser (http://192.168.1.1). On this page you can connect to a local network with an Internet connection.

If the ESP is registered in the local WLAN network, the ifttt secret must be entered. For this, the local IP address of the ESP must be entered in the browser (Attention! Probably other IP than described above). Either this IP is taken from the local access point (e.g. FritzBox) or the ESP outputs it to the serial interface at startup. For this, the LoLin NodeMCU must be connected to the computer (baud rate 115200) and a serial terminal (e.g. Tera Term) must be started.

The ifttt secret is entered on the now displayed page. The password is "iftttSecret". After saving, the ESP is ready for operation.
The ifttt configuration webpage also displays the status of the input pins.

If inputs D1, D2 or D5 are now connected to 3V, a message is sent to ifttt (via https). If the connection is disconnected again, a second message is sent.

Message contents when connecting the inputs:
- InternOn
- ExternOn
- AlarmOn

Message contents when removing the inputs:
- InternOff
- ExternOff
- AlarmOff

## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

Dieses Programm ist Freie Software: Sie können es unter den Bedingungen der GNU General Public License, wie von der Free Software Foundation, Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren veröffentlichten Version, weiter verteilen und/oder modifizieren.

Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch OHNE JEDE GEWÄHR,; sogar ohne die implizite Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK. Siehe die GNU General Public License für weitere Einzelheiten.

Sie sollten eine Kopie der GNU General Public License zusammen mit diesem Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.


