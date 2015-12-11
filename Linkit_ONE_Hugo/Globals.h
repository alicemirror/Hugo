/*
  Hugo Project 1.0 Rev.3

  Structures and global definitions
  Localisation: English Int.

  Created by Enrico Miglino
  Platform: LinkIt ONE by Mediatek
  Copyright Balearic Dynamics Spain (balearicdynamics@gmail.com)

  Source released under Apache license 2.0
*/

#define REMOTE_NUMBER "+34636230559"
#define REMOTE_NUMBER_LENGTH 20
#define SMS_PAUSED 1000 // SMS Internal pause delay witing actions to be completed
#define SMS_INTERVAL 2000 // SMS Check message period
#define STANDBY_INTERVAL 30000 // Loop delay when the board is not active
#define WARNING_BATTERY_LOW_LIMIT 34 // Battery level limit before automatic warnings
#define MSG_BUFFER 160 // SMS Message string buffer length
#define SIM_INIT_ERRORCOUNT 5 // Max number of retries intializing the SIM card

// Control commands
#define CMD_START "<cmd>Start"
#define CMD_STOP "<cmd>Stop"
#define CMD_INFO "<cmd>Info"
#define CMD_HELP "<cmd>Help"
#define CMD_OVERHEATING "<cmd>Hot"
#define CMD_FIRMWARE "<cmd>Firmware"
#define CMD_QUIET "<cmd>Quiet"
#define CMD_COLLECT "<cmd>Collect"
#define CMD_WEB "<cmd>Web"

#define CMD_FIRMWAREINFO "Hugo 1.0 Rev.8\n(rel. Dec, 11, 2015)\nBattery level: %d\nBalearic Dynamics, Spain\nbalearicdynamics@gmail.com"
#define CMD_BOARDARMED "Hugo 1.0\nBoard armed: send 'Info' for updates"
#define CMD_ALREADYRUNNING "Already running (Send 'Info' for updates)"
#define CMD_BOARDSTOPPED "Board stopped. Send 'Start' to follow"
#define CMD_NOTRUNNING "Not running (Send 'Start' to init)"
#define CMD_UNKNOWN "Unknown (send: Start, Stop, Info, Quiet, Web, Collect, Help)"
#define CMD_NOINFO "Not running. Send 'Start' before"
#define CMD_HELPMSG "Commands: Start, Stop, Info, Quiet, Web, Collect, Help"
#define CMD_SETTINGS "\n(Web: %s, Quiet %s, Log %s)"
#define CMD_CHANGE_SETTINGS "\nHugo 1.0\nCurrent settings\nWeb: %s, Quiet %s, Log %s)"

#define WARNING_BATTERY "Warning, low battery!\n"
#define BATTERY_LEVEL "Batt level: %d\n"
#define TEMPERATURE "Temperature: %2.2fC\n"
#define WARNING_TEMPERATURE "OVERHEATING!!!\nTemperature %2.2fC is too high!\n"
#define MOTION "Motion status: %s\n"

// Motion statuses
#define MOTION_ON "active"
#define MOTION_OFF "inactive"

// Celsius conversion factor from analog read
#define CELSIUS_K 0.48828125
// Number of samples every temperature reading
#define TEMPERATURE_SAMPLES 10
// Max temperature limit
#define WARNING_TEMPERATURE_LIMIT 42

#define GPS_BUFF_LENGTH 256
#define GPS_REQUEST_SMS 1 // Return a string link for Googlemaps
#define GPS_REQUEST_WEB 2 // Return a string ready for sending to the server
#define NO_GPS_DATA "No GPS data present"

// Mediateck SandBox keys
#define DEVICEID "Dj89rB3z" // Input your deviceId
#define DEVICEKEY "gRTFafJI9Agklvg0" // Input your deviceKey
#define SITE_URL "api.mediatek.com"
#define SITE_PORT 80

// Server communication protocol strings
#define SERVER_GET "GET /mcs/v2/devices/"
#define SERVER_CONNECTIONS_HTTP "/connections.csv HTTP/1.1"
#define SERVER_HOST "Host: "
#define SERVER_DEVICE_KEY "deviceKey: "
#define SERVER_CONNECTION_CLOSE "Connection: close"
#define SERVER_POST "POST /mcs/v2/devices/"
#define SERVER_DATAPOINTS_HTTP "/datapoints.csv HTTP/1.1"
#define SERVER_DATALENGTH "Content-Length: "
#define SERVER_CONTENT_TYPE "Content-Type: text/csv"

// Dataset fixed strings (remote variable IDs)
#define REMOTE_BATTERY "2001,,"
#define REMOTE_POSITION "1001,,"
#define REMOTE_MOTION "3001,,"
#define REMOTE_TEMPERATURE "4001,,"

// Max number or retries on connection error
#define MAX_RETRIES 10

// Delay values during the connection exchanges (ms)
#define HTTP_RESPONSE_DELAY 100
#define WEB_CONNECTION_DELAY 1000

// Data update frequencies in ms
#define FREQ_REMOTE 10000 // (10 sec.)
#define CYCLES_BATTERY_LEVEL 90 // The number of ms is CYCLES_BATTERY_LEVEL * FREQ_REMOTE (15 min)
#define CYCLES_TEMPERATURE 15 // The number of ms is CYCLES_TEMPERATURE * FREQ_REMOTE (2.5 min)
#define CYCLES_MOTION 30 // The number of ms is CYCLES_MOTION * FREQ_REMOTE (5 min)
#define MAX_INACTIVITY_WARNING 30 // No-motion period before starting visual warning (5 min)

// Mobile access point ssid (no password, no user)
// and other constants
#define GPRS_APN "lowi.private.omv.es"
#define GPRS_POWER_DELAY 500

// Color coding struct
int rgb[8][3] = { 
  { HIGH, LOW, LOW },     // RED
  { LOW, HIGH, LOW },     // GREEN
  { LOW, LOW, HIGH },     // BLUE
  { HIGH, HIGH, LOW },    // YELLOW  
  { HIGH, LOW, HIGH },    // MAGENTA
  { LOW, HIGH, HIGH },    // CYAN
  { HIGH, HIGH, HIGH },   // WHITE
  { LOW, LOW, LOW }       // BLACK
};

struct warnings {
  boolean battery;              // Battery alert internal status
  boolean batteryAutoAlert;     // Automatic battery alert 
  boolean motion;               // Spot motion status
  boolean temperature;          // Temperature limit status
  boolean motionAlert;          // Motion alert status (samples counter)
  boolean onWeb;                // Web cloud update on/off status
  boolean isQuiet;              // RGB LED notifications on/off status
  boolean isLogging;            // Collect tracking data on a local file on/off
};

// colors structure IDs primary colors RGB
#define L_RED 0
#define L_GREEN 1
#define L_BLUE 2
// colors structure IDs colos sequence
#define C_RED 0
#define C_GREEN 1
#define C_BLUE 2
#define C_YELLOW 3
#define C_MAGENTA 4
#define C_CYAN 5
#define C_WHITE 6
#define C_BLACK 7

#define SMS_COLOR C_YELLOW
#define WEB_COLOR C_WHITE
#define TEMPERATURE_ALERT_COLOR C_MAGENTA
#define BATTERY_ALERT_COLOR C_RED
#define MOTION_ALERT_COLOR C_BLUE
#define MOTION_WARNING_COLOR C_YELLOW
#define MOVING_COLOR C_GREEN

// RGB PINs
#define RED_PIN 2
#define GREEN_PIN 4
#define BLUE_PIN 3

// Color flashing delay
#define FLASH_SHORT_SIGNAL  125
// Color alert delay
#define FLASH_ALERT_SIGNAL 500

// Min number of motion events detected in one second to activate the signal
#define MOTION_MIN_LEVEL 10

// Motion detection status
#define MOTION_PIN 5

