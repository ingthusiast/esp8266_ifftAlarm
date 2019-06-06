#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>

ESP8266WebServer server(80);

WiFiClientSecure client;

// conditional compilation
//#define SOFT_AP_DEBUG_MODE
#define DEBUG_MESSAGES

// defines to config device
#define iftttEEPROMAddress 0

// constants won't change.
const int httpsPort = 443;
const char* iftttServer = "maker.ifttt.com";

const String preStatement = "/trigger/";
const String postStatement = "/with/key/";

const String msgInternallyActivated = "InternOn";
const String msgInternallyDeactivated = "InternOff";

const String msgExternallyActivated = "ExternOn";
const String msgExternallyDeactivated = "ExternOff";

const String msgAlarmActivated = "AlarmOn";
const String msgAlarmDeactivated = "AlarmOff";

const int internalStatePin = 5;   // the number of the State_internal pin
const int externalStatePin = 4;   // the number of the State_external pin
const int alarmStatePin = 14;     // the number of the State_alarm pin
const int ledPin =  2;            // the number of the LED pin

// variables will change:
String iftttSecret = "no_secret_yet";
String msg;

int State_internal = 0;         // variable for reading the State_internal status
int State_external = 0;         // variable for reading the State_external status
int State_alarm = 0;            // variable for reading the State_alarm status

int State_internal_old = 0;     // variable for the previous State_internal status
int State_external_old = 0;     // variable for the previous State_external status
int State_alarm_old = 0;        // variable for the previous State_alarm status

bool sendIftttMsg = false;

//===============================================================
// This is the template for the ifttt config page
//===============================================================
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<h2>Ifttt Alarm Connect</h2>
<form action="/action_page">
ifttt secret:<br>
<input type="text" name="secret" style="width: 350px;" value="iftttSecret">
<br>
password:<br>
<input type="text" name="pwd" style="width: 350px;" value="">
<br><br>
<input type="submit" value="Submit">
</form>
<br><h3>input state</h3>
<table border="black" cellpadding="5">
<tr><th>internal</th><th>external</th><th>alarm</th></tr>
<tr><td>ims</td><td>ems</td><td>ais</td></tr>
<tr><td colspan="3" align="center"><a href="javascript:history.go(0)">reload input states</a></td></tr></table>
</body>
</html>
)=====";

//===============================================================
// This routine writes a string to EEPROM memory
//===============================================================
void writeString(char add, String data)
{
	int _size = data.length();
	int i;
	
	for(i=0;i<_size;i++)
	{
		EEPROM.write(add+i,data[i]);
	}
	
	EEPROM.write(add+_size,'\0');   // Add termination null character for String Data
	
	EEPROM.commit();
}

//===============================================================
// This routine reads a string out of EEPROM memory
//===============================================================
String read_String(char add)
{
	int i;
	char data[100]; // Max 100 Bytes
	int len=0;
	unsigned char k;
	
	k=EEPROM.read(add);
	
	while(k != '\0' && len<500)   // Read until null character
	{    
		k=EEPROM.read(add+len);
		data[len]=k;
		len++;
	}
	
	data[len]='\0';
	
	return String(data);
}

//===============================================================
// This routine is the main setup reoutine
//===============================================================
void setup()
{
	Serial.begin(115200);
	EEPROM.begin(512);

	Serial.println("These are the messages for ifttt service:");
	Serial.print(msgInternallyActivated);
	Serial.print(" - ");
	Serial.print(msgInternallyDeactivated);
	Serial.print(" - ");
	Serial.print(msgExternallyActivated);
	Serial.print(" - ");
	Serial.print(msgExternallyDeactivated);
	Serial.print(" - ");
	Serial.print(msgAlarmActivated);
	Serial.print(" - ");
	Serial.println(msgAlarmDeactivated);

	WiFiManager wifiManager;

	// do something when the device enters configuration mode on failed WiFi connection
	wifiManager.setAPCallback(configModeCallback);

	// config portal timeout
	wifiManager.setConfigPortalTimeout(180);

	// set custom ip for portal
	wifiManager.setAPStaticIPConfig(IPAddress(192,168,1,1), IPAddress(192,168,1,1), IPAddress(255,255,255,0));

	// tries to connect to last known settings
	// if it does not connect it starts an access point with the specified name
	// here  "AlarmConnect_AP" with password "AlarmConnect_AP"
	// and goes into a blocking loop awaiting configuration
	// first parameter is name of access point, second is the password
	if (!wifiManager.autoConnect("AlarmConnect_AP", "AlarmConnect_AP"))
	{
		Serial.println("failed to connect, we should reset as see if it connects");
		delay(3000);
		ESP.reset();
		delay(5000);
	}

	// if you get here you have connected to the WiFi
	Serial.println("connected");

	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	// get ifttt secret out of eeprom
	iftttSecret = read_String(iftttEEPROMAddress);
	delay(1000);
	Serial.print("restored EEPROM Content: ");
	Serial.println(iftttSecret);

	setup_webserver();

	// initialize digital pin LED_BUILTIN as an output.
	pinMode(ledPin, OUTPUT);

	// initialize digital input pinns
	pinMode(internalStatePin, INPUT);
	pinMode(externalStatePin, INPUT);
	pinMode(alarmStatePin, INPUT);
	
#ifdef SOFT_AP_DEBUG_MODE	
	int test_timeout = 0;
	while(1)
	{
		Serial.println(".");
		delay(1000);
		test_timeout++;

		if(test_timeout > 30)
		{
			Serial.println("TEST TIMEOUT -> RESET");
			//reset settings - for testing
			wifiManager.resetSettings();
			while(1)
			{
				delay(5000);
				Serial.println("reset device....!");
				ESP.reset();
			}
		}
	}
#endif

}

//===============================================================
// This routine is called when SoftAP is started
//===============================================================
void configModeCallback (WiFiManager *myWiFiManager)
{
	Serial.println("Entered config mode");
	Serial.println(WiFi.softAPIP());

	Serial.println(myWiFiManager->getConfigPortalSSID());
	
}


//===============================================================
// This routine sets up the webserver
//===============================================================
void setup_webserver(void)
{
	server.on("/", handleRoot);      // Which routine to handle at root location
	server.on("/action_page", handleForm); // form action is handled here

	server.begin();                  // Start server
	Serial.println("HTTP server started");

}

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot()
{
	String s = MAIN_page; // Read HTML contents
	s.replace("iftttSecret", iftttSecret);
	
	if (State_alarm == HIGH)
	{
		s.replace("ais", "HIGH");
	} else
	{
		s.replace("ais", "LOW");
	}
	
	if (State_internal == HIGH)
	{
		s.replace("ims", "HIGH");
	} else
	{
		s.replace("ims", "LOW");
	}
	
	if (State_external == HIGH)
	{
		s.replace("ems", "HIGH");
	} else
	{
		s.replace("ems", "LOW");
	}
		
	server.send(200, "text/html", s); // Send web page
}

//===============================================================
// This routine is executed when you press submit
//===============================================================
void handleForm()
{
	String s = "Ifttt Secret Taken Over! <a href='/'> Go Back </a>";

	String pwd = server.arg("pwd");

	String secret = server.arg("secret");

	if (pwd == "iftttSecret")
	{
		iftttSecret = secret;
		writeString(iftttEEPROMAddress, iftttSecret);
		delay(10);

		Serial.print("new ifttt secret:");
		Serial.println(iftttSecret);
	} else
	{
		Serial.println("Incorrect password -> change not adopted");
		s = "Incorrect password! <a href='/'> Go Back </a>";
	}

	server.send(200, "text/html", s); // Send web page
}

//===============================================================
// This routine makes an HTTP request to the IFTTT web service
//===============================================================
void makeIFTTTRequest()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("WiFi not connected! -> reconnect");

		while (WiFi.status() != WL_CONNECTED)
		{
			delay(500);
			Serial.print(".");
		}
	}

	Serial.print("Connecting to "); 
	Serial.print(iftttServer);

	int retries = 5;

	while(!!!client.connect(iftttServer, httpsPort) && (retries-- > 0))
	{
		Serial.print(".");
	}

	Serial.println();

	if(!!!client.connected())
	{
		Serial.println("Failed to connect, going back to sleep");
	}

	Serial.print("Request resource: "); 

	Serial.println(msg);

	client.print(String("GET ") + msg + 
						" HTTP/1.1\r\n" +
						"Host: " + iftttServer + "\r\n" + 
						"Connection: close\r\n\r\n");

	int timeout = 5 * 10; // 5 seconds

	while(!!!client.available() && (timeout-- > 0))
	{
		delay(100);
	}

	if(!!!client.available())
	{
		Serial.println("No response, going back to sleep");
	}

	while(client.available())
	{
		Serial.write(client.read());
	}

	Serial.println("\nclosing connection");
	client.stop();
}

//===============================================================
// This is the main loop
//===============================================================
void loop()
{
	// read the state of inputs:
	State_internal = digitalRead(internalStatePin);
	State_external = digitalRead(externalStatePin);
	State_alarm = digitalRead(alarmStatePin);

	// turn LED off:
	digitalWrite(ledPin, HIGH);

	// check if an alarm has been triggered
	if (State_alarm != State_alarm_old)
	{
		if (State_alarm == HIGH)
		{
			msg = preStatement + msgAlarmActivated + postStatement + iftttSecret;
		} else
		{
			msg = preStatement + msgAlarmDeactivated + postStatement + iftttSecret;
		}

		sendIftttMsg = true;
	} else
	{
	// as long as no alarm has been triggered, query the other states
	if (State_external != State_external_old)
	{
		if (State_external == HIGH)
		{
			msg = preStatement + msgExternallyActivated + postStatement + iftttSecret;
		} else
		{
			msg = preStatement + msgExternallyDeactivated + postStatement + iftttSecret;
		}

		sendIftttMsg = true;

		} else if (State_internal != State_internal_old)
		{
			if (State_internal == HIGH)
			{
				msg = preStatement + msgInternallyActivated + postStatement + iftttSecret;
			} else
			{
				msg = preStatement + msgInternallyDeactivated + postStatement + iftttSecret;
			}

			sendIftttMsg = true;
		}
	}
	
#ifdef DEBUG_MESSAGES
	Serial.print(State_internal);
	Serial.print(State_external);
	Serial.println(State_alarm);
#endif

	if (sendIftttMsg == true)
	{
		// turn LED on
		digitalWrite(ledPin, LOW);

		makeIFTTTRequest();

		sendIftttMsg = false;
	} else
	{
		// turn LED on
		digitalWrite(ledPin, LOW);

		delay(1);

		// turn LED off
		digitalWrite(ledPin, HIGH);
	}

	State_internal_old = State_internal;
	State_external_old = State_external;
	State_alarm_old = State_alarm;

	server.handleClient();          // Handle client requests

	delay(1000);
}
