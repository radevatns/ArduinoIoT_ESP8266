/*  measurement temp via LM35DZ and send data to thingspeak.

Name:		TempLM35toWeb.ino
Created:	8/3/2016 11:51:30 AM
Author:	nasko

Work with good accurancy. External Power supplay was obligatory!!!
Scematics was:
Arduino		VoltageRegulator	LM35	ESP8266	  capacitor 1uF
5V-------!-V in-------------!---V in--!----------!-------------
GND -----!-GND--------------!---------!-GND------!-------------
---------!-Vout----3.3V-----!---------!-Vcc------!-------------
---------!-Vout----3.3V-----!---------!-RST------!-------------
---------!-Vout----3.3V-----!---------!-CHPD-----!-------------
GND------!------------------!---GND---!----------!--+(-)-------
D3-------!------------------!---------!-TXD------!-------------
D4-------!------------------!---------!-RXD------!-------------
A2-------!------------------!---DAT---!----------!--+(-)-------
			
After put the capacitor 1uF between GND and signal on LM35Dz acurancy better. After put the code for low pass filter -everything was OK

*/

#include <SoftwareSerial.h>
float temperature;
int sensorValue;
int prevSensorValue;
float avrSensorValue = 0;//change int to float *** first place

SoftwareSerial wifiConnection(2, 3);//from pin 2 on UNO to pin TX on ESP8266 // from pin 3 on UNO to pin RX on ESP8266
const String wifiSSID = "@@@@@@@@^";// this will be change depend local WIFI network//Ged Office1
const String wifiPassword = "********";// this will be change depend local WIFI network//gedclients

void setup() {
	analogReference(INTERNAL);
	

	Serial.begin(115200);
	wifiConnection.begin(115200);
	delay(1000);

	if (!connected())
	{
		Serial.println("Unable to connect to the Internet");
	}
	else
	{
		Serial.println("The ESP8266 was connected to the Internet");
	}
}

void loop() {
	
	sensorValue = analogRead(A2);
	delay(1000);

	//Serial.print("sensorValue is: ");
	//Serial.println(sensorValue);
	
	prevSensorValue = avrSensorValue;//this is Low pass filter
	if (prevSensorValue == 0)//this is only for start measurement. If ==0 without this the measurement first 30min is wrong
	{
		prevSensorValue = analogRead(A2);
		sensorValue = prevSensorValue;// this row is because after start first reading of sensorValue is upper than normal.
		//Serial.print("sec analogRead is: ");
		//Serial.println(prevSensorValue);
		//Serial.println("in the if = 0");

	}

	avrSensorValue = (0.1 * sensorValue) + (0.9*prevSensorValue);// calculation for low pass filter
	delay(1000);
	
	temperature = (1.076 * avrSensorValue / 1023) * 100;   //temp = (5.0 * sensorValue * 100.0) / 1024;
	Serial.print("Avarage Row data from sensor is: ");
	Serial.println(avrSensorValue);
	Serial.print("Row data from sensor is: ");
	Serial.println(sensorValue);
	Serial.print("temperature in the room is: ");
	Serial.println(temperature,1);

	send(temperature, avrSensorValue);//first is float second was int. If change the positions temp is int in thingspeak
	delay(30000);
}
//void send(float sendTemperature, int sendSensorValue) { *** second place
void send(float sendTemperature, float sendSensorValue) {
	String request;
	char buf[16];	// why is 16. With [6] work too
	String data1 = dtostrf(sendTemperature, 4, 1, buf);// with floating point
	String data2 = String(sendSensorValue);// without floating point***if change  first+second place work with float
	String data = "field1=" + data1 + "&field2=" + data2;
	request = "POST /update HTTP/1.0\r\nConnection: close\r\nHost: api.thingspeak.com\r\nX-THINGSPEAKAPIKEY: LEZHYIP19H5Q631W\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: " + String(data.length()) + "\r\n\r\n";

	wifiConnection.print("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");
	wifiConnection.find("OK");

	Serial.println("sent a WiFi connection request");
	delay(1000);
	wifiConnection.print("AT+CIPSEND=");
	wifiConnection.find(">");
	wifiConnection.println(String(request.length() + data.length()));
	Serial.println("HTTP sent a request");
	delay(1000);

	wifiConnection.print(request);
	wifiConnection.print(data);
	wifiConnection.find("SEND OK");
	Serial.println("Connection was OK");//this not work correct because after dissconect the power of ESP this row was printed

	String receivedData = "";//getting data
	char k;
	long time = millis();


	while (receivedData.indexOf("CLOSED") == -1) {
		if (millis() - time > 3000) {
			break;
		}
		while (wifiConnection.available())
		{
			k = wifiConnection.read();// incoming datas from the module is recording/saving
			receivedData += k;
		}
	}
	Serial.println("Incoming data:");
	Serial.println(receivedData); //after disconnect the ESP cannot receive data.the "...+IPD,678:HTTP/1.1 200 O..." is missing
}
String sendAtComands(String AtComand, int timeOut, boolean debug)//this is string to send the AT comands.
{
	wifiConnection.print(AtComand);

	Serial.println(AtComand); // To Do check what is Atcomand and write here coment
	long int time = millis();

	String reply = ""; //answer
	while ((time + timeOut) > millis()) // wait 
	{
		while (wifiConnection.available())
		{
			char k = wifiConnection.read(); //the data coming from the module is saving now
			reply += k;
		}
		if (reply.indexOf("OK") != -1)
			break;
	}
	if (debug)
		Serial.print(reply); // if debug true -> print the replay // the values from the module is coming in order to put to the display's screen.
	return reply;
}

boolean connected() {
	sendAtComands("AT+CWMODE=1\r\n", 1000, true);  // try to connect with mode1. 1= Station, 2= AcessPoint, 3=both, 1 is the default mode of router, AP is a normal mode for devices
	sendAtComands("AT+CWJAP=\"" + wifiSSID + "\",\"" + wifiPassword + "\"\r\n", 10000, true); // Join Access Point
	String status = "";// I dont know what is this command row
	status = sendAtComands("AT+CIFSR\r\n", 8000, true); // Check IP address and connection information

	if (status.indexOf("FAIL") != -1) { //connection success status has been tested. 
		return false;
	}
	return true;
}
