/*//WORK- on 3.3V and 5.0V!
http://arduino.stackexchange.com/questions/18575/send-at-commands-to-esp8266-from-arduino-uno-via-a-softwareserial-port
 Name:		ATcomands.ino
 Created:	7/21/2016 5:00:06 PM
 Author:	nasko
*/
#include <SPI.h>
#include <SoftwareSerial.h>

SoftwareSerial esp8266(2, 3); //from pin 2 to pin TX on ESP8266 // from pin 3 to pin RX on ESP8266

void setup() {
	// Open serial communications and wait for port to open:
	Serial.begin(115200);
	while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	}

	Serial.println("Started");

	// set the data rate for the SoftwareSerial port
	esp8266.begin(115200);
	esp8266.write("AT\r\n");
}

void loop() {
	if (esp8266.available()) {
		Serial.write(esp8266.read());
	}
	if (Serial.available()) {
		esp8266.write(Serial.read());
	}
}