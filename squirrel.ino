/*
  Project Name:	squirrel
  Developer:	Eric Klein Jr. (temp2@ericklein.com)
  Description:	box to detect and deter squirrels

  See README.md for target information, revision history, feature requests, etc.
*/

// libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>
#include <TimeLib.h>

// Conditionals (hardware functionality)
#define DEBUG 			// debug messages to serial port
//#define ULTRASONIC	// ultrasonic sensor

// for battery level read code in logging function (Adafruit Feather M0 Proto specific)
#define VBATPIN A7

// SD card for logging
#define CARDCS			5     // SD Card chip select pin
File logfile;

// VS1053 for Feather M0, M4
#define VS1053_RESET	-1	// VS1053 reset pin (not used!)
#define VS1053_CS       6	// VS1053 chip select pin (output)
#define VS1053_DCS		10	// VS1053 Data/command select pin (output)
#define VS1053_DREQ     9	// VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

// distance sensor
#ifdef ULTRASONIC
	// HR-404SC
	#define triggerPin	12
	#define echoPin 	11
#else
	//VL53L0X
	#include "Adafruit_VL53L0X.h"
	Adafruit_VL53L0X lox = Adafruit_VL53L0X();
#endif
const byte triggerDistance = 20;  // Distance in cm to toggle LCD backlight

void setup()
{
	#ifdef DEBUG
		Serial.begin(115200);
		while (!Serial) 
	  	{
	    	delay(1);
	  	}
	 	Serial.println("Squirrel app started");
	#endif

	#ifdef ULTRASONIC
		//Setup ultrasonic sensor
		pinMode(triggerPin, OUTPUT);
		pinMode(echoPin, INPUT);
	#else
		if (!lox.begin())
		{
    		#ifdef DEBUG
    			Serial.println("Failed to initialize VL53L0X");
			#endif
    		while(1);
  		}
  			#ifdef DEBUG
  				Serial.println("VL53L0X initialized");
			#endif
	#endif

  	//pinMode(1, INPUT_PULLUP); // Adafruit suggested troubleshooting fix
	if (! musicPlayer.begin())
	{ // initialise the music player
    	#ifdef DEBUG
    		Serial.println("Failed to initialize VS1503");
		#endif
     	while (1); // don't do anything more
  	}
	#ifdef DEBUG
  		Serial.println("VS1053 initialized");
	  	musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
	#endif

  	// logging file
  	if (!SD.begin(CARDCS))
  	{
    	#ifdef DEBUG
    		Serial.println("SD failed, or not present");
    	#endif
    	while (1);  // don't do anything more
  	}
  	#ifdef DEBUG
  		Serial.println("SD card available");
		// list files on SD card
		printDirectory(SD.open("/"), 0);
	#endif

	// create a new log file
	char filename[] = "LOGGER00.CSV";
	for (int i = 0; i < 100; i++)
	{
		filename[6] = i/10 + '0';
		filename[7] = i%10 + '0';
		if (! SD.exists(filename)) 
		{
			// only open a new file if it doesn't exist
			logfile = SD.open(filename, FILE_WRITE); 
			break;  // leave the loop!
		}
	}
	if (! logfile)
	{
		#ifdef DEBUG
			Serial.println("could not create logfile");
		#endif
	}
	#ifdef DEBUG
		Serial.print("Logging to: ");
		Serial.println(filename);
	#endif
	// log file header row
	logfile.println("datetime,battery_level,object_distance");    

	// Set volume for left, right channels. lower numbers == louder volume!
	musicPlayer.setVolume(5,5);

	// If DREQ is on an interrupt pin we can do background audio playback
	musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int

	//set datetime; change before compile assuming battery is attached to preserve this
	// replace with RTC time
	// setTime(hr,min,sec,day,mnth,yr);
	setTime(15,41,0,9,7,2020);
}

void loop()
{
	annoySquirrel(readDistance());
	delay(1000);
}

int readDistance ()
{
    // Returns distance from sensor in centimeters
	int  distance;

	#ifdef ULTRASONIC
		long duration;
		// clears the triggerPin
		digitalWrite(triggerPin, LOW);
		delayMicroseconds(2);
		// Sets the triggerPin on HIGH state for 10 microseconds
		digitalWrite(triggerPin, HIGH);
		delayMicroseconds(10);
		digitalWrite(triggerPin, LOW);
		// Reads the echoPin, returns the sound wave travel time in microseconds
		duration = pulseIn(echoPin, HIGH);
		// Distance = (Speed of sound * Time delay) / 2
		// the speed of sound is 343.4 m/s or 0.0343 cm/microsecond to the Temperature of 20°C.
		// inches = (duration/2) / 741;
		distance = duration / 58;
	#else
		VL53L0X_RangingMeasurementData_t measure;
		lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  		if (measure.RangeStatus != 4)
  		{  
	  		distance = measure.RangeMilliMeter/10; // converting mm to cm
  		}
  		else
  		{
    		// phase failures have incorrect data
  			#ifdef DEBUG
  				Serial.print(measure.RangeMilliMeter);
  				Serial.println(" ;distance measurement out of range");
			#endif
  		}
	#endif
	return (distance);
}

void annoySquirrel(int distance)
{
	if (distance < triggerDistance)
	{
		#ifdef DEBUG
			Serial.println("Object within range");
		#endif

	  	if (musicPlayer.readyForData())
	  	{
	  		//musicPlayer.sineTest(0x44, 500);
	  		musicPlayer.playFullFile("/track001.mp3");

		#ifdef DEBUG
			Serial.println("Played music");
		#endif
				logEvent(distance);
		}
		else
		{
			Serial.println("Not ready for data");
			musicPlayer.dumpRegs();
		}
		// Pound the fence (vibration code)

	}
	else
	{
	#ifdef DEBUG
		Serial.println("No object in range");
	#endif	
	}
}

// File listing helper
void printDirectory(File dir, int numTabs)
{
   while(true)
   {
     
     File entry =  dir.openNextFile();
     if (! entry)
     {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++)
     {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory())
     {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     }
     else
     {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

void logEvent (int distance)
{
	// datetime, battery level, distance
	// datetime
	logfile.print('"');
	logfile.print(year());
	logfile.print("/");
	logfile.print(month());
	logfile.print("/");
	logfile.print(day());
	logfile.print(" ");
	logfile.print(hour());
	logfile.print(":");
	logfile.print(minute());
	logfile.print(":");
	logfile.print(second());
	logfile.print('"');
	logfile.print(",");
	#ifdef DEBUG
		Serial.print("Datetime is: ");
		Serial.print(year());
		Serial.print("/");
		Serial.print(month());
		Serial.print("/");
		Serial.print(day());
		Serial.print(" ");
		Serial.print(hour());
		Serial.print(":");
		Serial.print(minute());
		Serial.print(":");
		Serial.println(second());
	#endif
	// battery level; This is Adafruit Feather M0 Proto specific code!
	// float measuredvbat = analogRead(VBATPIN);
	float measuredvbat = 0.0;
	measuredvbat *= 2;    // we divided by 2, so multiply back
	measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
	measuredvbat /= 1024; // convert to voltage
	logfile.print(measuredvbat);
	logfile.print(",");
	#ifdef DEBUG
		Serial.print("VBat: " ); Serial.println(measuredvbat);
	#endif
	// distance
	logfile.println(distance);
	#ifdef DEBUG
		Serial.print("Distance to object is ");
		Serial.print(distance);
		Serial.println(" cm");
	#endif
	logfile.flush();
	#ifdef DEBUG
		Serial.println("log file updated");
	#endif
}