/* work! send data to web every 39 sec
field 3 send info how many times send info to web 

original from http://www.instructables.com/id/Monitor-Temperature-and-Humidity-Value-From-Your-W/step2/Software-setup/
 Name:		DHT22ToWeb.ino
 Created:	7/28/2016 10:52:35 AM
 Author:	nasko
*/

#include <SoftwareSerial.h>
#include <DHT.h>
#define DHTPIN 10     // what pin we're connected to pi n 10
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); //// Initializ

SoftwareSerial wifiConnection(2, 3);//from pin 2 on UNO to pin TX on ESP8266 // from pin 3 on UNO to pin RX on ESP8266
const String wifiSSID = "@@@@@@@";// this will be change depend local WIFI network
const String wifiPassword = "*********";// this will be change depend local WIFI network



void setup() {
	Serial.begin(115200);
	wifiConnection.begin(115200);
	delay(1000);

	dht.begin();// TO DO test without this row

	if (!connected()) 
		{
			Serial.println("Unable to connect to the internet");
		}
	else 
		{
			Serial.println("The ESP8266 was connected to the Internet");
		}

}

//int chk; this is check sum for error in DHT22. 

float hum;  //Stores humidity value
float temp;
int diffnumber = 0;//work this is counter

void loop(){
	Serial.println("Start of the loop"); //
	hum = dht.readHumidity();//Read data and store it to variables hum and temp
	temp = dht.readTemperature();

	Serial.print("Humidity: ");//Print temp and humidity values to serial monitor
	Serial.print(hum,1);
	Serial.print(" %, Temperature: ");
	Serial.print(temp,1);
	Serial.println(" Celsius");
	delay(2000); //Delay 2 sec.

	int  iHum = (int)hum;//convert to integer
	send(temp, iHum);//the original was "send(tempValue, i);"
	delay(30000); /* 30 sec delay */

}


void send(float temperature, int number) {

	String request;
	/* data = "field1=100&field2=30"; sample data */

	
	diffnumber += 1;// add 1 every time when the code loop here

	char buf[16];
	String data1 = dtostrf(temperature, 4, 1, buf);// for this command check http://www.hobbytronics.co.uk/arduino-float-vars
	String data2 = String(number);

	String data3 = String(diffnumber); 
	Serial.println("diffnumberis: " + diffnumber);


	Serial.println("number is "+ number);//del after check what do this
	Serial.println(data1 + " - >data1");
	Serial.println(data2 + " - >data2");
	// this is original and work String data = "field1=" + data1 + "&field2=" + data2;
	String data = "field1=" + data1 + "&field2=" + data2 + "&field3=" + data3;// look like work- send Zero to thingspeak
	request = "POST /update HTTP/1.0\r\nConnection: close\r\nHost: api.thingspeak.com\r\nX-THINGSPEAKAPIKEY: LEZHYIP19H5Q631W\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: " + String(data.length()) + "\r\n\r\n";
												/*serial returned:      	Update HTTP/1.0
																			Connection: close
																			Host : api.thingspeak.com
																			X - THINGSPEAKAPIKEY : LEZHYIP19H5Q631W
																			Content - Type : application / x - www - form - urlencoded
																			Content - Length :
																			sent a connection request
																			HTTP sent a request
																			Connection was founded
																			Incoming data :
																			*/
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
	wifiConnection.print(AtComand);	// 

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

	Serial.println("Status is: " + status); // may be print the IP addres like a string. If work and if want Remove

	if (status.indexOf("FAIL") != -1) { // //connection success status has been tested. 
		return false;
	}
	return true;
}
