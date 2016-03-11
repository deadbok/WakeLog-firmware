#include <strings.h>

#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include <SmingCore/Wire.h>
#include <Libraries/DS3232RTC/DS3232RTC.h>
#include "FlashLog.h"
 
// Connect DS3232 / DS3231 to GND, VCC, GPIO0 - SCL, GPIO2 - SDA or any other free GPIO and change below Wire.pins(0,2)
// to you SCL and SDA pins

/**
 * @brief Linker symbol that points to the end of the ROM code in flash.
 */
extern char _irom0_text_start[];
extern char _irom0_text_end[];

void onNtpReceive(NtpClient& client, time_t timestamp);

Timer statusTimer;
FlashLog<time_t> *flog;
HttpServer server;
NtpClient ntpClient("pool.ntp.org", 0, onNtpReceive);

void printStatus()
{
	Serial.println("Status:");
	if (WifiStation.isConnected())
	{
		Serial.printf(" WIFI connected, IP address: ");
		Serial.print(WifiStation.getIP());
		Serial.println(".");
	}
	Serial.print(" Local Time: ");
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print(" UTC Time: ");
	Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));
	
	Serial.print(" RTC Time: ");
	DateTime _date_time = DSRTC.get();
	Serial.println(_date_time.toFullDateTimeString());
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	String head = "<html><head><title>OnLog</title></head><body><h2>Log:</h2>";
	String body;
	String tail = "</body></html>";
	
	debugf("Sending index.");
	
	while (flog->getCount())
	{
		body += DateTime(flog->popFront()).toFullDateTimeString();
		body += "<br />";
	}
	
	response.sendString(head);
	response.sendString(body);
	response.sendString(tail);
}

void onOther(HttpRequest &request, HttpResponse &response)
{
	debugf("Sending 404.");
	response.notFound();
}

void startWebServer()
{
	Serial.println("Starting web server.");
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onOther);

	// Web Sockets configuration
	//server->enableWebSockets(true);
	//server.setWebSocketConnectionHandler(wsConnected);
	//server.setWebSocketMessageHandler(wsMessageReceived);
	//server.setWebSocketBinaryHandler(wsBinaryReceived);
	//server.setWebSocketDisconnectionHandler(wsDisconnected);

	Serial.print("Web server started.");
}


void onNtpReceive(NtpClient& client, time_t timestamp)
{
	debugf("Time from NTP server: %s.", DateTime(timestamp).toFullDateTimeString().c_str());

	ntpClient.setAutoQuery(false);
	SystemClock.setTime(timestamp);
	//DSRTC.set(SystemClock.now());
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	Serial.printf(" WIFI connected, IP address: ");
	Serial.print(WifiStation.getIP());
	Serial.println(".");
	Serial.println("Setting time via NTP.");
	
	ntpClient.setAutoQueryInterval(60);
	ntpClient.setAutoQuery(true);
	ntpClient.requestTime();

	debugf("Connected time: %s.", SystemClock.now().toFullDateTimeString().c_str());
}

void connectFail()
{
	debugf("No WIFI connection.");
	WifiStation.waitConnection(connectOk, WIFI_RETRY_SEC, connectFail);
}

void init()
{
	uint32_t log_addr;
	DateTime log_time;
	
	//Turn of SDK debug messages.
	system_set_os_print(true);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(NAME " version " VERSION);
	
	debugf(" ROM code start: 0x%x.", (uint32_t)_irom0_text_start);
	debugf(" ROM code end: 0x%x.", (uint32_t)_irom0_text_end);
	log_addr = (((uint32_t)(_irom0_text_end) - INTERNAL_FLASH_START_ADDRESS + 0x4000) & 0x000fc000) >> 0xc;
	debugf(" Saving log data at flash sector 0x%x.", log_addr);
	
	//Setup GPIO for RTC module.
	Wire.pins(0, 2); //Change to your SCL,SDA GPIO pin number
    Wire.begin();

	//Set timezone hourly difference to UTC
	SystemClock.setTimeZone(1);
	
	//Log the current time.
	log_time = SystemClock.now();
	Serial.printf("Logging time stamp: %s.\n", log_time.toFullDateTimeString().c_str()); 
    flog = new FlashLog<time_t>(log_addr);
	flog->pushBack(log_time);

	//Start status timer.
	if (STATUS_INT_SEC)
	{
		statusTimer.initializeMs(STATUS_INT_SEC * 1000, printStatus).start();
	}
	
	//Start connecting WIFI.
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	WifiStation.waitConnection(connectOk, WIFI_RETRY_SEC, connectFail);
	
	startWebServer();
}
