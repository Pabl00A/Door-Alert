

// ----------------------------
// Standard Libraries - Already Installed if you have ESP32 set up
// ----------------------------

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "esp_camera.h"

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

//#include <UniversalTelegramBot.h>
#include "UniversalTelegramBot.h"
// Library for interacting with the Telegram API
// Search for "Telegegram" in the Library manager and install
// The universal Telegram library
// https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot

#include <ArduinoJson.h>
// Library used for parsing Json from the API responses
// Search for "Arduino Json" in the Arduino Library manager
// https://github.com/bblanchon/ArduinoJson


//------- Replace the following! ------

//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

// Initialize Wifi connection to the router
char ssid[] = "*************************";     // your network SSID (name)
char password[] = "**************************************"; // your network key

// Initialize Telegram BOT
#define BOTtoken "*********************************"  // your Bot Token (Get from Botfather)
String chat_id = "*********************";   // Get chat_id
//------- ------------------------ ------

#include "camera_pins.h"
#include "camera_code.h"



#define FLASH_LED_PIN 4

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

bool flashState = LOW;

camera_fb_t* fb = NULL;

bool isMoreDataAvailable();
//byte* getNextBuffer();
//int getNextBufferLen();

////////////////////////////////  send photo as 512 byte blocks -
int currentByte;
uint8_t* fb_buffer;
size_t fb_length;

bool isMoreDataAvailable() {
	return (fb_length - currentByte);
}

uint8_t getNextByte() {
	currentByte++;
	return (fb_buffer[currentByte - 1]);
}

bool dataAvailable = false;

//////////////////////////////// send photo as 1 giant block

bool isMoreDataAvailableXXX() {
	if (dataAvailable)
	{
		dataAvailable = false;
		return true;
	}
	else {
		return false;
	}
}

byte* getNextBuffer() {
	if (fb) {
		return fb->buf;
	}
	else {
		return nullptr;
	}
}

int getNextBufferLen() {
	if (fb) {
		return fb->len;
	}
	else {
		return 0;
	}
}

int periodo = 7200000;//2horas
unsigned long TiempoAhora = 0;
int periodo1 = 1000;
unsigned long TiempoAhora1 = 0;
bool estado = 0;//
boolean fff = false;

unsigned long TiempoAhoracheckeo = 0;


#define mc_38 13
////////////////////////////

void handleNewMessages(int numNewMessages) {
	Serial.println("handleNewMessages");
	Serial.println(String(numNewMessages));

	for (int i = 0; i < numNewMessages; i++) {
		// Chat id of the requester
		String CHAT_ID = String(bot.messages[i].chat_id);
		if (CHAT_ID != chat_id) {
			bot.sendMessage(CHAT_ID, "Sorry, this bot is no longer available", "");
			bot.sendMessage(chat_id, "Alguien entr??", "");////////////////////////////////nse, puede ser errir
			continue;
		}
		//  bot.sendMessage(chat_id, "Buenas", "");
		 // Print the received message
		String text = bot.messages[i].text;



		if (text == "/uxga" || text == "/photo" || text == "/caption") {
			foto();
		}


	}
}



void setup() {
	Serial.begin(115200);
reconect:
	delay(3000);
	// Attempt to connect to Wifi network:
	Serial.print("Connecting Wifi: ");
	Serial.println(ssid);
	delay(2000);
	// Set WiFi to station mode and disconnect from an AP if it was Previously connected

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	delay(2000);
	int iii = 0;
	while (WiFi.status() != WL_CONNECTED && iii < 15) {
		Serial.print(".");
		delay(200);
		iii++;
	}
	if (iii == 15) {
		goto reconect;
	}
	Serial.println("");
	Serial.println("WiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	// Make the bot wait for a new message for up to 60seconds
	bot.longPoll = 10;

	delay(2000);
	pinMode(FLASH_LED_PIN, OUTPUT);
	digitalWrite(FLASH_LED_PIN, flashState); //defaults to low
	delay(3000);
	if (!setupCamera()) {
		goto reconect;
		Serial.println("Camera Setup Failed!");
		delay(10000);
		Serial.println("Restarting por camera");
		ESP.restart();
		while (true) {
			delay(100);
		}
	}
	String welcome = "Welcome, solo va foto.\n\n";
	welcome += "/photo : will take a photo\n";
	bot.sendMessage(chat_id, welcome, "");
	pinMode(mc_38, INPUT_PULLUP);
}

void foto() {
	fb = NULL;

	sensor_t* s = esp_camera_sensor_get();
	//s->set_framesize(s, FRAMESIZE_QVGA);  // jz  qvga 320x250   4 kb
	s->set_framesize(s, FRAMESIZE_UXGA);  // jz  uxga 1600x1200 100 kb

	fb = esp_camera_fb_get();
	esp_camera_fb_return(fb);
	fb = esp_camera_fb_get();
	esp_camera_fb_return(fb);



	// Take Picture with Camera
	fb = esp_camera_fb_get();
	if (!fb) {
		Serial.println("Camera capture failed");
		bot.sendMessage(chat_id, "Camera capture failed", "");
		return;
	}

	currentByte = 0;
	fb_length = fb->len;
	fb_buffer = fb->buf;


	Serial.println("\n\n\nSending UXGA");
	Serial.println("\n>>>>> Sending as 512 byte blocks, with jzdelay of 0, bytes=  " + String(fb_length));

	bot.sendPhotoByBinary(chat_id, "image/jpeg", fb_length,
		isMoreDataAvailable, getNextByte,
		nullptr, nullptr);

	dataAvailable = true;
	Serial.println("\n>>>>>Sending as one block, bytes=  " + String(fb_length));

	Serial.println(">>>> This should delay 1 minute and then fail");

	bot.sendPhotoByBinary(chat_id, "image/jpeg", fb->len,
		isMoreDataAvailableXXX, nullptr,
		getNextBuffer, getNextBufferLen);

	Serial.println(">>>> That should have failed");

	Serial.println("done!");


	esp_camera_fb_return(fb);


}

void loop() {

	delay(1000);
	/*  if (millis() > Bot_lasttime + Bot_mtbs)  {
		int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

		while (numNewMessages) {
		  Serial.println("got response");
		  handleNewMessages(numNewMessages);
		  numNewMessages = bot.getUpdates(bot.last_message_received + 1);
		}

		Bot_lasttime = millis();
	  }*/
	if (millis() > 36000000) {
		bot.sendMessage(chat_id, "Empez?? checkeo", "");
      ESP.restart();
	}
	if (millis() > TiempoAhora + periodo) {
		TiempoAhora = millis();
		bot.sendMessage(chat_id, "online", "");
	}


	if (digitalRead(mc_38)) {
		if (millis() > TiempoAhora1 + 2000) {
			delay(2000);
			foto();
			bot.sendMessage(chat_id, "ALGUIEN ENTR??", "");
			delay(2000);
			foto();
			delay(2000);
			foto();
			delay(2000);
			foto();
			TiempoAhora1 = millis();
		}
	}


}
