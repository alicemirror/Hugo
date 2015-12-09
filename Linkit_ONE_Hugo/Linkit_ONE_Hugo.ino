/*
  Hugo Project 1.0 Rev.3

  Main application

  Created by Enrico Miglino
  Platform: LinkIt ONE by Mediatek
  Copyright Balearic Dynamics Spain (balearicdynamics@gmail.com)

  Source released under Apache license 2.0
*/

#include <LGSM.h>
#include <LGPS.h>
#include <LBattery.h>
#include <LGPRS.h>
#include <LGPRSClient.h>
#include "Globals.h"

String remoteNumber = "";  // the number you will call

// Monitoring status
boolean armed; // True when the system is active
boolean cardReady; // True when the card is ready to operate SMS
boolean msgQueue; // True if there is a message in the sending queue
String msgQueueMessage; // The message that should be resent

unsigned long LastReport = 0; // Last time the connection has been triggered

// Number of cycles of remote sending before updting the
// battery level on the web
int batteryWebCounter;

// Number of cycles of remote sending before updting the
// temperature level on the web
int temperatureCounter;

// Number of cycles of remote sending before updting the
// motion status on the web
int motionCounter;

// Instantiate the GPRS
LGPRSClient globalClient;

// Warning sensor status
warnings alerts;

// Number of no-motion detections before setting the alarm condition.
// At the first next detected movement the alarm is reset.
int inactivityWarning;

// Number of low battery readings before sending an automatic alert
// depending on the different loop delays when the board is running or
// not, this alert occours with different frequencies
int lowBatteryNotification;

// -------------------------------------------------------
// Setup
// -------------------------------------------------------
void setup() {

// Serial.begin(38400);

  setupControl(); // Initialized the on-board control panel
  setColor(C_BLACK); // Power Off the RGB LED

  LTask.begin(); // Called once in setup()
  initSMSService();
  powerGPRS();

// Client has to be initiated after GPRS is established with the 
// correct APN settings.
// This is a temporary solution described in support forums}
// http://labs.mediatek.com/forums/posts/list/75.page
  LGPRSClient client;
  globalClient = client;

  welcomeSequence();
}

// -------------------------------------------------------
// Main loop
// -------------------------------------------------------
void loop() {

  // Check if there are incoming data from the server
  if (globalClient.available()) {// if there are incoming bytes available from the server
    char c = globalClient.read();   // read them and ignore
    // Data are not used, so are discharged
  }

  // --------------------------------- SMS PROCESS
  // Check if the card is ready to work
  if(cardReady) {
    // Check if there is an answer in the queue before processing
    // further ingoing messages
    if(msgQueue) {
        setColor(SMS_COLOR);
        sendSMSAnswer(msgQueueMessage);
        delay(SMS_PAUSED);
        // Reinitialise the SMS receiving
        resetSMSService();
        setColor(C_BLACK);
    } // process message queue
    else {
      // Check if there is new SMS
      if(LSMS.available()) {
        setColor(SMS_COLOR);
        // If the sender number is not authorized the message is flushed and ignored
        if(checkRemoteNumber()) {
          // Process the command and send the answer
          String inMessage;
          inMessage = readSMSMessage();
          sendSMSAnswer(inMessage);
          delay(SMS_PAUSED);
        } // End reading the message 
        // Reinitialise the SMS receiving
        resetSMSService();
        setColor(C_BLACK);
      } // SMS available
    } // process ingoing messages 
  } // Card is ready
  else {
    // Try reinitializing the SMS  service
    // if the previous call had no effect
    resetSMSService();
  } // Reinit the SMS service
  // --------------------------------- SMS PROCESS END

  // --------------------------------- WEB UPDATE
  // If the system has been initialized check for the timer trigger and
  // send data to the remote server
  if(armed) {
    // Check for the timer trigger
    if (millis() >= LastReport + FREQ_REMOTE) {
      // If battery counter reach the max value, it is updated the battery level
      // instead of the current GPS position
      String remoteData;  // Data string to send to the server

      // Check for battery level sending
      if(batteryWebCounter >= CYCLES_BATTERY_LEVEL) {
        remoteData = getWebBatteryLevel();
        batteryWebCounter = 0;
      }
      else if(temperatureCounter >= CYCLES_TEMPERATURE) {
        remoteData = getWebTemperature();
        // If the environmental temperature is overheating, it is forced an SMS sending
        if(checkTemperature() >= WARNING_TEMPERATURE_LIMIT) {
        setColor(SMS_COLOR);
        sendSMSAnswer(CMD_OVERHEATING);
        delay(SMS_PAUSED);
        // Reinitialise the SMS receiving
        resetSMSService();
        setColor(C_BLACK);
        }
        temperatureCounter = 0;
      }
      else if(motionCounter >= CYCLES_MOTION) {
        remoteData = getWebMotion();
        motionCounter = 0;
      }
      else {
        remoteData = getGPSWebPosition();
        batteryWebCounter++; // Increase the cycle counters
        motionCounter++;
        temperatureCounter++;
      } // Read GPS normally
      // Send the dta to the web
      setColor(WEB_COLOR);
      uploadstatus(remoteData);
      setColor(C_BLACK);
      LastReport = millis();
    } // 
  } // Following is armed
  // --------------------------------- WEB UPDATE END

  // --------------------------------- ALERT VISUAL SIGNALS AND LAST STUFF
  // Rapid flashing
  int loopDelay;
  
  if(armed) {
    checkAlerts();
    alarmVisualAlerts();
    setColor(C_WHITE);
    loopDelay = FLASH_SHORT_SIGNAL;
    setColor(C_BLACK);
  } else {
    setColor(C_CYAN);
    delay(FLASH_SHORT_SIGNAL);
    setColor(C_BLACK);
    loopDelay = STANDBY_INTERVAL;
  }

  delay(SMS_INTERVAL); // Loop delay
// --------------------------------- LOOP END
}

// -------------------------------------------------------
// Read the SMS message
// -------------------------------------------------------
String readSMSMessage() {
  int v;
  String msg;

  msg = "<cmd>";
  
  while(true) {
    v = LSMS.read();
    if(v < 0)
      break; 
      
    msg += (char)v; // Queue the character in the message string

  } // SMS reading
    
  return msg;
}

// -------------------------------------------------------
// Validate the remote sender number
// -------------------------------------------------------
boolean checkRemoteNumber() {
  char buf[REMOTE_NUMBER_LENGTH];
  String remoteNumber;
  
  // Retrieve the sender number
  LSMS.remoteNumber(buf, REMOTE_NUMBER_LENGTH); 

  // Check the remote number validity
  remoteNumber = buf;
  return remoteNumber.equals(REMOTE_NUMBER);
}

// -------------------------------------------------------
// Get the battery level string for message
// -------------------------------------------------------
String getBatteryLevel() {
  char batteryLevel[MSG_BUFFER];

  sprintf(batteryLevel, BATTERY_LEVEL, checkBatteryLevel());
  
  return batteryLevel;
}

// -------------------------------------------------------
// Get the temperature string for message
// -------------------------------------------------------
String getTemperature() {
  char tempLevel[MSG_BUFFER];

  sprintf(tempLevel, TEMPERATURE, checkTemperature());
  
  return tempLevel;
}


// -------------------------------------------------------
// Get the temperature alert string for message
// -------------------------------------------------------
String getTemperatureAlert() {
  char tempLevel[MSG_BUFFER];

  sprintf(tempLevel, WARNING_TEMPERATURE, checkTemperature());
  
  return tempLevel;
}

// -------------------------------------------------------
// Get the motion status string for message
// -------------------------------------------------------
String getMotion() {
  char motionStatus[MSG_BUFFER];

  if(alerts.motionAlert) {
    sprintf(motionStatus, MOTION, MOTION_ON);
  }
  else {
    sprintf(motionStatus, MOTION, MOTION_OFF);
  }
  
  return motionStatus;
}

// -------------------------------------------------------
// Get the battery level string for Web
// -------------------------------------------------------
String getWebBatteryLevel() {
  char batteryLevel[MSG_BUFFER];

  sprintf(batteryLevel, "%s%d", REMOTE_BATTERY, checkBatteryLevel());
// For debug only (replace the line above)
//  sprintf(batteryLevel, "%s%d", REMOTE_BATTERY, (int)9);
  
  return batteryLevel;
}

// -------------------------------------------------------
// Get the temperature string for Web
// -------------------------------------------------------
String getWebTemperature() {
  char temperature[MSG_BUFFER];

  sprintf(temperature, "%s%2.2f", REMOTE_TEMPERATURE, checkTemperature());
// For debug only (replace the line above)
//  sprintf(temperature, "%s%2.2f", REMOTE_TEMPERATURE, (float)43.50);
  
  return temperature;
}


// -------------------------------------------------------
// Get the motion status string for Web
// -------------------------------------------------------
String getWebMotion() {
  char motion[MSG_BUFFER];

  sprintf(motion, "%s%d", REMOTE_MOTION, alerts.motionAlert);
  
  return motion;
}

// -------------------------------------------------------
// Check the battery level.
// -------------------------------------------------------
int checkBatteryLevel() {
  return LBattery.level();
}

// -------------------------------------------------------
// Check the motion status reading the sensor status every
// 10 ms for 1 second
// -------------------------------------------------------
boolean checkMotion() {
  int motionCount;

  motionCount = 0;

  for(int j = 0; j < 100; j++) {
    motionCount += digitalRead(MOTION_PIN);
    delay(10);
  }

  // Motion counter should be greater than MOTION_MIN_LEVEL to detect signal
  if(motionCount > MOTION_MIN_LEVEL)
    return true;
  else
    return false;
}

// -------------------------------------------------------
// Check the temperature
// -------------------------------------------------------
float checkTemperature() {
  int analogValue;
  int tempPin = A0;
  float tempCelsius;

  analogValue = 0;
  tempCelsius = 0;

  // Loop read samples
  for(int j = 0; j < TEMPERATURE_SAMPLES; j++) {
    analogValue += analogRead(tempPin);
  }

  // And calculate the simple average value
  analogValue /= TEMPERATURE_SAMPLES;

  // Calculate the temperature value
  tempCelsius = analogValue * CELSIUS_K;

  return tempCelsius;
}

// -------------------------------------------------------
// Send an SMS warning message without request
// -------------------------------------------------------
void sendSMSWarning(String smsMsg) {

  // Set the target number beginning the SMS sending process
  LSMS.beginSMS(REMOTE_NUMBER);
  // Send the message (response)
  LSMS.print(smsMsg); 
  
  LSMS.endSMS();
}

// -------------------------------------------------------
// Send the response string in human readable form to the
// sender.
// -------------------------------------------------------
void sendSMSAnswer(String smsMsg) {
  String response = ""; // The response string to the command
  boolean sendResponse;

  msgQueue = false; // Disable the queue. Will be reenable if impossible to send message
  sendResponse = false; // Set to true only if a valid command is received

  // Check the message content for command
  // and prepare the response message

  // ................................................... Hot
  // (this is a self-sending message)
  if(smsMsg.equals(CMD_OVERHEATING) ) {
    response = getTemperatureAlert();
    sendResponse = true;
  // ................................................... Start
  } else if(smsMsg.equals(CMD_START) ) {
    if(!armed) {
      powerGPS(true);
      response = CMD_BOARDARMED;
      armed = true;
      sendResponse = true;
    }
    else {
      response = CMD_ALREADYRUNNING;
      sendResponse = true;
    }
  // ................................................... Stop
  } else if(smsMsg.equals(CMD_STOP) ) {
    if(armed) {
      powerGPS(true);
      response = CMD_BOARDSTOPPED;
      armed = false;
      sendResponse = true;
    }
    else {
      response = CMD_NOTRUNNING;
      sendResponse = true;
    }
  // ................................................... Info
  } else if(smsMsg.equals(CMD_INFO) ) {
    if(armed) {
      response = getBatteryLevel();
      response += getTemperature();
      response += getMotion();
      response += getGPSPosition();
      sendResponse = true;
    }
    else {
      response = CMD_NOINFO;
      sendResponse = true;
    }
  // ................................................... Firmware
  // (undocumented)
  } else if(smsMsg.equals(CMD_FIRMWARE) ) {
    char bl[MSG_BUFFER];
  
    sprintf(bl, CMD_FIRMWAREINFO, checkBatteryLevel());
    response = bl;
    sendResponse = true;
  // ................................................... Help
  } else if(smsMsg.equals(CMD_HELP) ) {
    response = CMD_HELPMSG;
    sendResponse = true;
  } 
  // ................................................... Unknown
  else {
    response = CMD_UNKNOWN;
    sendResponse = true;
  }

  // Send the answer SMS
  if(sendResponse) {
    // Set the target number beginning the SMS sending process
    LSMS.beginSMS(REMOTE_NUMBER);
    // Send the message (response)
    LSMS.print(response); 
    // Try sending message or activate the queue
    if(!LSMS.endSMS()) {
      msgQueue = true;  // Enable the queue for resend
      msgQueueMessage = smsMsg; // Saves the queued message
    }
  } // send response is true
}

// -------------------------------------------------------
// Initializes the SMS services ready to receive new ones
// And set the ready flag
// -------------------------------------------------------
void initSMSService() {
  int ec = 0; // Error count
  // Delete received SMS
  LSMS.flush();
  delay(SMS_PAUSED);
  // Wait for card initialisation 
  // If the card can't be initialised for a period, exit         
  while(ec < SIM_INIT_ERRORCOUNT) {
     cardReady = LSMS.ready();
     // Check if the card is ready or increase the error count
     if(cardReady) {
      ec = SIM_INIT_ERRORCOUNT;
     } // Card ready
     else {
       ec++;
     } // Card not ready
    delay(SMS_PAUSED);
  } // Wait for card ready or error count reached
}

// -------------------------------------------------------
// Flush the ingoing SMS buffer
// -------------------------------------------------------
void resetSMSService() {
  LSMS.flush();
  delay(SMS_PAUSED);
}

// -----------------------------------------------------------------
// Power On/Off the GPS module
// -----------------------------------------------------------------
boolean powerGPS(boolean onoff) {

  if(onoff == true) {
    LGPS.powerOn();
  }
  else {
    LGPS.powerOff();
  }
  return onoff;
}

// -----------------------------------------------------------------
// Power the GPRS module (called once on startup)
// -----------------------------------------------------------------
void powerGPRS() {

    while (!LGPRS.attachGPRS(GPRS_APN,"","")) { 
      delay(GPRS_POWER_DELAY);  
    }
}

// -----------------------------------------------------------------
// Get the actual GPS position for SMS (create a Google maps link)
// -----------------------------------------------------------------
String getGPSPosition() {
  gpsSentenceInfoStruct info;
  
  LGPS.getData(&info);
  return parseGPGGA((const char*)info.GPGGA, GPS_REQUEST_SMS);
}

// -----------------------------------------------------------------
// Get the actual GPS position for Mediatek sandbox variable
// -----------------------------------------------------------------
String getGPSWebPosition() {
  gpsSentenceInfoStruct info;
  
  LGPS.getData(&info);
  return parseGPGGA((const char*)info.GPGGA, GPS_REQUEST_WEB);
}

//----------------------------------------------------------------------
// Parse GPS NMEA buffer for extracting data
// Returns the string ready for the web of the SMS answer, depending
// on the request parameter.
//----------------------------------------------------------------------
String parseGPGGA(const char* GPGGAstr, int typeRequest){
    float altitude;
    float m_speed; // speed in m/s
    int sat_num; // number of visible satellites
    int day, month, year;
    float latitude;
    float longitude;
    char latitude_dir;
    char longitude_dir;
    int fix;
    int tmp, hour, minute, second, num ;
    char buff[GPS_BUFF_LENGTH]; // GPS data buffer
    String response; // The result string

  // Check if the GPS string is valid
  if(GPGGAstr[0] == '$') {
    
    tmp = getComma(1, GPGGAstr);
    // Get time details
    hour     = (GPGGAstr[tmp + 0] - '0') * 10 + (GPGGAstr[tmp + 1] - '0');
    minute   = (GPGGAstr[tmp + 2] - '0') * 10 + (GPGGAstr[tmp + 3] - '0');
    second    = (GPGGAstr[tmp + 4] - '0') * 10 + (GPGGAstr[tmp + 5] - '0');

    //get time
    if(typeRequest == GPS_REQUEST_SMS) {
      sprintf(buff, "\nUTC time %2d:%2d:%2d\n", hour, minute, second);
      response = buff;
    }
    
    //get lat/lon coordinates
    float latitudetmp;
    float longitudetmp;
    tmp = getComma(2, GPGGAstr);
    latitudetmp = getFloatNumber(&GPGGAstr[tmp]);
    tmp = getComma(4, GPGGAstr);
    longitudetmp = getFloatNumber(&GPGGAstr[tmp]);
    latitude = convertLatitude(latitudetmp);
    longitude = convertLongitude(longitudetmp);
    
    if(typeRequest == GPS_REQUEST_SMS) {
      sprintf(buff, " http://maps.google.com/?q=%10.4f,%10.4f\n", latitude, longitude);
      response += removeSpaces(buff);
    }
    else {
      // Build the response string for the web
      sprintf(buff, "%s%10.4f,%10.4f,0", REMOTE_POSITION, latitude, longitude);
      response = removeSpaces(buff);
    }

    // Extra info: direction and satellite information only in the SMS response
    if(typeRequest == GPS_REQUEST_SMS) {
      //get lat/lon direction
      tmp = getComma(3, GPGGAstr);
      latitude_dir = (GPGGAstr[tmp]);
      tmp = getComma(5, GPGGAstr);    
      longitude_dir = (GPGGAstr[tmp]);
      sprintf(buff, "Direction: %c, %c\n", latitude_dir, longitude_dir);
      response += buff;
    
      //get GPS fix quality
      tmp = getComma(6, GPGGAstr);
      fix = getIntNumber(&GPGGAstr[tmp]);    
      sprintf(buff, "GPS fix quality = %d\n", fix);
      response += buff;

      //get satellites in view
      tmp = getComma(7, GPGGAstr);
      num = getIntNumber(&GPGGAstr[tmp]);    
      sprintf(buff, "Num satellites: %d\n", num);
      response += buff;
    } // request type is SMS

    return response;
  } // GPS parser
  else{
    if(typeRequest == GPS_REQUEST_SMS) {
      return NO_GPS_DATA; 
    } // SMS message
    else {
      return ""; // Web varuabke
    }
  } // No GPS data
}

//----------------------------------------------------------------------
// Convert GPGGA longitude (degrees-mins-secs) to true decimal-degrees
//----------------------------------------------------------------------
float convertLatitude(float latitude) {
  int lat_deg_int = int(latitude/100);    //extract the first 2 chars to get the latitudinal degrees
    // must now take remainder/60
    // this is to convert from degrees-mins-secs to decimal degrees
    // so the coordinates are "google mappable"
    float latitude_float = latitude - lat_deg_int * 100;    //remove the degrees part of the coordinates - so we are left with only minutes-seconds part of the coordinates
    return lat_deg_int + latitude_float / 60;      //add back on the degrees part, so it is decimal degrees
}

//----------------------------------------------------------------------
// Convert GPGGA longitude (degrees-mins-secs) to true decimal-degrees
//----------------------------------------------------------------------
float convertLongitude(float longitude) {
  int lon_deg_int = int(longitude/100);   //extract first 3 chars to get the longitudinal degrees
    // must now take remainder/60
    // this is to convert from degrees-mins-secs to decimal degrees
    // so the coordinates are "google mappable"
    float longitude_float = longitude - lon_deg_int * 100;     
    return lon_deg_int + longitude_float / 60;
}

// -----------------------------------------------------------------
// Remove the spaced from a string
// -----------------------------------------------------------------
static String removeSpaces(String strBuff) {
  String result = "";

  for(int j = 0; j < strBuff.length(); j++) {
    if(strBuff.charAt(j) != ' ')
      result += strBuff.charAt(j);
  }

  return result;
}

//----------------------------------------------------------------------
//!\brief  return position of the comma number 'num' in the char array 'str'
//!\return  char
//----------------------------------------------------------------------
static unsigned char getComma(unsigned char num,const char *str){
  unsigned char i,j = 0;
  int len=strlen(str);
  for(i = 0;i < len;i ++){
    if(str[i] == ',')
      j++;
    if(j == num)
      return i + 1; 
    }
  return 0; 
}

//----------------------------------------------------------------------
//!\brief convert char buffer to float
//!\return  float
//----------------------------------------------------------------------
static float getFloatNumber(const char *s){
  char buf[10];
  unsigned char i;
  float rev;

  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atof(buf);
  return rev; 
}

//----------------------------------------------------------------------
//!\brief convert char buffer to int
//!\return  float
//----------------------------------------------------------------------
static float getIntNumber(const char *s){
  char buf[10];
  unsigned char i;
  float rev;

  i=getComma(1, s);
  i = i - 1;
  strncpy(buf, s, i);
  buf[i] = 0;
  rev=atoi(buf);
  return rev; 
}

// -----------------------------------------------------------------
// Build the battery data to be sent to the server
// -----------------------------------------------------------------
String buidBatteryData() {
  String dataVariable;
  dataVariable = REMOTE_BATTERY;
  dataVariable += LBattery.level();
  return dataVariable;
}

// -----------------------------------------------------------------
// Upload remote status
// -----------------------------------------------------------------
void uploadstatus(String dataVariable){

int dataLength = dataVariable.length();

  if(globalClient.connect(SITE_URL, SITE_PORT)) {
  
    globalClient.print(SERVER_POST);
    globalClient.print(DEVICEID);
    globalClient.println(SERVER_DATAPOINTS_HTTP);
    globalClient.print(SERVER_HOST);
    globalClient.println(SITE_URL);
    globalClient.print(SERVER_DEVICE_KEY);
    globalClient.println(DEVICEKEY);
    globalClient.print(SERVER_DATALENGTH);
    globalClient.println(dataLength);
    globalClient.println(SERVER_CONTENT_TYPE);
    globalClient.println(SERVER_CONNECTION_CLOSE);
    globalClient.println();
    globalClient.println(dataVariable);
  
    delay(HTTP_RESPONSE_DELAY);
  }
}

// -----------------------------------------------------------------
// Set the desired color on the RGB LED (including black = off)
// -----------------------------------------------------------------
void setColor(int colorID) {

  digitalWrite(RED_PIN, rgb[colorID][L_RED]);
  digitalWrite(GREEN_PIN, rgb[colorID][L_GREEN]);
  digitalWrite(BLUE_PIN, rgb[colorID][L_BLUE]);

}

// -----------------------------------------------------------------
// Initializes the I/O pins of the control panel and configuration status
// -----------------------------------------------------------------
void setupControl() {
  
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(MOTION_PIN, INPUT);

  armed = false;  // Board boot as not running
  cardReady = false;  // Enabled on MicroSD card initialisation
  msgQueue = false; // Status depends if there are SMS awaiting for sending
  batteryWebCounter = CYCLES_BATTERY_LEVEL; 
  temperatureCounter = CYCLES_TEMPERATURE;
  motionCounter = CYCLES_MOTION;
  inactivityWarning = 0;
  lowBatteryNotification = 0;

  alerts.temperature = false;
  alerts.battery = false;
  alerts.batteryAutoAlert = false;
  alerts.motion = false;
  alerts.motionAlert = false;
}

// -----------------------------------------------------------------
// LED initialization cycle
// -----------------------------------------------------------------
void welcomeSequence() {

  // Loop over all the colors in sequence from RED to BLACK
  for(int j = C_RED; j <= C_BLACK; j++) {
    for(int k = 0; k < 3; k ++) {
      setColor(j);
      delay(FLASH_SHORT_SIGNAL);
    } // every color cycle
  } // all colors loop
  
}

// -----------------------------------------------------------------
// Check and update the alert status
// -----------------------------------------------------------------
void checkAlerts() {
  // Check battery level
  if(checkBatteryLevel() <= WARNING_BATTERY_LOW_LIMIT) {
  alerts.battery = true;
  }
  else {
    alerts.battery = false;
  }

  // Check temperature
  if(checkTemperature() > WARNING_TEMPERATURE_LIMIT) {
    alerts.temperature = true;
  }
  else {
    alerts.temperature = false;
  }

  // Check motion status
  if(checkMotion() == false) {
    alerts.motion = true; 
  }
  else {
    alerts.motion = false;
  }
}

// -----------------------------------------------------------------
// Manage the visual alarms for alerts
// -----------------------------------------------------------------
void alarmVisualAlerts() {

  // Check temperature alert
  if(alerts.temperature) {
    setColor(TEMPERATURE_ALERT_COLOR);
    delay(FLASH_ALERT_SIGNAL);
    setColor(C_BLACK);
    delay(FLASH_SHORT_SIGNAL);
  }

  if(alerts.motion) {
    // Check if it is time to enable the warning
    if(inactivityWarning >= MAX_INACTIVITY_WARNING) {
      alerts.motionAlert = true;
      setColor(MOTION_ALERT_COLOR);
      delay(FLASH_ALERT_SIGNAL);
      setColor(C_BLACK);
      delay(FLASH_SHORT_SIGNAL);
      inactivityWarning = 0;
    } // Set warning
    else {
      alerts.motionAlert = false;
      // increase the counter
      inactivityWarning++;
      setColor(MOTION_WARNING_COLOR);
      delay(FLASH_ALERT_SIGNAL);
      setColor(C_BLACK);
      delay(FLASH_SHORT_SIGNAL);
    } // Alert persists, increase counter
  } // Motion is in alert condition
  else {
    // No alert motion condition
    // Reset the warning counter
    alerts.motionAlert = false;
    inactivityWarning = 0;
    setColor(MOVING_COLOR);
    delay(FLASH_ALERT_SIGNAL);
    setColor(C_BLACK);
    delay(FLASH_SHORT_SIGNAL);
  }
   
   if(alerts.battery) {
    setColor(BATTERY_ALERT_COLOR);
    delay(FLASH_ALERT_SIGNAL);
    setColor(C_BLACK);
    delay(FLASH_SHORT_SIGNAL);
   }
}

