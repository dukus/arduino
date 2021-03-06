
int testWifi(void) {
	int c = 0;
	Serial.println("Waiting for Wifi to connect");
	while (c < 20) {
		if (WiFi.status() == WL_CONNECTED) { return(20); }
		delay(500);
		//Serial.print(WiFi.status());
		c++;
		Serial.print(".");
	}
	Serial.println("Connect timed out, opening AP");
	return(10);
}

void handleRoot() {
	int sec = millis() / 1000;
	int min = sec / 60;
	int hr = min / 60;

	IPAddress ip = WiFi.softAPIP();
	String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
	String s = "<html>";
	s += "<head><title>Configure your device</title>";
	s += "<script type = \"text/javascript\">";
	s += "function httpGet(theUrl)";
	s += "{";
	s += "var xmlHttp = null;";
	s += "	xmlHttp = new XMLHttpRequest();";
	s += "	xmlHttp.open(\"GET\", theUrl, false);";
	s += "	xmlHttp.send(null);";
	s += "	return xmlHttp.responseText;";
	s += "	}";
	s += "</script></head>";
	s += "<body> Hello from ESP8266 at ";
	s += ipStr;
	s += "<p>";
	s += "<form method='get' action='a'><label>SSID: </label><select name='ssid' length=32>" + st + " </select> <br><br>";
	s += "<label>PASS: </label><input name='pass' length=32 value=\"" + String(settings.pass) + "\"><br><br>";
	s += "<label>MQTT server: </label><input name='mqtt' length=32 value=\"" + String(settings.mqttServer) + "\"><br><br>";
	s += "<label>Client Id: </label><input name='clientid' length=32 value=\""+String(settings.clientId)+"\"><br><br><input type='submit'></form>";
	s += "<p>";
	s += "<a href=\"/restart\">Restart device</a>";
	s += "<HR>";
	s += "<button  onclick=\"httpGet('/relay1on')\">Relay 1 On</button><br><br>";
	s += "<button  onclick=\"httpGet('/relay1off')\">Relay 1 Off</button><br><br>";
	s += "<button  onclick=\"httpGet('/relay2on')\">Relay 2 On</button><br><br>";
	s += "<button  onclick=\"httpGet('/relay2off')\">Relay 2 Off</button><br><br>";
	s += "<HR>";
	s += "Last error :"+last_error;
	s += "</body>";
	s += "</html>\r\n\r\n";
	server.send(200, "text/html", s);
}


void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += (server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for (uint8_t i = 0; i < server.args(); i++) {
		message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
	}

	server.send(404, "text/plain", message);
}

void launchWeb()
{
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();
	delay(100);
	int n = WiFi.scanNetworks();
	Serial.println("scan done");
	if (n == 0)
		Serial.println("no networks found");
	else
	{
		Serial.print(n);
		Serial.println(" networks found");
		for (int i = 0; i < n; ++i)
		{
			// Print SSID and RSSI for each network found
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
			delay(10);
		}
	}
	Serial.println("");
	st = "";
	for (int i = 0; i < n; ++i)
	{
		// Print SSID and RSSI for each network found
		st += "<option value=\"";
		st += WiFi.SSID(i);
		st += "\">";
		st += i + 1;
		st += ": ";
		st += WiFi.SSID(i);
		st += " (";
		st += WiFi.RSSI(i);
		st += ")";
		st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
		st += "</option>";
	}
	delay(100);
	char charBuf[50];

	getMac().toCharArray(charBuf, 50);
	strcat(charBuf, " - ");
	strcat(charBuf, settings.clientId);

	WiFi.softAP(charBuf);
	Serial.print("IP address: ");
	Serial.println(WiFi.softAPIP());

	if (MDNS.begin("esp8266")) {
		Serial.println("MDNS responder started");
	}

	server.on("/relay1on", Relay1On);
	server.on("/relay1off", Relay1Off);
	server.on("/relay2on", Relay2On);
	server.on("/relay2off", Relay2Off);

	server.on("/", handleRoot);
	server.on("/restart", []() { ESP.restart(); });
	server.on("/a", handleSettings);
	server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");
}

void handleSettings() {
	strcpy(settings.ssid, server.arg("ssid").c_str());
	strcpy(settings.pass, server.arg("pass").c_str());
	strcpy(settings.clientId, server.arg("clientid").c_str());
	strcpy(settings.mqttServer, server.arg("mqtt").c_str());
	saveConfig();
	server.send(200, "text/html", "Config saved <BR> <BR> <a href=\"/restart\">Restart device</a>");
}

String getMac()
{
	uint8_t mac[6];
	WiFi.macAddress(mac);
	return macToStr(mac);
}

String macToStr(const uint8_t* mac)
{
	String result;
	for (int i = 0; i < 6; ++i) {
		result += String(mac[i], 16);
		if (i < 5)
			result += ':';
	}
	return result;
}

char* toChar(String str)
{
	char message_buff[100];
	str.toCharArray(message_buff, str.length() + 1);

}