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

Timer printTimer;
FlashLog<time_t> *flog;
HttpServer *server;

void PrintSystemTime()
{
	Serial.print("Local Time: ");
	Serial.println(SystemClock.getSystemTimeString());
	Serial.print("UTC Time: ");
	Serial.println(SystemClock.getSystemTimeString(eTZ_UTC));
	
	Serial.print("RTC Time: ");
	DateTime _date_time = DSRTC.get();
	Serial.println(_date_time.toFullDateTimeString());
	flog->pushBack(SystemClock.now());
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	const char *html = "<html><head><title>OnLog</title></head><body><h2>Log:</h2></body></html>";
	
	response.sendString(html);
}

void onOther(HttpRequest &request, HttpResponse &response)
{
	response.notFound();
}

void startWebServer()
{
	server = new HttpServer;
	server->listen(80);
	server->addPath("/", onIndex);
	server->setDefaultHandler(onOther);

	// Web Sockets configuration
	//server->enableWebSockets(true);
	//server.setWebSocketConnectionHandler(wsConnected);
	//server.setWebSocketMessageHandler(wsMessageReceived);
	//server.setWebSocketBinaryHandler(wsBinaryReceived);
	//server.setWebSocketDisconnectionHandler(wsDisconnected);

	Serial.print("Web server started on IP ");
	Serial.print(WifiStation.getIP());
	Serial.println(".");
}

// Will be called when WiFi station was connected to AP
void connectOk()
{
	DateTime _date_time = DSRTC.get();
	debugf("Connected time: %s.", _date_time.toFullDateTimeString().c_str());
	Serial.println("Starting web server.");
	//startWebServer();
}

void init()
{
	uint32_t log_addr;
	
	//system_set_os_print(false);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(NAME " version " VERSION);
	
	debugf(" ROM code start: 0x%x.", (uint32_t)_irom0_text_start);
	debugf(" ROM code end: 0x%x.", (uint32_t)_irom0_text_end);
	log_addr = LOG_FLASH_SEC;
	debugf(" Saving log data at flash sector 0x%x.", log_addr);
	Wire.pins(0, 2); //Change to your SCL,SDA GPIO pin number
    Wire.begin();
    
    flog = new FlashLog<time_t>(log_addr);

	// Set timezone hourly difference to UTC
	SystemClock.setTimeZone(1);

	printTimer.initializeMs(LOG_INT_SEC * 1000, PrintSystemTime).start();
	
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiStation.waitConnection(connectOk);
}
