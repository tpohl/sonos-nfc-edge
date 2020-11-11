// This #include statement was automatically added by the Particle IDE.
#include <HttpClient.h>

#include <HttpClient.h>

#define SYSLOG_DEBUG
#define SYSLOG_USE_DEVICEID
#include <psyslog.h>


#include <MFRC522.h>
#define SS_PIN SS
#define RST_PIN D2

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance.





// Color LED
int red = D4;
int green = D5;
int blue = D6;


unsigned int ledsOff = 0; 

// The room
String room;

// The sonos api hostname
String sonosApiHostName;

String lastTag;

void setup() {
    
	Serial.begin(9600);	// Initialize serial communications with the PC
	
	RGB.control(true); 
    RGB.color(0, 0, 0);
	
	// Logging 
	waitUntil(WiFi.ready);

    // Change this IP to your syslog server!
    syslog_initialize("logs6.papertrailapp.com", 14747);

    // MRFC522 Setup
	mfrc522.setSPIConfig();
	mfrc522.PCD_Init();	// Init MFRC522 card
	
	// SETUP RGB LED
    pinMode (red, OUTPUT);
    pinMode (green, OUTPUT);
    pinMode (blue, OUTPUT);


    //Read the room
    room = readRoom();
    
    sonosApiHostName=readHost();
    
    // SETUP CLOUD FUNCITONS
    // register the cloud functions
    Particle.function("room", setRoom);
    Particle.function("apiHost", setSonosApiHostName);
    Particle.function("tag", prepareTag);
    Particle.function("wifiSetup", enableWifiSetup);
    Particle.variable("room", room);
    Particle.variable("apiHost", sonosApiHostName);
    Particle.variable("lastTag", lastTag);
	
	setColor(0,255,0);
	
	LOGI("Device started");
	
}

bool shallWrite = false;
String payloadToWrite;

int setRoom(String _room){
    room = _room;
    Serial.print("Room Updated to ");
    Serial.println(room);
    writeRoom(_room);
    return 1;
}

const int roomAddr = 0;

String readRoom(){
    const int STRING_BUF_SIZE = 16;
	char stringBuf[STRING_BUF_SIZE];

	EEPROM.get(roomAddr, stringBuf);
	stringBuf[sizeof(stringBuf) - 1] = 0; // make sure it's null terminated

	// Initialize a String object from the buffer
	String room(stringBuf);
	
	LOGI("Read Room %s from EEPROM", stringBuf);
    return room;
}


void writeRoom(String _room){
    // WRITE ROOM TO EEPROM
	const int STRING_BUF_SIZE = 16;
	char stringBuf[STRING_BUF_SIZE];

	String str = _room;
	Serial.printlnf("Storing Room =%s", str.c_str());
    LOGI("Storing Room %s", str.c_str());
	// getBytes handles truncating the string if it's longer than the buffer.
	str.getBytes((unsigned char *)stringBuf, sizeof(stringBuf));
	EEPROM.put(roomAddr, stringBuf);
}

int setSonosApiHostName(String _hostname){
    sonosApiHostName = _hostname;
    writeHost(_hostname);
    return 1;
}

const int hostAddr = 16;
const int hostLength = 32;

String readHost(){
    const int STRING_BUF_SIZE = hostLength;
	char stringBuf[STRING_BUF_SIZE];

	EEPROM.get(hostAddr, stringBuf);
	stringBuf[sizeof(stringBuf) - 1] = 0; // make sure it's null terminated

	// Initialize a String object from the buffer
	String sonosApiHostName(stringBuf);
	
	LOGI("Read sonosApiHostName %s from EEPROM", stringBuf);
    return sonosApiHostName;
}


void writeHost(String _host){
        // WRITE HOST TO EEPROM
	const int STRING_BUF_SIZE = hostLength;
	char stringBuf[STRING_BUF_SIZE];

	String str = _host;
	Serial.printlnf("Storing Host =%s", str.c_str());
    LOGI("Storing HOST %s", str.c_str());
	// getBytes handles truncating the string if it's longer than the buffer.
	str.getBytes((unsigned char *)stringBuf, sizeof(stringBuf));
	EEPROM.put(hostAddr, stringBuf);
}




// Cloud Functions
int prepareTag(String tag){
    payloadToWrite = tag;
    Serial.print("Hold Card to write ");
    Serial.println(tag);
    setColor(1,0,0);
    LOGI("Preparing Write to Tag: %s", tag.c_str());
    shallWrite = true;
    return 1;
}

int enableWifiSetup(String timeout){
 //   WiFi.setListenTimeout(60);
 //   WiFi.listen();
    return 1;
}


// LED Colors
void setColor(int _red, int _green, int _blue){
  LOGI("Switching Light to rgb(%d,%d,%d)", _red, _green, _blue);
//  analogWrite(green, _green);   // Turn ON the LED
//  analogWrite(blue, _blue);
  digitalWrite(red, _red > 0 );
  digitalWrite(green, _green > 0 );
  digitalWrite(blue, _blue > 0 );
  
  RGB.color(_red, _green, _blue);
  
  if (_red+_green+_blue>0) {
    // If not off, set a timer.
    ledsOff = millis() + 5000;
  } else {
      ledsOff = -1;
  }
}

// ------------------- WRITE to TAGS -------------------

// Constants for the lightweight TAGS
    const byte blockLength = 4;
    const int firstBlock = 7;
    const int lastBlock = 16;
    const int length = (lastBlock-firstBlock)*blockLength;

void writeStringToTag(String text){

    byte data[length];
    byte status;
    // Initialize with zeroes.
    for(int i = 0; i < length; i++)
    {
        data[i] = 0;
    }
    text.getBytes(data, length);
    
    Serial.print("PAYLOAD to WRITE ");
    Serial.println(text);
    
    byte buffer[4];
            for(int j = 0; j < 4; j++)
    {
        buffer[j] = 0;
    }
    for(int i = firstBlock; i < lastBlock; i++) {
 
        int offset = (i-firstBlock) * blockLength;
        buffer[0]=data[offset+0];
        buffer[1]=data[offset+1];
        buffer[2]=data[offset+2];
        buffer[3]=data[offset+3];
        
        for(int i = 0; i < blockLength; i++)
        {
          Serial.println(buffer[i]);
        }
      
      status = mfrc522.MIFARE_Ultralight_Write(i,buffer,blockLength);
       if (status != MFRC522::STATUS_OK) {
           Serial.println("Data write failed");
           Serial.println(mfrc522.GetStatusCodeName(status));
           LOGI("Cannot write to tag: ", mfrc522.GetStatusCodeName(status));
           return;
       }
    }
    
    mfrc522.PICC_HaltA();
    Serial.println("Data write completed.");
    LOGI("Data write completed.");
    setColor(0,0,255);
    shallWrite = false;
   
}

// ------------------- READ from TAGS -------------------

String readStringFromTag(){
    byte data[length];
    byte status;
    
    
    for(byte i = firstBlock; i < lastBlock; i+=4) {
        byte buffer[18];
        byte size = sizeof(buffer);
        // READ RETURNS 4 BLOCKS at once
        status = mfrc522.MIFARE_Read(i,buffer, &size);
        if (status != MFRC522::STATUS_OK) {
           Serial.println("Data read failed");
           Serial.println(mfrc522.GetStatusCodeName(status));
           LOGI("Log Data read failed %s", mfrc522.GetStatusCodeName(status));
           return "";
       }
       // Dump data
       byte offset_pages = (i-firstBlock) * blockLength;
		for (byte offset = 0; offset < 4; offset++) {
			
			for (byte index = 0; index < 4; index++) {
				byte i2 = 4 * offset + index;
				data[offset_pages + i2] = buffer[i2];
			}
		}
    }
    String text = String((char*) data);
    LOGI("Read Tag. %s", data);
    lastTag = text;
    return text;
}


// ------------------------------- HTTP / SONOS STUFF

HttpClient http;
String lastName = "";
unsigned int forgetLastTag = -1; 
unsigned int TAG_FORGET_PERIOD = 30000; // Forget the last Tag after 30 Seconds

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

void play(String room, String sonosType, String name){
    if (!lastName.equals(name)) {
        Particle.publish("sonos-nfc/play", name, 60, PRIVATE);
        lastName = name;
        forgetLastTag = millis() + TAG_FORGET_PERIOD;
    
        request.hostname = sonosApiHostName;
        request.port = 5005;
        String path = "/";
        path = path + room + "/" + sonosType + "/" + name;
        request.path = path;
        Serial.print("SONOS - Getting ");
        Serial.println(path);
        LOGI("Performing SONOS Action: %s", path.c_str());
        // The library also supports sending a body with your request:
        //request.body = "{\"key\":\"value\"}";
    
        // Get request
        http.get(request, response, headers);
        Serial.print("Application>\tHTTP Response Body: ");
        Serial.println(response.body);
        LOGD("SONOS API Response: %s", response.body.c_str());
    }
}

void checkForgetLastTag(){
    if (forgetLastTag > 0 && forgetLastTag < millis()) {
        lastName = "";
        forgetLastTag = -1;
    }
}
// --------------------------------



void loop() {
    if (ledsOff > 0 && ledsOff < millis()) {
        setColor(0,0,0);
    }
    checkForgetLastTag();
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
	    Serial.println("PICC_ReadCardSerial() failed. ");
	    LOGD("PICC_ReadCardSerial() failed. ");
	    setColor(255,0,0);
		return;
	}

    if (shallWrite) {
        writeStringToTag(payloadToWrite);
        
    } else {
         String tag = readStringFromTag();
         if (tag[0] != '\0') {
            
            LOGD("Tag read: %s", tag.c_str());
            Serial.print("Tag read ");
            Serial.println(tag);
            Particle.publish("sonos-nfc/read", tag, 60, PRIVATE);
            
            char typeChar = tag[0];
            String name = tag.substring(2);
            
            // Check for NFC Standard Tag first
            char textCheck1 = tag[0];
            char textCheck2 = tag[1];
            if (textCheck1 == 'e' && textCheck2 == 'n' ){
                typeChar= tag[2];
                name = tag.substring(4, tag.length()-1);
               
            } 
            
            String sonosType;
            switch (typeChar) {
                case 'p' :
                    sonosType = "playlist";
                    play(room, sonosType, name);
                    break;
                default:
                    LOGW("No Valid SONOS Type read: %s", typeChar);
                    Serial.printlnf("No Valid SONOS Type read: %s", typeChar);
            }
            setColor(0,255,0);
         }
    }
   

}
