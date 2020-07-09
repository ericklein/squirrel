/*
  Project Name:	squirrel
  Developer:	Eric Klein Jr. (temp2@ericklein.com)
  Description:	box to detect and deter squirrels

  See README.md for target information, revision history, feature requests, etc.
*/

// Conditionals (hardware functionality)
#define DEBUG 			// debug messages to serial port
//#define ULTRASONIC	// ultrasonic sensor

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// Feather M4, M0, 328, nRF52840 or 32u4
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
// DREQ should be an Int pin *if possible* (not possible on 32u4)
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

// range finding
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
	 	Serial.println("Squirrel app launched");
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

	if (! musicPlayer.begin())
	{ // initialise the music player
    	#ifdef DEBUG
    		Serial.println("Couldn't find VS1053, do you have the right pins defined?");
		#endif
     	while (1);
  	}
	#ifdef DEBUG
  		Serial.println("VS1053 initialized");
	  	musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
	#endif

  
  	if (!SD.begin(CARDCS))
  	{
    	#ifdef DEBUG
    		Serial.println(F("SD failed, or not present"));
    	#endif
    	while (1);  // don't do anything more
  	}
  	#ifdef DEBUG
  		Serial.println("SD OK!");
		// list files
		printDirectory(SD.open("/"), 0);
	#endif
  
	// Set volume for left, right channels. lower numbers == louder volume!
	musicPlayer.setVolume(5,5);

	// If DREQ is on an interrupt pin we can do background
	// audio playing
	musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
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
    		Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
	  		distance = measure.RangeMilliMeter/10; // converting mm to cm
  		}
  		else
  		{
    		// phase failures have incorrect data
  			#ifdef DEBUG
  				Serial.println("distance measurement out of range");
			#endif
  		}
	#endif
	#ifdef DEBUG
		Serial.print("Distance to object is ");
		Serial.print(distance);
		Serial.println(" cm");
	#endif
	return (distance);
}

void annoySquirrel(int distance)
{
	// Start the loud music
	if (distance < triggerDistance)
	{
		musicPlayer.playFullFile("/track001.mp3");
		//musicPlayer.startPlayingFile("/track001.mp3");		
		#ifdef DEBUG
		Serial.println("Playing track001");
		#endif
	}
	// Pound the fence
}

/// File listing helper
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}