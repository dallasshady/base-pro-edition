#include "headers.h"
#include "jumper.h"
#include "imath.h"
#include "gear.h"
#include "sound.h"
#include "hud.h"
#include "npc.h"
#include "windpointer.h"
#include "../common/istring.h"
#include "unicode.h"
#include "mmsystem.h"

float timestamp = 0.0f;
//// SASHA
int cnt = 1;
float it = 4000.0f;
float it2;
float it3;
float itb=0;
boolean change = false;  
int buttonState = 0;
float diff;
float altg;
float altc;
float altagl;
float pressure;
float temp;
float halt;
float malt;
float state_data;	// stores state variables
float state_timer;	// decreases with each iteration of loop() by dt
float deltaTime;
float dropzoneAltitude; // calibrated AGL

// vertical velocity
float Vvert;
float Vvert_acc;				// change in vertical velocity
float Vvert_timer;			// timer for calculating vertical velocity every second
float Vvert_prev_altitude;	// previous altitude required for Vvert calculation

// state minimum/maximum constants
#define STATE_LIMITS_MINIMUM_CLIMB 3
#define STATE_LIMITS_MINIMUM_FREEFALL -15.0f
#define STATE_LIMITS_MINIMUM_CHOP_FREEFALL -30.0f	// minimum for re-entering freefall from canopy flight
#define STATE_LIMITS_MINIMUM_OPENING_ACC 10 
#define STATE_LIMITS_MINIMUM_OPENING_VVERT -30
#define STATE_LIMITS_MINIMUM_CANOPY_ACC 3
#define STATE_LIMITS_MINIMUM_LANDED_AGL 10
#define STATE_LIMITS_MINIMUM_LANDED_VVERT -2

// CALIBRATING		- default state. Takes 3 reading every second and calculates the average
// GROUND			- transitions from CALIBRATING after 3 seconds
// CLIMB			- transitions from GROUND or JUMPRUN, Vvert reaches at least 3 m/s (~590 ft/min) and stays for 3 seconds
//						L410 initial rate of climb: 1378 ft/min
//						AN-2 initial rate of climb: 690 ft/min
//						Skyvan initial rate of climb: 1640 ft/min
//						Pilatus Porter initial rate of climb: 941 ft/min
// JUMPRUN			- transitions from CLIMB, after altitude stays within 50m. in recent three seconds
// FREEFALL			- transitions from GROUND, CLIMB or JUMPRUN when Vvert reaches -15 m/s
// CANOPY_OPENING	- transitions from FREEFALL, CLIMB or JUMPRUN after Vver_acc reaches 10 m/s^2 for 1.5 seconds and Vvert is higher then -30 m/s
// CANOPY_FLIGHT	- transitions from any of the previous when Vvert_acc is no more than 3 m/s^2
// LANDED			- transitions when altitude 0 +/- 10m and Vver 0 +/- 5 m/s
enum ALTI_STATES {
	CALIBRATING,
	GROUND,
	CLIMB,
	JUMPRUN,
	FREEFALL,
	CANOPY_OPENING,
	CANOPY_FLIGHT,
	LANDED
};

int flight_state = CALIBRATING;		// flight state

class Adafruit_BMP085 {
public:
	float altitude;
	float pressure;
	float pressureDelta;
	float temperature;
	bool begin() {
		altitude = 0.0f;
		pressure = 0.0f;
		pressureDelta = getCore()->getRandToolkit()->getUniform(-50.0f, 50.0f);
		temperature = 15.0f;
		return true;
	}
	float readAltitude(float sealevelPressure = 101325) {
		return altitude;
	}
	float readPressure() {
		return pressure + pressureDelta;
	}
	float readTemperature() {
		return temperature;
	}
};
class LCD5110 {
public:
	wchar_t buffer[256];

	LCD5110::LCD5110() {}
	LCD5110::LCD5110(int a, int b, int c, int d, int e) {}
	void InitLCD(int a) {
		clrScr();
	}
	void invert(bool a) {}
	void setFont(unsigned int a) {}
	void print (wchar_t *a, int b, int c) {
		wcscat(buffer, a);
	}
	void printNumI(int number, int b, int c) {
		wchar_t a[32];
		swprintf(a, 256, L"%i", number);
		wcscat(buffer, a);
	}
	void printNumF(float number, int decimals, int b, int c) {
		wchar_t a[32];
		swprintf(a, 256, L"%4.2f", number);
		wcscat(buffer, a);
	}
	void clrScr() {
		for (int i = 0; i < 256; ++i) {
			buffer[i] = '\0';
		}
	};
};
class serial {
public:
	void begin(int a) {}
	void write(char *a) {}
	void print(char *str) {}
	void println(char *str) {}
	void println(int str) {}
	void println(double str) {}
	void println(bool *str) {}
} Serial;

// STUBS
enum STUBS {
	//INPUT = 0,
	HIGH,
	LOW
};
void pinMode(int a, int b) {}
void delay(int a) {}
int digitalRead(int a) {
	return LOW;
}
float getDeltaTime() {
	return deltaTime;
}

Adafruit_BMP085 bmp;
int alt_read();
void disp_alt(); 

LCD5110 myGLCD(7,6,5,3,4);
unsigned int SmallFont = 0;
unsigned int MediumNumbers = 0;
unsigned int BigNumbers = 0;
unsigned int font1 = 0;
void setup() {
	//Button input pin  
	pinMode(8, 0);

	myGLCD.InitLCD(60);
	myGLCD.invert(false);
	//contrast 0-127  
	// myGLCD.setContrast(60);

	//bmp85  
	Serial.begin(9600);
	Serial.write("SETUP!!!");
	if (!bmp.begin()) {
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
		myGLCD.setFont(SmallFont);
		myGLCD.print(L"Sensor error",12,30);

		//while (1) {}
	}

	// Vvert calculation
	Vvert = 0.0f;
	Vvert_prev_altitude = bmp.readAltitude();
	Vvert_timer = 1.0f;

	deltaTime = 0.0f;
	// initial state is calibration. give it one seconds to take the first reading
	flight_state = CALIBRATING;
	state_timer = 1.0f;	
	// reset calibration readings
	state_data = 0.0f;
	altg = bmp.readAltitude();    

	//one time alt display                   
	alt_read();                   
	disp_alt(); 
}
float debug = 0.0f;
void update_flightstate() {
	// advance timer
	if (state_timer > 0) {
		state_timer -= getDeltaTime();
	}

	// Calibration state
	// state_data - stores number of readings taken
	if (flight_state == CALIBRATING) {
		if (state_timer <= 0.0f) {
			dropzoneAltitude += bmp.readAltitude();
			state_data += 1.0f;
			// three readings taken?
			if (state_data == 3.0f) {
				dropzoneAltitude /= 3.0f;
				// set data for next state
				state_data = 0;
				state_timer = 1.0f;
				flight_state = GROUND;
				Vvert = 0.0f;
			// wait for another reading in one second
			} else {
				state_timer = 1.0f;
			}
		}

	// ground state (read altitude change every second)
	// state_data	- store number of readings (seconds) Vvert was higher than minimum for climb
	} else if (flight_state == GROUND) {
		// at least a second has passed
		if (state_timer <= 0.0f) {
			if (Vvert >= STATE_LIMITS_MINIMUM_CLIMB || Vvert <= STATE_LIMITS_MINIMUM_FREEFALL) {
				// count reading as possible climb
				state_data += 1.0f;
			} else {
				// reset number of readings if Vvert is too low
				state_data = 0.0f;
			}
			// enought to enter climb?
			if (state_data >= 2.5f) {	// 2.5f for possible float errors
				// set data for next state
				state_data = 0.0f;
				state_timer = 1.0f;
				if (Vvert <= STATE_LIMITS_MINIMUM_FREEFALL) {
					flight_state = FREEFALL;
				} else {
					flight_state = CLIMB;
				}
			// continue to one more second
			} else {
				// reset timer
				state_timer += 1.0f;
			}
		}
	// climb state (read altitude change every second and wait for JUMPRUN of FREEFALL)
	// state_data	- store number of readings (seconds) Vvert was lower than minimum for FREEFALL 
	//				  or low enough for JUMPRUN
	} else if (flight_state == CLIMB) {
		// at least a second has passed
		if (state_timer <= 0.0f) {
			if (Vvert <= STATE_LIMITS_MINIMUM_FREEFALL || Vvert >= -STATE_LIMITS_MINIMUM_CLIMB && Vvert <= STATE_LIMITS_MINIMUM_CLIMB) {
				// count reading as possible freefall or jumprun
				state_data += 1.0f;
			} else {
				// reset number of readings if Vvert is too high
				state_data = 0.0f;
			}

			// enought to enter freefall?
			if (state_data >= 1.5f && Vvert <= STATE_LIMITS_MINIMUM_FREEFALL) {	// 1.5f for possible float errors
				// reset data for next state (state_data will be used for altitude in CLIMB state, so don't touch
				state_data = 0.0f;
				state_timer = 0.0f;
				flight_state = FREEFALL;
			// enought to enter jumprun?
			} else if (state_data >= 1.5f) {	// 1.5f for possible float errors
				state_timer = 0.0f;
				flight_state = JUMPRUN;
			// continue to one more second
			} else {
				// reset timer
				state_timer += 1.0f;
			}
		}
	// jumprun state (read altitude change every second and wait for JUMPRUN of FREEFALL)
	// state_data	- store number of readings (seconds) Vvert was lower than minimum for FREEFALL 
	//				  or high enough for CLIMB
	} else if (flight_state == JUMPRUN) {
		if (Vvert <= STATE_LIMITS_MINIMUM_FREEFALL || Vvert >= STATE_LIMITS_MINIMUM_CLIMB) {
			// count reading as possible freefall or climb
			state_data += 1.0f;
		} else {
			// reset number of readings if Vvert is not useful
			state_data = 0.0f;
		}

		// enought to enter freefall?
		if (state_data >= 1.5f && Vvert <= STATE_LIMITS_MINIMUM_FREEFALL) {	// 1.5f for possible float errors
			// reset data for next state (state_data will be used for altitude in CLIMB state, so don't touch
			state_data = 0.0f;
			state_timer = 0.0f;
			flight_state = FREEFALL;
		// enought to enter climb?
		} else if (state_data >= 1.5f && Vvert >= STATE_LIMITS_MINIMUM_CLIMB) {	// 1.5f for possible float errors
			state_timer = 0.0f;
			flight_state = CLIMB;
		}

	// freefall state (read vertical velocity change every second and wait for CANOPY OPENING)
	} else if (flight_state == FREEFALL) {
		if (Vvert_acc >= STATE_LIMITS_MINIMUM_OPENING_ACC || Vvert >= STATE_LIMITS_MINIMUM_OPENING_VVERT) {
			// count reading as possible canopy opening
			state_data += 1.0f;
		} else {
			// reset number of readings if Vvert is not useful
			state_data = 0.0f;
		}

		// enought to enter canopy opening?
		if (state_data >= 3.5f) {	// 3.5f for possible float errors
			// reset data for next state (state_data will be used for altitude in CLIMB state, so don't touch
			state_data = 0.0f;
			state_timer = 0.0f;
			flight_state = CANOPY_OPENING;
		}

	// canopy opening state (read vertical velocity change every second and wait for CANOPY OPENING)
	} else if (flight_state == CANOPY_OPENING) {
		if (Vvert_acc <= STATE_LIMITS_MINIMUM_CANOPY_ACC) {
			// count reading as possible canopy opening
			state_data += 1.0f;
		} else {
			// reset number of readings if Vvert is not useful
			state_data = 0.0f;
		}

		// enought to enter canopy flight?
		if (state_data >= 1.5f && Vvert_acc <= STATE_LIMITS_MINIMUM_CANOPY_ACC) {	// 1.5f for possible float errors
			state_data = 0.0f;
			state_timer = 0.0f;
			flight_state = CANOPY_FLIGHT;
		}
	// canopy opening state (read vertical velocity change every second and wait for LANDED or FREEFALL)
	} else if (flight_state == CANOPY_FLIGHT) {
		if (Vvert <= STATE_LIMITS_MINIMUM_CHOP_FREEFALL) {
			// count reading as possible freefall or climb
			state_data += 1.0f;
		} else {
			// reset number of readings if Vvert is not useful
			state_data = 0.0f;
		}

		// enought to enter freefall?
		if (state_data >= 1.5f) {	// 1.5f for possible float errors
			// reset data for next state (state_data will be used for altitude in CLIMB state, so don't touch
			state_data = 0.0f;
			state_timer = 0.0f;
			flight_state = FREEFALL;
		}

		// go for landed
		if (it3 < STATE_LIMITS_MINIMUM_LANDED_AGL && Vvert >= STATE_LIMITS_MINIMUM_LANDED_VVERT) {
			state_data = 0.0f;
			state_timer = 0.0f;
			flight_state = LANDED;
		}
	}
}

void calculate_Vvert() {
	//Vvert_acc = Vvert;
	//Vvert = (bmp.readAltitude() - Vvert_prev_altitude) / getDeltaTime();
	//Vvert_acc = Vvert - Vvert_acc;

	//Vvert_prev_altitude = bmp.readAltitude();

	// every second
	Vvert_timer -= getDeltaTime();

	if (Vvert_timer <= 0.0f) {
		Vvert_acc = Vvert;
		// calculate new Vvert
		const float Vvert_new = (bmp.readAltitude() - Vvert_prev_altitude);
		// de-noise
		const float Vvert_delta = (Vvert_new - Vvert) * 0.3f;
		Vvert += Vvert_delta;

		Vvert_prev_altitude = bmp.readAltitude();
		Vvert_timer += 1.0f;
		
		// acceleration
		Vvert_acc = Vvert - Vvert_acc;
	}
}
//Main data display function
void disp_alt() {
	myGLCD.clrScr();
	myGLCD.setFont(SmallFont);
	//font1 or SmallFont

	// print flight state
	if (flight_state == CALIBRATING) {
		myGLCD.print(L"CALIBRATING ",1,1);
		myGLCD.printNumF(state_timer,2, 21, 22);

	} else if (flight_state == GROUND) {
		myGLCD.print(L"GROUND ",1,1);
		myGLCD.printNumF(Vvert,2, 21, 22);
		myGLCD.print(L" m/s",1,1);

	} else if (flight_state == CLIMB) {
		myGLCD.print(L"CLIMB ",1,1);
		myGLCD.printNumF(Vvert,2, 21, 22);
		myGLCD.print(L" m/s",1,1);

	} else if (flight_state == JUMPRUN) {
		myGLCD.print(L"JUMPRUN",1,1);

	} else if (flight_state == FREEFALL) {
		myGLCD.print(L"FREEFALL ",1,1);
		myGLCD.printNumF(Vvert, 2, 21, 22);
		myGLCD.print(L" m/s",1,1);

	} else if (flight_state == CANOPY_OPENING) {
		myGLCD.print(L"CANOPY OPENING ",1,1);
		myGLCD.printNumF(Vvert_acc, 2, 21, 22);
		myGLCD.print(L" m/s^2",1,1);

	} else if (flight_state == CANOPY_FLIGHT) {
		myGLCD.print(L"CANOPY FLIGHT ",1,1);
		myGLCD.printNumF(Vvert, 2, 21, 22);
		myGLCD.print(L" m/s", 1, 1);

	} else if (flight_state == LANDED) {
		myGLCD.print(L"LANDED", 1, 1);
	} 

	//dt 
	myGLCD.print(L"\ndT: ",1,1);
	myGLCD.printNumF(getDeltaTime(), 2, 21, 22);
	myGLCD.print(L" s\n",55,1);
	//agl 
	myGLCD.print(L"AGL: ",1,1);
	myGLCD.printNumF(it3, 2, 21, 22);
	myGLCD.print(L" m\n",55,1);
	//asl
	myGLCD.print(L"ASL: ",1,12);
	myGLCD.printNumF(altc, 2, 21, 22);
	myGLCD.print(L" m\n",55,12);
	//barometric pressure
	myGLCD.print(L"BP:",1,22);
	myGLCD.printNumF(pressure,2, 21, 22);
	myGLCD.print(L" hPa\n",66,22);
	//Temperature
	myGLCD.print(L"Temp.:",1,29);
	myGLCD.printNumF(temp,2,40,29);
	myGLCD.print(L" C\n",72,29);
	// Serial.println(pressure);

	Serial.println(it3);
	Serial.println(change); 
}

//Button pressed function
int button() {
   buttonState = digitalRead(8);
   if(buttonState == HIGH) {
	   change=true; 
	   cnt=cnt+1;
	   myGLCD.clrScr();
	   myGLCD.setFont(BigNumbers);
	   myGLCD.printNumF(halt,1,10,15);
	   Serial.println("Pushed");
	   Serial.println(halt);
   } else if (buttonState == LOW) {
	   cnt=0;
	   return 0;
   }
   return 0;
}

//Altitude read function
int alt_read() {
	altc = (bmp.readAltitude(103000));
	altagl = altc-altg;
	if (altagl<0) {
		altagl=0;
	}
	it3 = altagl;
	Serial.println(it3);
	pressure = bmp.readPressure();
	pressure = pressure/100;
	Serial.println(pressure);
	temp = bmp.readTemperature();
	Serial.println(temp);
	halt = (float)altagl/100.0f;
	malt = altagl;
	return 0;
}

int rounder() {
	diff = it3 - itb;
	Serial.println(diff);
	diff = diff*diff;
	Serial.println(diff);
	diff = sqrt(diff);
	Serial.println(diff);
	delay(5);
	return 0;
}

int cntcyc() {
	buttonState = digitalRead(8);
	// if(buttonState == HIGH){
	// cnt=cnt+1;}
	if(cnt>3){cnt=1;}
	delay(20);
	return 0;
}
void big1() {
	myGLCD.clrScr();
	myGLCD.setFont(BigNumbers);
	myGLCD.printNumF(halt, 1, 10, 15);
}
void big2() {
	myGLCD.clrScr();
	myGLCD.setFont(BigNumbers);
	myGLCD.printNumF(malt, 2, 21, 22);
}



void loop() {
	alt_read();
	rounder();
	cntcyc();
	if (flight_state != CALIBRATING) {
		calculate_Vvert();
	}
	update_flightstate();

	//need to add return from button press screen update!!! 
	//set diff>0 to refresh on changes of 1
	// if(diff>0 || change==true){
	Serial.print("CNT: ");  
	Serial.println(cnt);
	//=================================Count one updates
	if (cnt==1 && diff>0) {
		disp_alt();
		delay(20);
	}
	else if(cnt==1 && buttonState == HIGH) {
		big1();
		cnt++;
		Serial.println("CNT1 button +");
		delay(20);
	}
	//====================================Count two updates
	else if(cnt==2 && diff>0) {
		big1();   
		delay(20);
	} else if(cnt==2 && buttonState == HIGH) {
		big2();   
		cnt++;
		Serial.println("CNT2 button +");
		delay(20);
	}
	//======================================Count three count  
	else if (cnt==3 && diff>0) {
		big2();   
		delay(20);

	} else if (cnt == 3 && buttonState == HIGH) {
		disp_alt();  
		cnt++; 
		Serial.println("CNT3 button +");
		delay(20);
	}
	itb=it3;
}
///// END SASHA



/**
 * transparent list of jumpers (for some code tricks)
 */

Jumper::JumperL Jumper::_jumperL;

/**
 * internal jumper properties
 */

const float glanceRayDistance      = 1000.0f;
const float glanceLoweringStep     = -5.0f;
const float headLoweringLimit      = 50.0f;
const float headLoweringVelocity   = 90.0f;
const float backboneDeviationLimit = 7.0f;

const unsigned int bcStepout = 10;

/**
 * base jumper idle sequence
 */

static engine::AnimSequence idleSequence = 
{ 
    FRAMETIME(1), 
    FRAMETIME(59), 
    engine::ltPeriodic, 
    FRAMETIME(1)
    //FRAMETIME(1824), 
    //FRAMETIME(1825), 
    //engine::ltPeriodic, 
    //FRAMETIME(1824)
};

/**
 * front flip jump
 */

static engine::AnimSequence frontFlipSequence = 
{ 
    FRAMETIME(1291),
    FRAMETIME(1305),
    engine::ltNone,
    0.0f
};

const float frontFlipAnimSpeed      = 0.5f;
const float frontFlipCriticalPeriod = ( FRAMETIME(1302) - FRAMETIME(1291) ) / frontFlipAnimSpeed;
// NOTE: some D3 bug probably: 1305 != 1302. WRONG! First member in critial must be 3 frames less!

/**
 * front hop hop flip jump
 */

static engine::AnimSequence frontHopHopFlipSequence = 
{ 
    FRAMETIME(1232),
    FRAMETIME(1305),
    engine::ltNone,
    0.0f
};

const float frontHopHopFlipAnimSpeed      = 0.5f;
const float frontHopHopFlipCriticalPeriod = ( FRAMETIME(1302) - FRAMETIME(1232) ) / frontHopHopFlipAnimSpeed;

/**
 * side step left jump
 */

static engine::AnimSequence sideStepLeftJumpSequence = 
{ 
    FRAMETIME(1684),
    FRAMETIME(1731),
    engine::ltNone,
    0.0f
};

const float sideStepLeftJumpAnimSpeed      = 0.5f;
const float sideStepLeftJumpCriticalPeriod = ( FRAMETIME(1731) - FRAMETIME(1684) ) / sideStepLeftJumpAnimSpeed;

/**
 * front-back flip jump
 */

static engine::AnimSequence frontBackFlipSequence = 
{ 
    FRAMETIME(1381),
    FRAMETIME(1416),
    engine::ltNone,
    0.0f
};

const float frontBackFlipAnimSpeed      = 0.5f;
const float frontBackFlipCriticalPeriod = ( FRAMETIME(1413) - FRAMETIME(1381) ) / frontBackFlipAnimSpeed;

static engine::AnimSequence runningBackFlipSequence = 
{ 
    FRAMETIME(1403), 
    FRAMETIME(1416),  
    engine::ltNone, 
    0.0f 
};
const float runningBackFlipAnimSpeed      = 0.45f;
const float runningBackFlipCriticalPeriod = ( FRAMETIME(1416) - FRAMETIME(1403) ) / runningBackFlipAnimSpeed;

/**
 * back flip jump
 */

static engine::AnimSequence backFlipSequence = 
{ 
    FRAMETIME(1870),
    FRAMETIME(1902),
    engine::ltNone,
    0.0f
};

const float backFlipAnimSpeed      = 0.5f;
const float backFlipCriticalPeriod = ( FRAMETIME(1899) - FRAMETIME(1870) ) / backFlipAnimSpeed;

/**
 * jumper action
 */

Jumper::JumperAction::JumperAction(Jumper* jumper) :
    Character::Action( jumper->getClump() ), _jumper( jumper )
{
}

/**
 * class implementation
 */

static engine::IAtomic* setJumperUpdateTresholdCB(engine::IAtomic* atomic, void* data)
{
    atomic->setUpdateTreshold( 0.0f, 0.0f );
    return atomic;
}

Jumper::Jumper(Actor* parent, Airplane* airplane, Enclosure* enclosure, Virtues* virtues, SpinalCord* spinalCord, Virtues::Equipment* forcedEquipment) : 
    Character( parent, Gameplay::iGameplay->findClump( "BaseJumper01" )->clone( "Jumper" ) )
{    
	_phase = jpCreating;
	
	setup();

    _name = "Jumper";    
    _clump->forAllAtomics( setJumperUpdateTresholdCB, NULL );
    _clump->getFrame()->setMatrix( Matrix4f( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 ) );
    _clump->getFrame()->getLTM();
    _isStuck = false;
    _isOverActivity = false;
    _jumpTime = 0.0f;
    _jumpPose.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
	_dt = 0.0f;
	_phJoint = NULL;
	_altimeter = NULL;
	_variometer = NULL;
	_inDropzone = false;
	_default_pose = 0;
	
	// tandem stuff
	_tandemStudent = NULL;
	for (int i = 0; i < 4; ++i) _tandemJoint[i] = NULL;

    // store enclosure
    _enclosure = enclosure;
    _airplane = airplane;
    _airplaneExit = NULL;

    if( _enclosure ) assert( _airplane == NULL );
    if( _airplane ) assert( _enclosure == NULL );

    // setup burden calculation
    _bcStep    = 0;
    _bcPrevVel = NxVec3( 0,0,0 );
    _bcBurden  = NxVec3( 0,-9.8f,0 );

    // setup health status
    _distanceToAbyss = -1.0f;
    _jerkLimit = getCore()->getRandToolkit()->getUniform( 0.125f, 0.33f );
    _fallLimit = getCore()->getRandToolkit()->getUniform( 0.33f, 0.66f );
    _adrenaline = 0.0f;
    _pulse = 60.0f;
    _hazardState = false;
    _shock = 0.0f;  
    _signatureType = stFull;
    _phaseIsEnabled = true;

    // calculate somersault matrix conversion
    engine::IAnimationController* animCtrl = _clump->getAnimationController();
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }
    animCtrl->setTrackAnimation( 0, Jumper::Flip::getFlipSequence() );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.75f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );
    _clump->getFrame()->getLTM();
    _mcPelvisToClump.setup( getPelvisFrame( _clump )->getLTM(), _clump->getFrame()->getLTM() );
    _mcChestToClump.setup( getChestFrame( _clump )->getLTM(), _clump->getFrame()->getLTM() );

    // setup procedural animation
    _headIncidence = 0.0f;
    _angleLR = _angleUD = 0.0f;
    _isContacted = true;

    // load virtues
    if( virtues )
    {
        _player     = false;
        _virtues    = virtues;
        _spinalCord = spinalCord;
    }
    else
    {
        _player     = true;
        _virtues    = _scene->getCareer()->getVirtues();
        _spinalCord = new SpinalCord();
		//getScene()->network->beginSending();
    }

	// debug msg window
	_debug_window = NULL;
	if (_player) {
		_debug_window = Gameplay::iGui->createWindow( "Instructor" ); assert( _debug_window );
		Gameplay::iGui->getDesktop()->insertPanel( _debug_window->getPanel() );
		_debug_window->align( gui::atCenter, 0, gui::atLeft, 0 );
	}

    // use forced equipment?
    if( forcedEquipment )
    {
        _useForcedEquipment = true;
        _layoffEquipment = _virtues->equipment;
        _virtues->equipment = *forcedEquipment;
    }
    else
    {
        _useForcedEquipment = false;
        // gear ageing
        _virtues->equipment.helmet.age++;
        _virtues->equipment.suit.age++;
        _virtues->equipment.rig.age++;
    }

    // scale model to obtain desired height
    Matrix4f m = _clump->getFrame()->getMatrix();
    m[0][0] *= 0.01f * _virtues->appearance.height, 
    m[0][1] *= 0.01f * _virtues->appearance.height, 
    m[0][2] *= 0.01f * _virtues->appearance.height,
    m[1][0] *= 0.01f * _virtues->appearance.height, 
    m[1][1] *= 0.01f * _virtues->appearance.height, 
    m[1][2] *= 0.01f * _virtues->appearance.height,
    m[2][0] *= 0.01f * _virtues->appearance.height, 
    m[2][1] *= 0.01f * _virtues->appearance.height, 
    m[2][2] *= 0.01f * _virtues->appearance.height;
    _clump->getFrame()->setMatrix( m );
    _clump->getFrame()->getLTM();

    // calculate local anchors for canopy connection
    Matrix4f childLTM  = Jumper::getPhysicsJointFrontLeft( _clump )->getLTM(); orthoNormalize( childLTM );
    Matrix4f parentLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    MatrixConversion mc;
    mc.setup( parentLTM, childLTM );
    Matrix4f childM = mc.getTransformation();
    _frontLeftAnchor = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );
    childLTM  = Jumper::getPhysicsJointFrontRight( _clump )->getLTM(); orthoNormalize( childLTM );
    parentLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _frontRightAnchor = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );
    childLTM  = Jumper::getPhysicsJointRearLeft( _clump )->getLTM(); orthoNormalize( childLTM );
    parentLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _rearLeftAnchor = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );
    childLTM  = Jumper::getPhysicsJointRearRight( _clump )->getLTM(); orthoNormalize( childLTM );
    parentLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _rearRightAnchor = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );

    // calculate pilot anchor
    childLTM  = Jumper::getLineRigJoint( _clump )->getLTM(); orthoNormalize( childLTM );
    parentLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM(); orthoNormalize( parentLTM );
    mc.setup( parentLTM, childLTM );
    childM = mc.getTransformation();
    _pilotAnchor = wrap( Vector3f( childM[3][0], childM[3][1], childM[3][2] ) );

    // initialize free fall physics
    NxBodyDesc nxBodyDesc;
    nxBodyDesc.massSpaceInertia.set( 0,0,0 ); // tensor will be computed automatically
    nxBodyDesc.mass = _virtues->appearance.weight;
	//getCore()->logMessage("mass: %2.2f", _virtues->appearance.weight); //65

    nxBodyDesc.linearDamping = 0.0f;
    nxBodyDesc.angularDamping = 2.0f;
    nxBodyDesc.flags = NX_BF_VISUALIZATION;    
    nxBodyDesc.solverIterationCount = 16;
    Vector3f aabbScale(
        Jumper::getCollisionFF( _clump )->getFrame()->getRight().length(),
        Jumper::getCollisionFF( _clump )->getFrame()->getUp().length(),
        Jumper::getCollisionFF( _clump )->getFrame()->getAt().length()
    );
    Vector3f aabbInf = Jumper::getCollisionFF( _clump )->getGeometry()->getAABBInf();
    Vector3f aabbSup = Jumper::getCollisionFF( _clump )->getGeometry()->getAABBSup();
    Vector3f aabbDim = ( aabbSup - aabbInf );
    aabbDim[0] *= aabbScale[0] * 0.5f,
    aabbDim[1] *= aabbScale[1] * 0.5f,
    aabbDim[2] *= aabbScale[2] * 0.5f;
    
    NxCapsuleShapeDesc nxFreefallDesc;
    nxFreefallDesc.setToDefault();
    nxFreefallDesc.radius = 0.01f * ( aabbDim[0] > aabbDim[1] ? aabbDim[0] : aabbDim[1] );
    nxFreefallDesc.height = 0.01f * ( 2 * aabbDim[2] );
    nxFreefallDesc.height -= 2 * nxFreefallDesc.radius;
    if( nxFreefallDesc.height < 0 ) nxFreefallDesc.height = 0;
    
    Matrix4f localPose( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
    localPose = Gameplay::iEngine->rotateMatrix( localPose, Vector3f( 1,0,0 ), 90.0f );
    nxFreefallDesc.localPose = wrap( localPose );
    nxFreefallDesc.materialIndex = _scene->getPhFleshMaterial()->getMaterialIndex();

    NxActorDesc nxActorDesc;
    nxActorDesc.userData = this;
    nxActorDesc.shapes.pushBack( &nxFreefallDesc );
    nxActorDesc.body = &nxBodyDesc;
    _phFreeFall = _scene->getPhScene()->createActor( nxActorDesc );
    assert( _phFreeFall );
    _phFreeFall->putToSleep();
    _phFreeFall->raiseActorFlag( NX_AF_DISABLE_RESPONSE );
    unsigned int flags = _scene->getPhScene()->getActorPairFlags( *_scene->getPhTerrain(), *_phFreeFall );
    flags = flags | NX_NOTIFY_ON_START_TOUCH;
    flags = flags | NX_NOTIFY_ON_TOUCH;
    flags = flags | NX_NOTIFY_ON_END_TOUCH;
    _scene->getPhScene()->setActorPairFlags( *_scene->getPhTerrain(), *_phFreeFall, flags );
    
    // initialize free fall PTV transformation
    Matrix4f freeFallLTM = Jumper::getCollisionFF( _clump )->getFrame()->getLTM();
    Matrix4f viewLTM  = _clump->getFrame()->getLTM();
    _mcFreeFall.setup( freeFallLTM, viewLTM );

    // initialize flight physics
    nxBodyDesc.setToDefault();
    nxBodyDesc.massSpaceInertia.set( 0,0,0 ); // tensor will be computed automatically
    nxBodyDesc.mass = _virtues->appearance.weight;
	//getCore()->logMessage("weuight: %2.2f",  _virtues->appearance.weight);
    nxBodyDesc.linearDamping = 0.0f;
    nxBodyDesc.angularDamping = 0.25f;
    nxBodyDesc.flags = NX_BF_VISUALIZATION;    
    nxBodyDesc.solverIterationCount = 32;
    aabbScale = Vector3f(
        Jumper::getCollisionFC( _clump )->getFrame()->getRight().length(),
        Jumper::getCollisionFC( _clump )->getFrame()->getUp().length(),
        Jumper::getCollisionFC( _clump )->getFrame()->getAt().length()
    );
    aabbInf = Jumper::getCollisionFC( _clump )->getGeometry()->getAABBInf();
    aabbSup = Jumper::getCollisionFC( _clump )->getGeometry()->getAABBSup();
    aabbDim = aabbSup - aabbInf;
    aabbDim[0] *= aabbScale[0] * 0.5f,
    aabbDim[1] *= aabbScale[1] * 0.5f,
    aabbDim[2] *= aabbScale[2] * 0.5f;
    NxCapsuleShapeDesc nxCapsuleDesc;
    nxCapsuleDesc.setToDefault();
    nxCapsuleDesc.radius = 0.01f * ( aabbDim[0] > aabbDim[2] ? aabbDim[0] : aabbDim[2] );
    nxCapsuleDesc.height = 0.01f * ( 2 * aabbDim[1] );
    nxCapsuleDesc.height -= 2 * nxCapsuleDesc.radius;
    if( nxCapsuleDesc.height < 0 ) nxCapsuleDesc.height = 0;

    nxCapsuleDesc.materialIndex = _scene->getPhMovingFleshMaterial()->getMaterialIndex();
    nxActorDesc.setToDefault();
    nxActorDesc.userData = this;
    nxActorDesc.shapes.pushBack( &nxCapsuleDesc );
    nxActorDesc.body = &nxBodyDesc;
    _phFlight = _scene->getPhScene()->createActor( nxActorDesc );    
    assert( _phFlight );
    _phFlight->putToSleep();
    flags = _scene->getPhScene()->getActorPairFlags( *_scene->getPhTerrain(), *_phFlight );
    flags = flags | NX_NOTIFY_ON_TOUCH;
    _scene->getPhScene()->setActorPairFlags( *_scene->getPhTerrain(), *_phFlight, flags );

    // initialize flight PTV transformation
    Matrix4f flightLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM();
    viewLTM  = _clump->getFrame()->getLTM();
    _mcFlight.setup( flightLTM, viewLTM );
    
    if( _enclosure )
    {
        // positioning in enclosure
        unsigned int markerId = _enclosure->getRandomRespawnMarker();
        Matrix4f posMatrix = _enclosure->getMarkerFrame( markerId )->getLTM();
        Vector3f pos( posMatrix[3][0], posMatrix[3][1], posMatrix[3][2] );

        // avoid position conflict
        unsigned int attemptId = 0;
        unsigned int attemptLimit = 100;
        while( Jumper::getPositionConflict( pos ) )
        {
            markerId = _enclosure->getRandomRespawnMarker();
            posMatrix = _enclosure->getMarkerFrame( markerId )->getLTM();
            pos = Vector3f ( posMatrix[3][0], posMatrix[3][1], posMatrix[3][2] );
            attemptId++;
            if( attemptId > attemptLimit ) break;
        }

        Vector3f at( posMatrix[2][0], posMatrix[2][1], posMatrix[2][2] );    
        Vector3f clumpAt = _clump->getFrame()->getAt();        
        at.normalize();
        clumpAt.normalize();
        float angle = calcAngle( clumpAt, at, Vector3f(0,1,0) );
    
        // let jumper stands in enclosure by feets
        pos = _enclosure->move( pos, Vector3f( 0,-100,0 ), jumperRoamingSphereSize );
        pos += Vector3f( 0, -jumperRoamingSphereSize, 0 );
        _clump->getFrame()->setPos( Vector3f(0,0,0) );
        _clump->getFrame()->rotate( Vector3f(0,1,0), -angle );
        _clump->getFrame()->setPos( pos );

        // setup idle action
        //_phase = jpRoaming;
        _action = new Character::Idle( _clump, &idleSequence, 0.2f, 0.5f );
    }

    if( _airplane )
    {
        // setup airplane exit point
        _airplaneExit = airplane->occupyRandomExitPoint();
        assert( _airplaneExit );
        const char* name = _airplaneExit->getName();

        // setup airplane idle action
        //_phase = jpRoaming;
        _action = new AirplaneIdle( this );
    }

    // hide effectors & risers
    hideEffectors( _clump );
    getRisers( _clump )->setFlags( 0 );

    // local sensor
    _sensor = new Sensor;

    // choose appearance preset    
    setHead( _clump, database::Face::getRecord( _virtues->appearance.face )->modelId, this );
    setHelmet( _clump, database::Helmet::getRecord( _virtues->equipment.helmet.id )->modelId, this );
    setBody( _clump, database::Suit::getRecord( _virtues->equipment.suit.id )->modelId, this );

    // setup render callback
    assert( _virtues->equipment.helmet.type == gtHelmet );
    assert( _virtues->equipment.suit.type == gtSuit );
    assert( _virtues->equipment.rig.type == gtRig );
    assert( _virtues->equipment.canopy.type == gtCanopy );
    _renderCallback.setFace( _virtues->appearance.face );
    _renderCallback.setHelmet( _virtues->equipment.helmet.id );
    _renderCallback.setSuit( _virtues->equipment.suit.id );
    _renderCallback.setRig( _virtues->equipment.rig.id );
    _renderCallback.apply( _clump );
    _riserCallback.apply( _clump );
    _riserCallback.showSlider = ( _virtues->equipment.sliderOption == ::soDown );

    // force LICENSED_CHAR appearance
    if( _player && _scene->getCareer()->getLicensedFlag())
    {
        setLicensedCharacterAppearance();
        setNoHelmet( _clump );
    }

    // save ghost telemetry for AI
    if( _player ) 
    {
        _saveGhost = CatToy::saveGhostCatToy( this, "./usr/cattoys/lastghost.cattoy" );
    }
    else
    {
        _saveGhost = NULL;
    }

    // create HUD actors
    if( _player )
    {
        _altimeter = new Altimeter( this );
        _variometer = new Variometer( this );
        new Timer( this, EVENT_DELAY_START, EVENT_DELAY_STOP, L"DELAY" );
        new HealthStatus( this );
    }

    // create wind pointer
    if( _player )
    {
        // wind is actual?
        if( database::LocationInfo::getRecord( _scene->getLocation()->getDatabaseId() )->wind )
        {
            new WindPointer( this );
        }
    }

    // create signature window
    _signature = Gameplay::iGui->createWindow( "JumperSignature" ); assert( _signature );
    Gameplay::iGui->getDesktop()->insertPanel( _signature->getPanel() );

    // create footsteps sound actor
    if( _player ) new FootstepsSound( this );

    // create freefall sound actor
    if( _player ) new FreefallSound( this );

    // create cry sound
    if( _player ) new CrySound( this );

    // gear of licensed character won't worn out
    if( _scene->getCareer()->getLicensedFlag() )
    {
        _virtues->equipment.helmet.state = 1.0f;
        _virtues->equipment.rig.state    = 1.0f;
        _virtues->equipment.suit.state   = 1.0f;
        _virtues->equipment.canopy.state = 1.0f;
    }

    // create canopy simulator
    _canopySimulator = new CanopySimulator( 
        this, 
        &_virtues->equipment.canopy,
        ( _virtues->equipment.sliderOption == ::soUp )
    );

    // create pilotchute simulator
    database::Canopy* canopyInfo = database::Canopy::getRecord( _virtues->equipment.canopy.id );
    assert( canopyInfo->numPilots > _virtues->equipment.pilotchute );
    database::Pilotchute* pcInfo = canopyInfo->pilots + _virtues->equipment.pilotchute;
    _pilotchuteSimulator = new PilotchuteSimulator( this, canopyInfo, pcInfo );

    // create reserve canopy simulator
	if (_virtues->equipment.reserve.type != gtUnequipped ) {
		_canopyReserveSimulator = new CanopySimulator( 
			this, 
			&_virtues->equipment.reserve,
			( _virtues->equipment.sliderOption == ::soUp )
		);
		// create reserve pilotchute simulator
		database::Canopy* reserveInfo = database::Canopy::getReserveRecord( _virtues->equipment.reserve.id );
		database::Pilotchute* pcRInfo = reserveInfo->pilots;
		_pilotchuteReserveSimulator = new PilotchuteSimulator( this, reserveInfo, pcRInfo );
		_pilotchuteReserveSimulator->setFreebag(true);
	} else {
		_canopyReserveSimulator = NULL;
		_pilotchuteReserveSimulator = NULL;
	}

    // register in tricklist
    _jumperL.push_back( this );

	// Special options?
	// PC in hand
	if (_player && 
		!database::Canopy::getRecord(_virtues->equipment.canopy.id)->skydiving &&
		_virtues->equipment.rig.rig_aad == 1) {

		_pilotchuteSimulator->connect(_phFreeFall, Jumper::getLineRigJoint( _clump ), _pilotAnchor);
		_pilotchuteSimulator->pull( Jumper::getLineHandJoint( _clump ) );
	}
	// Unpacked
	if (_player && 
		!database::Canopy::getRecord(_virtues->equipment.canopy.id)->skydiving &&
		_virtues->equipment.rig.rig_aad == 2) {
	}

	_phase = jpRoaming;
}

Jumper::~Jumper()
{
	if (_player) {
		getScene()->network->stopSending();
	}

    // finalize ghost saving
    if( _saveGhost ) delete _saveGhost;
	if (_debug_window) {
		_debug_window->getPanel()->release();
		_debug_window = NULL;
	}

    // stop action to prevent inherited ambiguity
    delete _action;
    _action = NULL;

    // inform all connected cattoys
    for( CatToyI catToyI = _catToys.begin(); catToyI != _catToys.end(); catToyI++ )
    {
        (*catToyI)->disconnect();
    }
	
	// release tandem joints
	for (int i = 0; i < 4; ++i) {
		if ( _tandemJoint[i] ) {
			_scene->getPhScene()->releaseJoint( *_tandemJoint[i] );
			_tandemJoint[i] = NULL;
		}
	}

    // destroy allocated elements        
    _scene->getPhScene()->releaseActor( *_phFlight );
    _scene->getPhScene()->releaseActor( *_phFreeFall );
    _renderCallback.restore( _clump );
    delete _sensor;

    // destroy signature window
    _signature->getPanel()->release();

    // unregister in transparent triklist
    for( JumperI jumperI = _jumperL.begin(); jumperI != _jumperL.end(); jumperI++ )
    {
        if( (*jumperI) == this )
        {
            _jumperL.erase( jumperI );
            break;
        }
    }

    // disguised?
    if( _useForcedEquipment )
    {
        _virtues->equipment = _layoffEquipment;
    }
}

Jumper *Jumper::getNextJumper() {
	JumperI jumperI;
	// walk through not me
	for( jumperI = _jumperL.begin(); jumperI != _jumperL.end(); jumperI++ ) {
		if( (*jumperI) == this ) break;
	}
	jumperI++;
	// next one is next jumper
	// if next is null, return first
	if (jumperI == _jumperL.end()) {
		jumperI = _jumperL.begin();
		return (*jumperI);
	}

	// return, even if it's me
	return (*jumperI);
}

/**
 * private behaviour
 */

float Jumper::getDistanceToAbyss(void)
{
    if( _phase != jpRoaming )
    {
        // we are falling in abyss
        return 0.0f;
    }

    // in airplane we are always look in to the abyss, but doesn't have a 
    if( _enclosure == NULL ) return 0.0f;

    // cast ray from left eye position in the glance direction
    Vector3f glancePos = _clump->getFrame()->getPos() + Vector3f(0,25,0); // + Vector3f(0,100,0);
    Vector3f glanceDirection = _clump->getFrame()->getAt();
    glanceDirection.normalize();
    _sensor->sense( 
        glancePos,
        glanceDirection * glanceRayDistance,
        _enclosure->getCollisionAtomic()
    );

    // is the glance ray collides with abyss triangle?
    bool lookInAbyss = false;
    float distanceToAbyss = glanceRayDistance;
    for( unsigned int i=0; i<_sensor->getNumIntersections(); i++ )
    {
        if( strcmp( _sensor->getIntersection(i)->shader->getName(), "EnclosureAbyss" ) == 0 )
        {
            lookInAbyss = true;
            distanceToAbyss = _sensor->getIntersection(i)->distance * glanceRayDistance;
            break;
        }
    }

    // we didn't looking in abyss
    if( !lookInAbyss ) return -1.0f;
    return distanceToAbyss;
}

void Jumper::lookWhereYouFly(Vector3f direction, float dt)
{

	 
    // head bone frame
    engine::IFrame* headFrame = getHeadFrame( _clump );
    Matrix4f headLTM = headFrame->getLTM();

    // move direction vector in to the local coordinate system of head frame
    Matrix4f parentLTM = headFrame->getLTM();
    parentLTM[3][0] = parentLTM[3][1] = parentLTM[3][2] = 0.0f;
    orthoNormalize( parentLTM );
    Matrix4f iParentLTM = Gameplay::iEngine->invertMatrix( parentLTM );
    direction = Gameplay::iEngine->transformNormal( direction, iParentLTM );

    // left/right deviation
    float maxAngleLR = 70.0f;
    float angleLR = 0.0f;
    Vector3f directionLR = direction;
    directionLR[0] = 0.0f;
    directionLR[1] = fabs( directionLR[1] );
    directionLR.normalize();
    angleLR = calcAngle( Vector3f( 0,1,0 ), directionLR, Vector3f( 1,0,0 ) );    
    if( fabs( angleLR ) > maxAngleLR ) angleLR = sgn( angleLR ) * maxAngleLR;

    // up/down deviation
    float interpolatorUD = fabs( angleLR ) / maxAngleLR;
    float maxAngleUD = 45.0f * ( 1.0f - interpolatorUD ) + 15.0f * interpolatorUD;
    Vector3f directionUD = direction;
    directionUD[2] = 0.0f;
    directionUD.normalize();
    float angleUD = calcAngle( Vector3f( 0,1,0 ), directionUD, Vector3f( 0,0,1 ) );    
    if( fabs( angleUD ) > maxAngleUD ) angleUD = sgn( angleUD ) * maxAngleUD;

    // update angles
    float turnVel = 60.0f;
    float angleDifference = -angleLR - _angleLR;
    float angleToTurn = turnVel * dt * sgn( angleDifference );
    if( fabs( angleToTurn ) > fabs( angleDifference ) ) angleToTurn = angleDifference;
    _angleLR += angleToTurn;
    angleDifference = -angleUD - _angleUD;
    angleToTurn = turnVel * dt * sgn( angleDifference );
    if( fabs( angleToTurn ) > fabs( angleDifference ) ) angleToTurn = angleDifference;
    _angleUD += angleToTurn;

	if (_player && _phase == jpFreeFalling && _angleUD > -20.0f) {
		_angleUD = -20.0f;
	}

    // deviate & finalize
    Matrix4f headMatrix = headFrame->getMatrix();
    headMatrix = Gameplay::iEngine->rotateMatrix( headMatrix, Vector3f( 1,0,0 ), _angleLR );
    headMatrix = Gameplay::iEngine->rotateMatrix( headMatrix, Vector3f( 0,0,1 ), _angleUD );
    headFrame->setMatrix( headMatrix );

	if (_phase == jpFreeFalling) {
		animBodyPart("Head", Vector3f( 1,0,0 ), _angleLR);
		animBodyPart("Head", Vector3f( 0,0,1 ), _angleUD);
	}
}


bool Jumper::animBodyPart(const char *frame_title, Vector3f axis, float angle) {
	engine::IFrame* frame = Gameplay::iEngine->findFrame( _clump->getFrame(), frame_title );
	if (frame == NULL) return false;	// frame not found

    Matrix4f matrix = frame->getMatrix();
	//axis = Vector3f( matrix[2][0], matrix[2][1], matrix[2][2] );
    axis.normalize();
    matrix = Gameplay::iEngine->rotateMatrix( matrix, axis, angle );
    frame->setMatrix( matrix );

	return true;
}
void Jumper::lookAtPoint(Vector3f point, float dt)
{
    // head bone frame
    engine::IFrame* headFrame = getHeadFrame( _clump );

    // direction from bone position to point
    Vector3f direction = point - headFrame->getPos();
    direction.normalize();

    // same action
    lookWhereYouFly( direction, dt );
}

void Jumper::beginFreefall() {
	if (_phase == jpCreating) {
		_phase = jpCreating;
		//return;
	}

	_phFreeFall->wakeUp();
	_phFlight->putToSleep();

	_phase = jpFreeFalling;

	delete _action;
	_action = new Tracking( this, _phFreeFall, &_mcFreeFall);
	happen( this, EVENT_JUMPER_BEGIN_FREEFALL );

	hideEffectors( _clump );
	getRisers( _clump )->setFlags( 0 );

    _jumpPose = _clump->getFrame()->getLTM();
    _jumpPose[3][1] += ::jumperRoamingSphereSize;
}
void Jumper::InDropzone(bool dropzone) {
	_inDropzone = dropzone;
}
bool Jumper::InDropzone() {
	return _inDropzone;
}
void Jumper::setDefaultPose(int pose) {
	this->_default_pose = pose;
}
int Jumper::getDefaultPose(void) {
	return this->_default_pose;
}
void Jumper::setTandemStudent(Jumper *student) {
	this->_tandemStudent = student;
	this->_tandemStudent->setNoHelmet(this->_tandemStudent->getClump());
}
Jumper *Jumper::getTandemStudent(void) {
	return this->_tandemStudent;
}

void Jumper::onRoaming(float dt)
{
	

	if (_phase == jpCreating) {
		return;
	}

	// control tandem student
	if (_tandemStudent != NULL) {
		_tandemStudent->getSpinalCord()->phase = _spinalCord->phase;
		_tandemStudent->getSpinalCord()->trigger_phase = _spinalCord->trigger_phase;
	}

	if (_debug_window && getAirplane() && _inDropzone) {
		Matrix4f pose = getPose();
		Vector2f pos = Vector2f(pose[3][0], pose[3][2]);
		pos.normalize();
		NxVec3 wind = getScene()->getWindAtPoint(NxVec3(pose[3][0], pose[3][1], pose[3][2]));
		wind.normalize();
		Vector2f wind2(wind.x, wind.z);
		float wind_angle = 0.0;
		if (wind.magnitude() < 1.5f) {
			wind_angle = 1.0f;
		} else {
			wind2.normalize();
			wind_angle = pos.dot(wind2);
		}

		if (wind_angle >= 0.8f && pose[3][1] >= 300.0f) {
			gui::IGuiPanel* panel = _debug_window->getPanel()->find( "Message" ); assert( panel && panel->getStaticText() );
			panel->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(899) );
		} else {
			gui::IGuiPanel* panel = _debug_window->getPanel()->find( "Message" ); assert( panel && panel->getStaticText() );
			panel->getStaticText()->setText( L"" );		
		}
	}




	//if (_spinalCord->leftWarp ) {
	//	idleSequence.startTime -= 0.03f;
	//	idleSequence.loopStartTime = idleSequence.startTime;
	//	getCore()->logMessage("Animation: %2.1f %2.1f", idleSequence.startTime * 100 / 3, idleSequence.endTime * 100 / 3);
	//} else if (_spinalCord->rightWarp ) {
	//	idleSequence.startTime += 0.03f;
	//	idleSequence.loopStartTime = idleSequence.startTime;
	//	getCore()->logMessage("Animation: %2.1f %2.1f", idleSequence.startTime * 100 / 3, idleSequence.endTime * 100 / 3);
	//}

	//if (_spinalCord->wlo ) {
		//idleSequence.endTime -= 0.03f;
		//getCore()->logMessage("Animation: %2.1f %2.1f", idleSequence.startTime * 100 / 3, idleSequence.endTime * 100 / 3);
	//} else if (_spinalCord->hook ) {
		//idleSequence.endTime += 0.03f;
		//getCore()->logMessage("Animation: %2.1f %2.1f", idleSequence.startTime * 100 / 3, idleSequence.endTime * 100 / 3);
	//}

	// update gadgets
	if (_variometer) {
		_variometer->updateActivity(dt);
	}
	if (_altimeter) {
		_altimeter->updateActivity(dt);
	}

    // airplane roaming is separate case
    if( _airplane )
    {
		//if (_spinalCord->modifier) {
		//	_airplane->getClump()->getFrame()->translate( Vector3f( 0.0f, 1000.0f * dt, 0.0f ) );
		//}
		/*
		NxActor *plane = _airplane->getPhActor();
		if (plane && plane->isSleeping() == false) {
			// pitch
			if (_spinalCord->up) {
				plane->addLocalTorque(NxVec3(_spinalCord->up*480000.0f, 0, 0));
			}
			if (_spinalCord->down) {
				plane->addLocalTorque(NxVec3(_spinalCord->down*-480000.0f, 0, 0));
			}
			if (_spinalCord->leftRearRiser) {
				plane->addLocalTorque(NxVec3(0, 0, _spinalCord->leftRearRiser*-480000.0f));
			}
			if (_spinalCord->rightRearRiser) {
				plane->addLocalTorque(NxVec3(0, 0, _spinalCord->rightRearRiser*480000.0f));
			}
			if (_spinalCord->left) {
				plane->addLocalTorque(NxVec3(0, _spinalCord->left*440000.0f, 0));
			}
			if (_spinalCord->right) {
				plane->addLocalTorque(NxVec3(0, _spinalCord->right*-440000.0f, 0));
			}
			if (_spinalCord->modifier) {
				plane->addLocalForce(NxVec3(0,0,4300.0f*_spinalCord->modifier));
			}
		}
		*/
		//	// output
		//	// indicated airspeed
		//	NxVec3 velocity = plane->getLinearVelocity();
		//	const float ias = velocity.magnitude();
		//	// altitude
		//	NxVec3 pos = plane->getGlobalPosition();
		//	const float alt = pos.y;
		//	// vertical speed
		//	const float Vs = velocity.y;
		//	// AOA
		//	NxMat34 globalPose = plane->getGlobalPose();
		//	NxVec3 x = globalPose.M.getColumn(0);
		//	NxVec3 y = globalPose.M.getColumn(1);
		//	NxVec3 z = globalPose.M.getColumn(2);

		//	// test
		//	NxVec3 normal;
		//	engine::IFrame *prop = _airplane->getPropFrame();
		//	normal = wrap(prop->getAt());
		//	normal.normalize();

		//	velocity.normalize();
		//	//const float AOA = -::calcAngle( z, velocity, x );
		//	const float AOA = ::calcAngle( normal, velocity );


		//	if (this->_debug_window) {
		//		gui::IGuiPanel* panel = this->_debug_window->getPanel()->find( "Message" );
		//		assert( panel && panel->getStaticText() );
		//		panel->getStaticText()->setText( wstrformat(L"normal: %2.2f %2.2f %2.2f\n\nAlt: %2.2f\nIAS: %2.2f\nVs: %2.2f\nAOA: %2.2f", normal.x, normal.y, normal.z, alt, ias, Vs, AOA).c_str() );
		//	}
		//}

		bool helicopter = strcmp(_airplane->getDesc()->templateClump->getName(), "Helicopter01") == 0;

		////if (helicopter) {
		//	//Matrix4f pose = _scene->getCamera()->getPose();
		//	//_airplane->setPose(pose);
		//	//getCore()->logMessage("SETPOSE");

		//	/// heli controls
		//	const float heliP = 20000.0f;
		//	const float heliT = 100.0f;

		//	const float power = (_spinalCord->hook - _spinalCord->wlo) * heliP * dt;
		//	const float fwd = (_spinalCord->up - _spinalCord->down) * heliP * dt;
		//	const float pitch = (_spinalCord->up - _spinalCord->down) * 30.0f * dt;
		//	const float turn = (_spinalCord->left - _spinalCord->right) * heliT * dt;

		//	_airplane->getClump()->getFrame()->translate( Vector3f( 0.0f, power, fwd ) );
		//	_airplane->getClump()->getFrame()->rotateRelative(Vector3f(0.0f, 1.0f, 0.0f), turn);
		//	//_airplane->getClump()->getFrame()->rotateRelative(Vector3f(1.0f, 0.0f, 0.0f), pitch);
		////}
        // detect phase switch
        if( _spinalCord->phase )
        {                
            _phase = jpFreeFalling;
			if (_debug_window) {
				gui::IGuiPanel* panel = _debug_window->getPanel()->find( "Message" ); assert( panel && panel->getStaticText() );
				panel->getStaticText()->setText( L"" );	
			}
            delete _action;
           
			// flip exits
			// front
			if (_spinalCord->up && helicopter && getScene()->getCareer()->getAcrobaticsSkill( ::acroFrontFlip )) {
				_action = new FlipJump( this, _phFreeFall, &_mcFreeFall, &frontFlipSequence, frontFlipAnimSpeed, frontFlipCriticalPeriod );
			// front back
			} else if (_spinalCord->down && helicopter && getScene()->getCareer()->getAcrobaticsSkill( ::acroFrontBackFlip )) {
				_action = new FlipJump( this, _phFreeFall, &_mcFreeFall, &frontBackFlipSequence, frontBackFlipAnimSpeed, frontBackFlipCriticalPeriod );
			// normal exit
			} else {
				_action = new AirplaneJump( this, _phFreeFall, &_mcFreeFall );
			}

            _jumpPose = _clump->getFrame()->getLTM();
            _jumpPose[3][1] += ::jumperRoamingSphereSize;
            // event
            happen( this, EVENT_JUMPER_BEGIN_FREEFALL );
			++_virtues->statistics.numSkydives;
        }

        // no further activity
        return;
    }

    // dispatch move action
    if( _spinalCord->up == 0 && _spinalCord->down == 0 )
    {
        bool actionIsWalkForward = actionIs(WalkForward);
        bool actionIsWalkBackward = actionIs(WalkBackward);
        // current action is move?
        if( actionIsWalkForward || actionIsWalkBackward )
        {
            // event
            if( actionIsWalkForward )
            {
                happen( this, EVENT_JUMPER_END_WALKFWD );
            }
            else
            {
                happen( this, EVENT_JUMPER_END_WALKBCK );
            }
            // prevent blending of bone procedural animation
            _clump->getAnimationController()->advance( 0.0f );
            // stop turn
            delete _action;            
            _action = new Character::Idle( _clump, &idleSequence, 0.2f, 0.5f );
        }
    }
    else
    {
        // current action is not move?
        if( _spinalCord->up != 0 && !actionIs(WalkForward) )
        {
            // check turn action, if present, event is happen
            if( actionIs(Turn) ) happen( this, EVENT_JUMPER_END_TURN );
            // prevent blending of bone procedural animation
            _clump->getAnimationController()->advance( 0.0f );
            // begin new action
            delete _action;
            _action = new WalkForward( this );
            // event
            happen( this, EVENT_JUMPER_BEGIN_WALKFWD );
        }
        else if( _spinalCord->down != 0 && !actionIs(WalkBackward) )
        {
            // check turn action, if present, event is happen
            if( actionIs(Turn) ) happen( this, EVENT_JUMPER_END_TURN );
            // prevent blending of bone procedural animation
            _clump->getAnimationController()->advance( 0.0f );
            // begin new action
            delete _action;
            _action = new WalkBackward( this );
            // event
            happen( this, EVENT_JUMPER_BEGIN_WALKBCK );
        }
    }

    // dispatch turn action
    if( _spinalCord->left == 0 && _spinalCord->right == 0 )
    {
        // current action is turn?
        if( actionIs(Turn) )
        {
            // event
            happen( this, EVENT_JUMPER_END_TURN );
            // prevent blending of bone procedural animation
            _clump->getAnimationController()->advance( 0.0f );
            // stop turn
            delete _action;
            _action = new Character::Idle( _clump, &idleSequence, 0.2f, 0.5f );
        }
    }
    else
    {
        // current action is not turn and not move?
        if( !actionIs(Turn) && !actionIs(WalkForward) && !actionIs(WalkBackward) )
        {
            // prevent blending of bone procedural animation
            _clump->getAnimationController()->advance( 0.0f );
            // begin turn action
            delete _action;
            _action = new Turn( this, _spinalCord->right - _spinalCord->left );
            // event
            happen( this, EVENT_JUMPER_BEGIN_TURN );
        }
    }

    // detect phase switch
    if( _spinalCord->phase )
    {        
        float maxDistance = 50.0f;
        
        ActionChannel* modifier = Gameplay::iGameplay->getActionChannel( ::iaModifier );
        bool isWearWingsuit = database::Suit::getRecord( getVirtues()->equipment.suit.id )->wingsuit;
        bool isRunning = modifier->getTrigger();// && !isWearWingsuit;

        WalkForward* walkForward = dynamic_cast<WalkForward*>( _action );
        if( walkForward && isRunning )
        {
            maxDistance = ( 0.25f * walkForward->getVelocity() );
        }

        // sense enclosure bounds before the character
        _sensor->sense(
            _clump->getFrame()->getPos() + Vector3f(0,25,0), // Vector3f(0,180,0),
            _clump->getFrame()->getAt() * maxDistance,
            _enclosure->getCollisionAtomic()
        );
        bool isAbyss = false;
        float distanceToAbyss = maxDistance;
		unsigned int i;
        for( i=0; i<_sensor->getNumIntersections(); i++ )
        {
            if( strcmp( _sensor->getIntersection(i)->shader->getName(), "EnclosureAbyss" ) == 0 )
            {
                distanceToAbyss = _sensor->getIntersection(i)->distance;
                isAbyss = true;
                break;
            }
        }

        // sense enclosure bounds behind the character
        _sensor->sense(
            _clump->getFrame()->getPos() + Vector3f(0,25,0), //Vector3f(0,180,0),
            _clump->getFrame()->getAt() * -maxDistance,
            _enclosure->getCollisionAtomic()
        );
        bool isAbyssBehind = false;
        distanceToAbyss = maxDistance;
        for( i=0; i<_sensor->getNumIntersections(); i++ )
        {
            if( strcmp( _sensor->getIntersection(i)->shader->getName(), "EnclosureAbyss" ) == 0 )
            {
                distanceToAbyss = _sensor->getIntersection(i)->distance;
                isAbyssBehind = true;
                break;
            }
        }
        
        // jump!
        if( isAbyss )
        {
            // modify freefall adrenaline limit with accumulated value
            _fallLimit += _adrenaline;

            // if jumper is running            
            if( walkForward && isRunning )
            {
                // check player able to do this jump
                bool isAble = true;
                if( isPlayer() && !getScene()->getCareer()->getAcrobaticsSkill( ::acroJumpFromRun ) ) isAble = false;
                // start action
                if( isAble && walkForward->getVelocity() > 200.0f )
                {
                    _phase = jpFreeFalling;
                    float vel = walkForward->getVelocity();
                    delete _action;

                    _action = new RunningJump( this, _phFreeFall, &_mcFreeFall, vel );
					//_action = new FlipJump( this, _phFreeFall, &_mcFreeFall, &runningBackFlipSequence, runningBackFlipAnimSpeed, runningBackFlipCriticalPeriod );
                    _jumpPose = _clump->getFrame()->getLTM();
                    _jumpPose[3][1] += ::jumperRoamingSphereSize;
                    // events
					++_virtues->statistics.numBaseJumps;
                    happen( this, EVENT_JUMPER_BEGIN_FREEFALL );
                    happen( this, EVENT_JUMPER_END_WALKFWD );
                    happen( this, EVENT_JUMPER_END_WALKBCK );
                    happen( this, EVENT_JUMPER_END_TURN );
                }
            }
            else
            {                
                _phase = jpFreeFalling;
                delete _action;

				// side step left jump
				if ( isPlayer() && _spinalCord->left && _spinalCord->modifier && getScene()->getCareer()->getAcrobaticsSkill( ::acroFrontFlip )) {
					_action = new SideStepJump( this, _phFreeFall, &_mcFreeFall, &sideStepLeftJumpSequence, sideStepLeftJumpAnimSpeed, sideStepLeftJumpCriticalPeriod );
				// hop hop front flip
				} else if ( isPlayer() && _spinalCord->up && _spinalCord->down && getScene()->getCareer()->getAcrobaticsSkill( ::acroFrontFlip )) {
					_action = new FlipJump( this, _phFreeFall, &_mcFreeFall, &frontHopHopFlipSequence, frontHopHopFlipAnimSpeed, frontHopHopFlipCriticalPeriod );
                // choose jump action : front flip
				} else if ( isPlayer() &&
                    /* !isWearWingsuit && */
                    Gameplay::iGameplay->getActionChannel( ::iaForward )->getTrigger() && 
                    getScene()->getCareer()->getAcrobaticsSkill( ::acroFrontFlip ) )
                {
                    _action = new FlipJump( this, _phFreeFall, &_mcFreeFall, &frontFlipSequence, frontFlipAnimSpeed, frontFlipCriticalPeriod );
                }
                // choose jump action : front-back flip
                else if( isPlayer() &&
                    /* !isWearWingsuit && */
                    Gameplay::iGameplay->getActionChannel( ::iaBackward )->getTrigger() &&
                    getScene()->getCareer()->getAcrobaticsSkill( ::acroFrontBackFlip ) )
                {
                    _action = new FlipJump( this, _phFreeFall, &_mcFreeFall, &frontBackFlipSequence, frontBackFlipAnimSpeed, frontBackFlipCriticalPeriod );
                }
                // choose jump action : StandingJump
                else
                {
                    _action = new StandingJump( this, _phFreeFall, &_mcFreeFall );
                }                
                _jumpPose = _clump->getFrame()->getLTM();
                _jumpPose[3][1] += ::jumperRoamingSphereSize;
                // event
				++_virtues->statistics.numBaseJumps;
                happen( this, EVENT_JUMPER_BEGIN_FREEFALL );
                happen( this, EVENT_JUMPER_END_WALKFWD );
                happen( this, EVENT_JUMPER_END_WALKBCK );
                happen( this, EVENT_JUMPER_END_TURN );
            }
        }
        else if( isAbyssBehind )
        {
            if( isPlayer() &&
                /* !isWearWingsuit && */
                Gameplay::iGameplay->getActionChannel( ::iaBackward )->getTrigger() &&
                getScene()->getCareer()->getAcrobaticsSkill( ::acroBackFlip ) )
            {
                // modify freefall adrenaline limit with accumulated value
                _fallLimit += _adrenaline;
                // start action
                _phase = jpFreeFalling;
                delete _action;
                _action = new FlipJump( this, _phFreeFall, &_mcFreeFall, &backFlipSequence, backFlipAnimSpeed, backFlipCriticalPeriod );
                // save jump pose
                _jumpPose = _clump->getFrame()->getLTM();
                _jumpPose[3][1] += ::jumperRoamingSphereSize;
                // event
				++_virtues->statistics.numBaseJumps;
                happen( this, EVENT_JUMPER_BEGIN_FREEFALL );
                happen( this, EVENT_JUMPER_END_WALKFWD );
                happen( this, EVENT_JUMPER_END_WALKBCK );
                happen( this, EVENT_JUMPER_END_TURN );
            }
        }
    }

    // dispatch end of action
    if( _action->isEndOfAction() )
    {
        // prevent blending of bone procedural animation
        _clump->getAnimationController()->advance( 0.0f );
        // idling
        delete _action;
        _action = new Character::Idle( _clump, &idleSequence, 0.2f, 0.5f );
    }
}

void Jumper::onFreeFalling(float dt)
{
	// connect tandem student
	if (_player && !_phFreeFall->isSleeping() && _tandemJoint[0] == NULL && _tandemStudent && !_tandemStudent->getFreefallActor()->isSleeping()) {
		//// connect joint
		const float maxDistance = 0.006f;
		const float y = 0.3f;
		NxDistanceJointDesc jointDesc;
		if (!_phFreeFall->isSleeping()) {
			jointDesc.actor[0] = _phFreeFall;
		} else {
			jointDesc.actor[0] = _phFlight;
		}
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3(-0.2f,0.0f,0.2f);
		jointDesc.localAnchor[1] = NxVec3(-0.2f,y,0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[0] = _scene->getPhScene()->createJoint( jointDesc ); 
		//NxDistanceJointDesc jointDesc;
		if (!_phFreeFall->isSleeping()) {
			jointDesc.actor[0] = _phFreeFall;
		} else {
			jointDesc.actor[0] = _phFlight;
		}
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3( 0.2f,0.0f,0.2f);
		jointDesc.localAnchor[1] = NxVec3( 0.2f,y,0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[1] = _scene->getPhScene()->createJoint( jointDesc ); 
		//NxDistanceJointDesc jointDesc;
		if (!_phFreeFall->isSleeping()) {
			jointDesc.actor[0] = _phFreeFall;
		} else {
			jointDesc.actor[0] = _phFlight;
		}
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3(-0.2f,0.0f,-0.2f);
		jointDesc.localAnchor[1] = NxVec3(-0.2f,y-0.05f,-0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[2] = _scene->getPhScene()->createJoint( jointDesc ); 
		//NxDistanceJointDesc jointDesc;
		if (!_phFreeFall->isSleeping()) {
			jointDesc.actor[0] = _phFreeFall;
		} else {
			jointDesc.actor[0] = _phFlight;
		}
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3( 0.2f,0.0f,-0.2f);
		jointDesc.localAnchor[1] = NxVec3( 0.2f,y-0.05f,-0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[3] = _scene->getPhScene()->createJoint( jointDesc ); 
	}


	//if (false && _player) {
	//	_scene->setTimeSpeed( 1.0f );
	//	float vel = _phFreeFall->getLinearVelocity().magnitude();
	//	if (vel > 50.0f) {
	//		float adrenaline = vel / 100.0f;
	//		if (adrenaline > 1.0f) adrenaline = 1.0f;
	//		_scene->setTimeSpeed( 1.0f + adrenaline*0.5f );
	//	}
	//}



	// freeze time
	//if (_player) {
	//	if (_spinalCord->trigger_modifier) {
	//		//_scene->setTimeSpeed( 0.01f );
	//		_scene->setTimeSpeedMultiplier(0.001f);
	//	} else {
	//		//_scene->setTimeSpeed( 1.0f );
	//		_scene->setTimeSpeedMultiplier(1.0f);
	//	}
	//}

	 // jumper looks at another jumper nearby
	//NxVec3 closest_jumper_pos = NxVec3(0,0,0);
	//float min_distance = -1.0f;
	//NxVec3 mypos = this->_phFreeFall->getGlobalPosition();
	//for( JumperI jumperI = _jumperL.begin(); jumperI != _jumperL.end(); jumperI++ ) {
	//	if( (*jumperI) != this ) {
	//		if ((*jumperI)->getFreefallActor()->isSleeping()) {
	//			continue;
	//		}

	//		// distance to jumper
	//		NxVec3 vec = mypos - (*jumperI)->getFreefallActor()->getGlobalPosition();
	//		float dst = vec.magnitude();
	//		if (min_distance >= dst || min_distance == -1.0f) {
	//			closest_jumper_pos = (*jumperI)->getFreefallActor()->getGlobalPosition();
	//			min_distance = dst;
	//		}
	//	}
	//}

	//// look at him
	//if (min_distance <= 30.0f && min_distance > 0.0f) {
	//	lookAtPoint(wrap(closest_jumper_pos), dt);
	//}

	// AAD
	if (_virtues->equipment.reserve.type != gtUnequipped && _virtues->equipment.rig.rig_aad > 0) {
		database::AAD *device = database::AAD::getRecord(_virtues->equipment.rig.rig_aad);
		if (!_phFreeFall->isSleeping() && fabs(_phFreeFall->getLinearVelocity().y) >= device->speed && _phFreeFall->getGlobalPosition().y <= device->altitude) {
			_spinalCord->pullReserve = true;
		}
	}

	// logbook time
	if (_jumpTime > 3.0f) {
		_virtues->statistics.freeFallTime += dt;
		// todo: no calculate this over and over again
		if (database::Suit::getRecord(_virtues->equipment.suit.id)->wingsuit) {
			_virtues->statistics.wingsuitTime += dt;
		}
	}

    // dispatch death
    // (in some cases jumper remains under control after fatal hit)
    if( ( _virtues->evolution.health == 0 ) && !actionIs(OutOfControl) )
    {
        delete _action;
        _action = new OutOfControl( this, _phFreeFall, &_mcFreeFall, 10.0f );
    }

	// cut main before deployment
	if (_virtues->equipment.reserve.type != gtUnequipped && _spinalCord->cutAway && _canopySimulator) {
		_canopySimulator->cutOnOpening = true;
	}
    // main pull
	if( _spinalCord->phase && !_phFreeFall->isSleeping() &&
        !actionIs(RunningJump) && !actionIs(StandingJump) && 
        !actionIs(OutOfControl) && !actionIs(Pull) &&
		(_pilotchuteSimulator && !_pilotchuteSimulator->isConnected()) &&
		 (_canopySimulator && !_canopySimulator->isOpened() ) )
    {
		if (_phJoint ) {
			_scene->getPhScene()->releaseJoint( *_phJoint );
			_phJoint = NULL;
		}

        delete _action;
        _action = new Pull( this, _phFreeFall, &_mcFreeFall, _pilotchuteSimulator, _pilotAnchor );
        _phase = jpFlight;
        // event
        happen( this, EVENT_DELAY_STOP );
    }

    // dispatch reserve pull
	if( _pilotchuteReserveSimulator &&
		!_pilotchuteReserveSimulator->isConnected() &&
		_spinalCord->pullReserve && 
        !actionIs(RunningJump) && !actionIs(StandingJump) && 
         !actionIs(Pull)
         )
    {
		//this->fireReserveCanopy();
			if (_phJoint ) {
				_scene->getPhScene()->releaseJoint( *_phJoint );
				_phJoint = NULL;
			}

            delete _action;
			_pilotchuteReserveSimulator->setFreebag(true);
            _action = new PullReserve( this, _phFreeFall, &_mcFreeFall, _pilotchuteReserveSimulator, _pilotAnchor );
            _phase = jpFlight;
            // event
            happen( this, EVENT_DELAY_STOP );
			return;
    }

    // dispatch end of action (jumps are only terminal actions)
    if( _action->isEndOfAction() )
    {
		// distatch
		if( _pilotchuteSimulator && _pilotchuteSimulator->isPulled() && !actionIs(Pull) )
        {
			if (_phJoint ) {
				_scene->getPhScene()->releaseJoint( *_phJoint );
				_phJoint = NULL;
			}

            delete _action;
            _action = new Pull( this, _phFreeFall, &_mcFreeFall, _pilotchuteSimulator, _pilotAnchor );
            _phase = jpFlight;
            // event
            happen( this, EVENT_DELAY_STOP );
        }
        else
        {

            if( actionIs( FlipJump ) )
            {
                delete _action;
                _action = new Jumper::Flip( this, _phFreeFall, &_mcFreeFall, 0.0f );

			} else if ( actionIs ( SideStepJump ) ) {
				delete _action;
				_action = new Jumper::Tracking( this, _phFreeFall, &_mcFreeFall );
			}
            else
            {
                delete _action;
                _action = new Jumper::Tracking( this, _phFreeFall, &_mcFreeFall );
            }
            // event
            happen( this, EVENT_DELAY_START );
        }        
    }
    
    if( ( _phase == jpFreeFalling ) && isPlayer() )
    {
        float ttfTreshold = 2.5f;
        float fttTreshold = 1.8f;

        // wingsuit condition
        bool isWearWingsuit = database::Suit::getRecord( getVirtues()->equipment.suit.id )->wingsuit;

        // dispatch tracking-to-flip and flip-to-tracking
        if( /*!isWearWingsuit &&*/ actionIs( Tracking ) && !_spinalCord->modifier )
        {
    //        // base jumper basis
    //        NxMat34 freeFallPose = _phFreeFall->getGlobalPose();
    //        NxVec3 x = freeFallPose.M.getColumn(0);

    //        // switch to flip by x-component of angular velocity        
    //        if( fabs( x.dot( _phFreeFall->getAngularVelocity() ) ) > ttfTreshold &&
				//!_spinalCord->leftWarp && !_spinalCord->rightWarp && ( _spinalCord->up || _spinalCord->down)
				//)
    //        {
    //            delete _action;
    //            _action = new Jumper::Flip( this, _phFreeFall, &_mcFreeFall, 0.5f );
    //        }
        }
        else if( actionIs( Flip ) && ( _action->getActionTime() > 0 ) )
        {
            // base jumper basis
            NxMat34 freeFallPose = _phFreeFall->getGlobalPose();
            NxVec3 x = freeFallPose.M.getColumn(0);

            // switch to tracking by x-component of angular velocity        
            if( fabs( x.dot( _phFreeFall->getAngularVelocity() ) ) < fttTreshold ||
                _spinalCord->modifier )
            {
                delete _action;
                _action = new Jumper::Tracking( this, _phFreeFall, &_mcFreeFall );
            }
        }
    }
}

void Jumper::cutAwayMainCanopy() {
	if (_virtues->equipment.reserve.type == gtUnequipped || _canopySimulator == NULL) return;

	// cut away in flight, let's start falling again (or just stay with reserve)
	if (_phFreeFall->isSleeping()) {
		if (!_canopyReserveSimulator->isConnected()) {		// if reserve is deployed, do not start freefalling
			_phFreeFall->wakeUp();
			_phFlight->putToSleep();

			_phase = jpFreeFalling;

			NxMat34 pose = _phFlight->getGlobalPose();
			NxVec3 linvel = _phFlight->getLinearVelocity(), angvel = _phFlight->getAngularVelocity();

			_phFreeFall->setGlobalPose(pose);
			_phFreeFall->setLinearVelocity(linvel);
			_phFreeFall->setAngularVelocity(angvel);

			delete _action;
			_action = new Tracking( this, _phFreeFall, &_mcFreeFall);
			happen( this, EVENT_JUMPER_BEGIN_FREEFALL );

			hideEffectors( _clump );
			getRisers( _clump )->setFlags( 0 );
		}
	} else {
		_phFreeFall->wakeUp();
		//_phFlight->putToSleep();

		_phase = jpFreeFalling;

		delete _action;
		_action = new Tracking( this, _phFreeFall, &_mcFreeFall);
		happen( this, EVENT_JUMPER_BEGIN_FREEFALL );
	}

	// destroy canopy simulator
	_canopySimulator->setLinetwists(0.0f);

	_canopySimulator->disconnect();

	//delete _canopySimulator;
	_canopySimulator = NULL;
	//delete _pilotchuteSimulator;
	_pilotchuteSimulator = NULL;
}

void Jumper::fireReserveCanopy() {
	if( !_pilotchuteReserveSimulator->isPulled() &&
		!_phFreeFall->isSleeping() )
	{
		// pilotchute pull frame
		Vector3f pp = Jumper::getLineRigJoint( this->getClump() )->getPos();
		Vector3f py = this->getVel(); py.normalize();
		Vector3f px; px.cross( py, Vector3f(0,1,0) ); px.normalize();
		Vector3f pz; py.cross( px, py ); pz.normalize();
		pp += py * 100.0f;

		// connect pilot chute
		_pilotchuteReserveSimulator->connect( 
			_phFreeFall, 
			Jumper::getBackBone( this->getClump() ), 
			_pilotAnchor 
		);

		// pull pilotchute
		_pilotchuteReserveSimulator->pull( Matrix4f(
			px[0], px[1], px[2], 0.0f,
			py[0], py[1], py[2], 0.0f,
			pz[0], pz[1], pz[2], 0.0f,
			pp[0], pp[1], pp[2], 1.0f
		) );
		_pilotchuteReserveSimulator->updateActivity( 0.0f );

		// and drop
		_pilotchuteReserveSimulator->drop( NxVec3( 0,0,0 ) );
		_pilotchuteReserveSimulator->setInflation( 0.3f );
		return;
	}


	// stop falling
	if (!_phFreeFall->isSleeping() || _phFlight->isSleeping()) {
		_phFlight->wakeUp();
		_phFreeFall->putToSleep();

		_phase = jpFlight;

		NxMat34 pose = _phFreeFall->getGlobalPose();
		NxVec3 linvel = _phFreeFall->getLinearVelocity(), angvel = _phFreeFall->getAngularVelocity();

		_phFlight->setGlobalPose(pose);
		_phFlight->setLinearVelocity(linvel);
		_phFlight->setAngularVelocity(angvel);

		delete _action;
		//_action = new Jumper::CanopyOpening( this, _phFreeFall, _phFlight, &_mcFlight, _pilotchuteReserveSimulator, _canopyReserveSimulator, _frontLeftAnchor, _frontRightAnchor, _rearLeftAnchor, _rearRightAnchor );
		_action = new Jumper::Flight(this, _phFlight, &_mcFlight);
		happen( this, EVENT_JUMPER_END_FREEFALL );
		//
	}


	Matrix4f sampleLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM();
	Vector3f sampleX( sampleLTM[0][0], sampleLTM[0][1], sampleLTM[0][2] );
	Vector3f sampleY( sampleLTM[1][0], sampleLTM[1][1], sampleLTM[1][2] );
	Vector3f sampleZ( sampleLTM[2][0], sampleLTM[2][1], sampleLTM[2][2] );
	Vector3f sampleP( sampleLTM[3][0], sampleLTM[3][1], sampleLTM[3][2] );
	// orient canopy towards jumper velocity
	sampleZ = wrap( _phFlight->getLinearVelocity() );
	sampleZ *= -1;
	sampleZ.normalize();
	sampleY = this->getClump()->getFrame()->getAt() * -1;
	sampleX.cross( sampleY, sampleZ );
	sampleX.normalize();
	sampleY.cross( sampleZ, sampleX );
	sampleY.normalize();
	sampleLTM.set( 
		sampleX[0], sampleX[1], sampleX[2], 0.0f,
		sampleY[0], sampleY[1], sampleY[2], 0.0f,
		sampleZ[0], sampleZ[1], sampleZ[2], 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	// move clump behind jumper
	sampleP += sampleZ * 100;
	sampleLTM[3][0] = sampleP[0];
	sampleLTM[3][1] = sampleP[1];
	sampleLTM[3][2] = sampleP[2];

	// connect & open canopy
	_canopyReserveSimulator->connect(
		_phFlight, 
		_frontLeftAnchor,
		_frontRightAnchor,
		_rearLeftAnchor,
		_rearRightAnchor,
		Jumper::getFrontLeftRiser( _clump ),
		Jumper::getFrontRightRiser( _clump ),
		Jumper::getRearLeftRiser( _clump ),
		Jumper::getRearRightRiser( _clump )
	);

	_canopyReserveSimulator->open( wrap( sampleLTM ), wrap(Vector3f(0, 10.0f, 0) )/*_phFlight->getLinearVelocity()*-2*/, 0, 0, 0 );

	_pilotchuteReserveSimulator->connect( 
		_canopyReserveSimulator->getNxActor(), 
		CanopySimulator::getPilotCordJoint( _canopyReserveSimulator->getClump() ),
		_canopyReserveSimulator->getPilotAnchor()
	);
	_phFreeFall->putToSleep();
}

CanopySimulator *Jumper::getDominantCanopy() {
	if ( !_canopyReserveSimulator ) return _canopySimulator;

	if ( _canopySimulator && !_canopySimulator->isCutAway && _canopySimulator->isOpened()) {// main open
		assert(_canopySimulator);
		return _canopySimulator;

	} else if ( _canopyReserveSimulator->isOpened()) {  // reserve open
		assert( _canopyReserveSimulator );
		return _canopyReserveSimulator;

	} else if ( _pilotchuteSimulator && _canopySimulator && !_canopySimulator->isCutAway && _pilotchuteSimulator->isConnected() ) {	//opening
		assert(_canopySimulator);
		return _canopySimulator;

	} else {		// make sure not to delete _canopyReserveSimulator before this method may be called
		assert( _canopyReserveSimulator );
		return _canopyReserveSimulator;
	}
}

PilotchuteSimulator *Jumper::getDominantPilotChute() {
	if (getDominantCanopy() == _canopySimulator) {
		return _pilotchuteSimulator;
	} else {
		return _pilotchuteReserveSimulator;
	}
}

void Jumper::onFlight(float dt)
{

	// disconnect tandem
	if (_player && !_phFlight->isSleeping() && _tandemStudent != NULL) {
		for (int i = 0; i < 4; ++i) {
			if ( _tandemJoint[i] ) {
				_scene->getPhScene()->releaseJoint( *_tandemJoint[i] );
			}
		}
		// reconnect tandem
		//// connect joint
		const float maxDistance = 0.006f;
		const float y = 0.3f;
		NxDistanceJointDesc jointDesc;
		jointDesc.actor[0] = _phFlight;
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3(-0.2f,0.0f,0.2f);
		jointDesc.localAnchor[1] = NxVec3(-0.2f,y,0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[0] = _scene->getPhScene()->createJoint( jointDesc ); 
		//NxDistanceJointDesc jointDesc;
		jointDesc.actor[0] = _phFlight;
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3( 0.2f,0.0f,0.2f);
		jointDesc.localAnchor[1] = NxVec3( 0.2f,y,0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[1] = _scene->getPhScene()->createJoint( jointDesc ); 
		//NxDistanceJointDesc jointDesc;
		jointDesc.actor[0] = _phFlight;
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3(-0.2f,0.0f,-0.2f);
		jointDesc.localAnchor[1] = NxVec3(-0.2f,y-0.05f,-0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[2] = _scene->getPhScene()->createJoint( jointDesc ); 
		//NxDistanceJointDesc jointDesc;
		jointDesc.actor[0] = _phFlight;
		jointDesc.actor[1] = _tandemStudent->getFreefallActor();
		jointDesc.maxDistance = maxDistance;
		jointDesc.minDistance = 0.0f;
		jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
		jointDesc.localAnchor[0] = NxVec3( 0.2f,0.0f,-0.2f);
		jointDesc.localAnchor[1] = NxVec3( 0.2f,y-0.05f,-0.2f);
		jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
		_tandemJoint[3] = _scene->getPhScene()->createJoint( jointDesc ); 
		_tandemStudent->getFreefallActor()->putToSleep();
		_tandemStudent = NULL;


	}

	// AAD
	if (_virtues->equipment.reserve.type != gtUnequipped && _virtues->equipment.rig.rig_aad > 0) {
		database::AAD *device = database::AAD::getRecord(_virtues->equipment.rig.rig_aad);
		if (!_phFlight->isSleeping() && fabs(_phFlight->getLinearVelocity().y) >= device->speed && _phFlight->getGlobalPosition().y <= device->altitude) {
			//_spinalCord->pullReserve = true;
		}
	}

	// calculate canopy time
	if (actionIs(Jumper::Flight)) {
		_virtues->statistics.canopyTime += dt;
	}

	// cut away main action
	if (_virtues->equipment.reserve.type != gtUnequipped &&			// reserve equipped
		_canopySimulator &&											// main still with the jumper (in container or deployed)
		(_spinalCord->cutAway || _canopySimulator->cutOnOpening)	// cutaway pressed
		//_phase != jpFreeFalling									// not have a canopy to cutaway (ie not freefalling)
		) {

		_spinalCord->cutAway = false;
		// if canopy still in container, cut it away, but it will get destroyed only when deployed
		if (!_canopySimulator->isOpened()) {
			_canopySimulator->cutOnOpening = true;
		// if canopy is out of the container, destroy it right now
		} else {
			this->cutAwayMainCanopy();
			return;
		}
	}


    // dispatch reserve pull (WHILE UNDER MAIN) (because, look at method name)
	if( _spinalCord->pullReserve && 
		_pilotchuteReserveSimulator &&
		!_pilotchuteReserveSimulator->isConnected() &&
		this->getDominantCanopy() == _canopySimulator &&	// flying under main?
        !actionIs(RunningJump) && !actionIs(StandingJump) && 
        !actionIs(OutOfControl) && !actionIs(Pull) &&
        !actionIs(Flip) )
    {
		this->fireReserveCanopy();

		return;
    }

	// choose canopy
	CanopySimulator *canopy = this->getDominantCanopy();
	PilotchuteSimulator *pc = this->getDominantPilotChute();

    // dispatch linetwists
    if( ( canopy->getLinetwists() != 0 ) && !actionIs(Linetwists) )
    {
        CanopyOpening* canopyOpening = dynamic_cast<CanopyOpening*>( _action );
        if( canopyOpening && !canopyOpening->isCriticalAnimationRange() )
        {
            // start linetwists
            delete _action;
            _action = new Linetwists( this, _phFlight, &_mcFlight );
            // event
            happen( this, EVENT_JUMPER_END_FREEFALL );
        }
    }
    if( ( canopy->getLinetwists() == 0 ) && actionIs(Linetwists) )
    {
        // start flight action
        delete _action;
        _action = new Jumper::Flight( this, _phFlight, &_mcFlight );
    }

    // dispatch end of action
    if( _action->isEndOfAction() )
    {
		if( actionIs(Pull) || actionIs(PullReserve) )
        {
            // start opening action
			canopy = this->getDominantCanopy();
			pc = this->getDominantPilotChute();

			if (pc->isConnected()) {	// are we really trying to open this chute? (could be cut, main sequence)
				delete _action;
				_action = new Jumper::CanopyOpening( this, _phFreeFall, _phFlight, &_mcFlight, pc, canopy, _frontLeftAnchor, _frontRightAnchor, _rearLeftAnchor, _rearRightAnchor );
			}
        }
        else if( actionIs(CanopyOpening) )
        {
			// start flight action
			delete _action;
			_action = new Jumper::Flight( this, _phFlight, &_mcFlight );
			// event
			happen( this, EVENT_JUMPER_END_FREEFALL );
        }
    }

	canopy = this->getDominantCanopy();
	if (canopy && canopy->getRDSLevel() == 1.0f) {
		// remove slider
		// _sliderUp to 0
		// canopy reconnect

		// disconnect PC
		pc = this->getDominantPilotChute();
		if (pc) {
			pc->disconnect();
			if (pc == _pilotchuteSimulator) _pilotchuteSimulator = NULL;
			if (pc == _pilotchuteReserveSimulator) _pilotchuteReserveSimulator = NULL;
			delete pc;
			pc = NULL;
		}
	}

    // while landing check ground under feets
    if( actionIs(Landing) )
    {
        // check the ground under feets of jumper
        float maxDist = 0.5f;
        NxRay worldRay;
        worldRay.dir = wrap( _clump->getFrame()->getUp() );
        worldRay.dir.normalize();
        worldRay.orig = wrap( _clump->getFrame()->getPos() );
        worldRay.orig += worldRay.dir * 0.1f;
        worldRay.dir.set( 0, -1, 0 );
        
        NxRaycastHit raycastHit;
        if( !_scene->getPhTerrainShape()->raycast( worldRay, maxDist, NX_RAYCAST_DISTANCE | NX_RAYCAST_NORMAL, raycastHit, true ) )
        {
            delete _action;
            _action = new BadLanding( this, _phFlight, &_mcFlight );
            happen( this, EVENT_EASY_CRY );
        }

        // check the jumper's stability
        Vector3f charUp = _clump->getFrame()->getUp();
        charUp.normalize();
        Vector3f worldUp(0,1,0);
        Vector3f axis; axis.cross( worldUp, charUp );
        float angle = fabs( ::calcAngle( charUp, worldUp, axis ) );
        if( angle > 45.0f )
        {
            delete _action;
            _action = new BadLanding( this, _phFlight, &_mcFlight );
            happen( this, EVENT_EASY_CRY );
        }
    }
}

void Jumper::onRoamingProcAnim(float dt)
{
    // this procedural animation only workd in enclosure mode
    if( !_enclosure ) return;

    // get distance to abyss
    _distanceToAbyss = getDistanceToAbyss();
    bool lookInAbyss = ( _distanceToAbyss >= 0 );

    // if character is far from abyss, leave this routine
    float necessaryIncidence = 0.0f;
    if( lookInAbyss ) 
    {
        necessaryIncidence = headLoweringLimit * ( 1.0f - _distanceToAbyss / glanceRayDistance );
    }
       
    if( _headIncidence > necessaryIncidence )
    {
        _headIncidence += -headLoweringVelocity * dt;
        if( _headIncidence < necessaryIncidence ) _headIncidence = necessaryIncidence;
    }
    else if( _headIncidence < necessaryIncidence )
    {
        _headIncidence += headLoweringVelocity * dt;
        if( _headIncidence > necessaryIncidence ) _headIncidence = necessaryIncidence;
    }

    // lower head bone 
    engine::IFrame* headFrame = getHeadFrame( _clump );
    Matrix4f boneMatrix = headFrame->getMatrix();
    Vector3f boneAxis( boneMatrix[2][0], boneMatrix[2][1], boneMatrix[2][2] );
    boneAxis.normalize();
    boneMatrix = Gameplay::iEngine->rotateMatrix( boneMatrix, boneAxis, _headIncidence );
    headFrame->setMatrix( boneMatrix );

    // deviate backbone
    engine::IFrame* backboneFrame = getBackBone( _clump );
    boneMatrix = backboneFrame->getMatrix();
    boneAxis.set( boneMatrix[2][0], boneMatrix[2][1], boneMatrix[2][2] );
    boneAxis.normalize();
    boneMatrix = Gameplay::iEngine->rotateMatrix( boneMatrix, boneAxis, backboneDeviationLimit * _headIncidence / headLoweringLimit );
    backboneFrame->setMatrix( boneMatrix );
}

void Jumper::onFreeFallingProcAnim(float dt)
{
    // process "look where you fly"
    if( !actionIs(Flip) )
    {
        Vector3f direction = wrap( _phFreeFall->getLinearVelocity() );
        direction.normalize();
        //lookWhereYouFly( direction, dt );
    }
}

void Jumper::onFlightProcAnim(float dt)
{
    if( actionIs(Flight) )
    {
        // process "look where you fly"
        Vector3f direction = wrap( _phFlight->getLinearVelocity() );
        direction.normalize();
        lookWhereYouFly( direction, dt );
    }
    else if( _scene->getCamera() )
    {
        // process look in camera
        Matrix4f cameraPose = _scene->getCamera()->getPose();
        lookAtPoint( Vector3f( cameraPose[3][0], cameraPose[3][1], cameraPose[3][2] ), dt );
    }
}

/**
 * abstraction layer
 */

void Jumper::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_JUMPER_ENUMERATE )
    {
        Jumper::JumperL* jumperList = reinterpret_cast<Jumper::JumperL*>( eventData );
        assert( jumperList );
        jumperList->push_back( this );
    }
    else if( eventId == EVENT_JUMPER_FIRSTPERSON && reinterpret_cast<Actor*>( eventData ) == this )
    {
        // hide head & relative gfx
        getHead( _clump, database::Face::getRecord( _virtues->appearance.face )->modelId )->setFlags( 0 );
        getHelmet( _clump, database::Helmet::getRecord( _virtues->equipment.helmet.id )->modelId )->setFlags( 0 );
        getLeftEye( _clump )->setFlags( 0 );
        getRightEye( _clump )->setFlags( 0 );

        // hide licensed heads
        #ifdef GAMEPLAY_EDITION_ND
            getHead( _clump, 4 )->setFlags( 0 );
        #endif
        #ifdef GAMEPLAY_EDITION_ATARI
            getHead( _clump, 4 )->setFlags( 0 );
        #endif
    }
    else if( eventId == EVENT_JUMPER_THIRDPERSON && reinterpret_cast<Actor*>( eventData ) == this )
    {
        unsigned int flags = engine::afCollision | engine::afRender;
        if( true || isPlayer() ) 
        {
            int shadows = atoi( Gameplay::iGameplay->getConfigElement( "video" )->Attribute( "shadows" ) );
            if( shadows ) flags = flags | engine::afCastShadow;
        }
        // show head & relative gfx 
        getHead( _clump, database::Face::getRecord( _virtues->appearance.face )->modelId )->setFlags( flags );
        getHelmet( _clump, database::Helmet::getRecord( _virtues->equipment.helmet.id )->modelId )->setFlags( flags );
        getLeftEye( _clump )->setFlags( engine::afRender );
        getRightEye( _clump )->setFlags( engine::afRender );
        // force LICENSED_CHAR appearance
        if( _player && _scene->getCareer()->getLicensedFlag() )
        {
            setLicensedCharacterAppearance();
            setNoHelmet( _clump );
        }
    }
    else if( eventId == EVENT_CANOPY_COLLAPSED )
    {
        // this event can be received from owned canopy only
        //if( actionIs(Flight) ) _hazardState = true;
    }    
    else if( eventId == EVENT_CAMERA_IS_ACTUAL )
    {
        onCameraIsActual();
    }
}

void Jumper::onContact(NxContactPair &pair, NxU32 events)
{
    // freefall actor?
    if( pair.actors[0] == _phFreeFall || pair.actors[1] == _phFreeFall )
    {
        // penalty for motion
        if( !_isContacted && ( events & NX_NOTIFY_ON_START_TOUCH || events & NX_NOTIFY_ON_TOUCH ) )
        {
            // penalty for velocity
            NxContactStreamIterator iter( pair.stream );
            while( iter.goNextPair() )
            {
                while( iter.goNextPatch() )
                {
                    const NxVec3& penaltyN = iter.getPatchNormal();
                    float penalty = -penaltyN.dot( _phFreeFall->getLinearVelocity() );
                    _phFreeFall->addForce( penaltyN * penalty, NX_VELOCITY_CHANGE );
                }
            }
        }

        // damage health
        if( !_isContacted && events & NX_NOTIFY_ON_START_TOUCH )
        {
            onDamage( pair.sumNormalForce.magnitude(), pair.sumFrictionForce.magnitude(), _phFreeFall->getLinearVelocity().magnitude() );
        }

        // enable collision
        if( events & NX_NOTIFY_ON_END_TOUCH )
        {
            _isContacted = false;
            unsigned int flags = _scene->getPhScene()->getActorPairFlags( *_scene->getPhTerrain(), *_phFreeFall );
            flags = flags & ~NX_NOTIFY_ON_END_TOUCH;
            _scene->getPhScene()->setActorPairFlags( *_scene->getPhTerrain(), *_phFreeFall, flags );
        }
    }
    // flight actor?
    else if( pair.actors[0] == _phFlight || pair.actors[1] == _phFlight )
    {
		pair.sumNormalForce.magnitude();
		pair.sumFrictionForce.magnitude();
		_phFlight->getLinearVelocity().magnitude();

        // damage health
        onDamage( pair.sumNormalForce.magnitude(), pair.sumFrictionForce.magnitude(), _phFlight->getLinearVelocity().magnitude() );

        // process actions
        if( events & NX_NOTIFY_ON_TOUCH && !actionIs(Landing) && !actionIs(BadLanding) )
        {   
            float maxDist = 0.5f;
            NxRay worldRay;
            worldRay.dir = wrap( _clump->getFrame()->getUp() );
            worldRay.dir.normalize();
            worldRay.orig = wrap( _clump->getFrame()->getPos() );
            worldRay.orig += worldRay.dir * 0.1f;
            worldRay.dir.set( 0, -1, 0 );

            NxRaycastHit raycastHit;
            if( _scene->getPhTerrainShape()->raycast( worldRay, maxDist, NX_RAYCAST_DISTANCE | NX_RAYCAST_NORMAL, raycastHit, true ) )
            {
                // check surface inclination
                float maxInclination = 0.47f;
                float inclination = raycastHit.worldNormal.dot( NxVec3(0,1,0) );

                if( inclination < maxInclination )
                {
                    if( _phFlight->getLinearVelocity().magnitude() < 4.0f )
                    {
                        delete _action;
                        _action = new BadLanding( this, _phFlight, &_mcFlight );
                        happen( this, EVENT_EASY_CRY );
                    }
                }
                // check velocity of jumper
                else if( _phFlight->getLinearVelocity().magnitude() < 8.0f )
                {
                    // reset canopy controls
					this->getDominantCanopy()->reset();

                    // check landing angle
                    Vector3f charUp = _clump->getFrame()->getUp();
                    charUp.normalize();
                    Vector3f worldUp = Vector3f(0,1,0);
                    Vector3f axis;
                    axis.cross( worldUp, charUp );
                    axis.normalize();
                    float angle = ::calcAngle( worldUp, charUp, axis ); 

                    delete _action;
                    // if jumper stands on feets and doesn't hardly hurt
                    if( fabs( angle ) < 30.0f && _virtues->evolution.health > 0.5f )
                    {
                        // succesful landing
                        _action = new Landing( this, _phFlight, &_mcFlight );
                    }
                    else
                    {
                        _action = new BadLanding( this, _phFlight, &_mcFlight );
                        happen( this, EVENT_EASY_CRY );
                    }                    
                }
            }
        }
    }
}

float Alt_prev = -1000.0f;
void Jumper::onUpdateActivity(float dt)
{
	if (_phase == jpCreating) {
		return;
	}


	// DEBUG stuff
	if (_debug_window) {
		/*
		Matrix4f pose = getPose();

		float altitude = pose[3][1] / 100.0f;
		if (Alt_prev <= -999.0f) Alt_prev = altitude;
		const float real_Vvert = (altitude - Alt_prev)/dt;
		Alt_prev = altitude;

		deltaTime = dt;
		bmp.pressure = altitude <= 10000.0f ? (1.196f - 0.0000826f * altitude) : (0.27f);
		bmp.pressure = (bmp.pressure / 1.2041f) * 101.325f;
		bmp.temperature = 15.0f;
		bmp.altitude = altitude + getCore()->getRandToolkit()->getUniform(-5.0f, 5.0f);
		
		const bool Vvert_log = (Vvert_timer <= deltaTime);
		loop();
		
		if (Vvert_log) {
			getCore()->logMessage("%2.4f\t%2.4f", real_Vvert, Vvert);
		}

		gui::IGuiPanel* panel = this->_debug_window->getPanel()->find( "Message" );
		panel->getStaticText()->setText( myGLCD.buffer );
		*/

		// network debug
		//Matrix4f pose = getPose();

		gui::IGuiPanel* panel = this->_debug_window->getPanel()->find( "Message" );
		panel->getStaticText()->setText( wstrformat(L"FPS: %2.5f\n", 1/dt).c_str());

		//panel->getStaticText()->setText( wstrformat(L"%2.1f %2.1f %2.1f %2.1f\n%2.1f %2.1f %2.1f %2.1f\n%2.1f %2.1f %2.1f %2.1f\n%2.1f %2.1f %2.1f %2.1f\n", pose[0][0], pose[0][1], pose[0][2], pose[0][3], pose[1][0], pose[1][1], pose[1][2], pose[1][3], pose[2][0], pose[2][1], pose[2][2], pose[2][3], pose[3][0], pose[3][1], pose[3][2], pose[3][3]).c_str());
		//panel->getStaticText()->setText( wstrformat(L"timestamp: %3.5f\nNet id: %d", timestamp, network_id).c_str() );
	}

	_dt = dt;

    if( _player ) 
    {
        _spinalCord->mapActionChannels();
        if( !_phaseIsEnabled ) _spinalCord->phase = false;
    }

    switch( _phase )
    {
    case jpRoaming:
        onRoaming( dt );
        break;
    case jpFreeFalling:
        onFreeFalling( dt );
        _jumpTime += dt;
        break;
    case jpFlight:
        onFlight( dt );
        _jumpTime += dt;
        break;
    }

    if( _player )
    {
        // after 2 seconds of jump time airplane enables rough mode
        if( _airplane )
        {
            if( !_airplane->isRoughMode() && _jumpTime > 2.0f ) 
            {
                _airplane->setRoughMode( true );
            }
            // after 4 seconds of jump time airplane enables landing mode
            if( !_airplane->isLandingMode() && _jumpTime > 4.0f ) 
            {
                _airplane->setLandingMode( true );
            }
        }
    }

    // inherited behaviour
    Character::onUpdateActivity( dt );

    // sound processing
    float freefallVel = 0.0f;
    float freefallMod = 1.0f;
    if( _phase == ::jpFreeFalling )
    {
        freefallVel = _phFreeFall->getLinearVelocity().magnitude();
        if( actionIs(Tracking) )
        {
            Tracking* tracking = dynamic_cast<Tracking*>( _action );
            freefallMod += 0.5f * fabs( tracking->getTrackingModifier() );
        }
    }
    else if( _phase == ::jpFlight )
    {
        if( !_phFreeFall->isSleeping() )
        {
            freefallVel = _phFreeFall->getLinearVelocity().magnitude();
        }
        else
        {
            freefallVel = _phFlight->getLinearVelocity().magnitude();
        }
    }
    happen( this, EVENT_JUMPER_FREEFALL_VELOCITY, &freefallVel );
    happen( this, EVENT_JUMPER_FREEFALL_MODIFIER, &freefallMod );

    // afterproc.
    if( !_isContacted && _phFreeFall->readActorFlag( NX_AF_DISABLE_RESPONSE ) )
    {
        _phFreeFall->clearActorFlag( NX_AF_DISABLE_RESPONSE );
    }

    // procedural animation
    switch( _phase )
    {
    case jpRoaming:
        onRoamingProcAnim( dt );
        break;
    case jpFreeFalling:
        onFreeFallingProcAnim( dt );
        break;
    case jpFlight:
        onFlightProcAnim( dt );
        break;
    }

    // update "jumper is over its activity" status
    if( !_isOverActivity )
    {
        // player is dead?
        if( _virtues->evolution.health == 0.0f )
        {
            _isOverActivity = true;
        }
        else if( _phase == ::jpFlight )
        {
            // player velocity
            float velocity = getVel().length() / 100.0f;
            // velocity treshold
            if( velocity < 0.25f )
            {
                _isOverActivity = true;
            }
        }
    }


	/// NETWORK

	// request a network id
	if (_phase == jpFreeFalling &&
		this->network_id == -1 &&
		_player &&
		!this->_phFreeFall->isSleeping()) {
			NetworkData *packet = new NetworkData;
			packet->timestamp = -Gameplay::iGameplay->getGlobalTime();
			packet->receiver_type = NET_REC_MISSION;
			packet->receiver_id = this->network_id;
			packet->data_type = 1;	// request for a network ID and send Virtues

			packet->data = new Virtues;
			packet->data_length = sizeof Virtues;
			memcpy(packet->data, _virtues, sizeof Virtues);

			if (!getScene()->network->arePacketsSendingLocked()) {
				getScene()->network->addPacketToBuffer(packet);
				getScene()->network->switchBuffers();
				this->network_id = -2;	// set to waiting
			}
			return;
	}

	// send information as player
	if (!this->_phFreeFall->isSleeping() && _player && this->network_id > 0) {
		const float timestamp = -Gameplay::iGameplay->getGlobalTime();
		NxMat34 globalpose = _phFreeFall->getGlobalPose();
		NxVec3 linearvel = _phFreeFall->getLinearVelocity();
		NxVec3 angularvel = _phFreeFall->getAngularVelocity();

		NetworkData *packet;

		// send matrix
		//packet = new NetworkData;
		//packet->timestamp = timestamp;
		//packet->receiver_type = NET_REC_JUMPER;
		//packet->receiver_id = this->network_id;
		//packet->data_type = 2;	// global pose
		//packet->data = new NxMat34;
		//packet->data_length = sizeof globalpose;
		//memcpy(packet->data, &globalpose, sizeof globalpose);
		//getScene()->network->addPacketToBuffer(packet);

		// send linear velocity
		packet = new NetworkData;
		packet->timestamp = timestamp;
		packet->receiver_type = NET_REC_JUMPER;
		packet->receiver_id = this->network_id;
		packet->data_type = 4;	// linear velocity
		packet->data = new NxVec3;
		packet->data_length = sizeof linearvel;
		memcpy(packet->data, &linearvel, sizeof linearvel);
		getScene()->network->addPacketToBuffer(packet);

		// send angular velocity
		packet = new NetworkData;
		packet->timestamp = timestamp;
		packet->receiver_type = NET_REC_JUMPER;
		packet->receiver_id = this->network_id;
		packet->data_type = 5;	// angular velocity
		packet->data = new NxVec3;
		packet->data_length = sizeof angularvel;
		memcpy(packet->data, &angularvel, sizeof angularvel);
		getScene()->network->addPacketToBuffer(packet);

		// send position
		NxVec3 pos = _phFreeFall->getGlobalPosition();
		packet = new NetworkData;
		packet->timestamp = timestamp;
		packet->receiver_type = NET_REC_JUMPER;
		packet->receiver_id = this->network_id;
		packet->data_type = 1;	// global position
		packet->data = new NxVec3;
		packet->data_length = sizeof pos;
		memcpy(packet->data, &pos, sizeof pos);
		getScene()->network->addPacketToBuffer(packet);

		// send spine
		float controls[4] = {_spinalCord->left, _spinalCord->right, _spinalCord->up, _spinalCord->down};
		packet = new NetworkData;
		packet->timestamp = timestamp;
		packet->receiver_type = NET_REC_JUMPER;
		packet->receiver_id = this->network_id;
		packet->data_type = 3;	// spinal cord
		packet->data = new float[4];
		packet->data_length = sizeof(float) * 4;
		memcpy(packet->data, &controls, sizeof(float) * 4);
		getScene()->network->addPacketToBuffer(packet);

		// send controls
		//packet->timestamp = timestamp;
		//packet->receiver_type = NET_REC_JUMPER;
		//packet->receiver_id = 1;
		//packet->data_type = 3;	// spinal cord
		//packet->data = new SpinalCord;
		//packet->data_length = sizeof SpinalCord;
		//memcpy(packet->data, _spinalCord, sizeof SpinalCord);
		//getScene()->network->sendPacketAsync(packet);

		getScene()->network->switchBuffers();
	}

	/// receive packets as NPC
	if (!_player) {
		int net_id = network_id;
		if (net_id < 0) {
			net_id = -net_id;
		}
		//NetworkData *packet = getScene()->network->consumePacketByReceiver(NET_REC_JUMPER, net_id);
		//int limit = 3;
		while (true) {
			Scene *scene = getScene();
			Network *network = scene->network;

			NetworkData *packet = network->consumePacketByReceiver(NET_REC_JUMPER, net_id);
			if (packet == NULL) break;
			getCore()->logMessage("JUMPER PACKET");
			consumePacket(packet);
		}
	}
}

void Jumper::consumePacket(NetworkData *packet) {
	//if (packet->data_type == 1) {
	//	NxVec3 pos;
	//	memcpy(&pos, packet->data, sizeof NxVec3);
	//	getCore()->logMessage("Got position: %2.5f %2.5f %2.5f", pos.x, pos.y, pos.z);
	//}
	//if (packet->data_type == 601) {
	//	NxVec3 pos;
	//	pos.x = 0;
	//	pos.z = 0;
	//	memcpy(&pos.y, packet->data, sizeof pos.y);
	//	if (pos.y < -1000000000000000000000.0f || pos.y > 1000000000000000000000.0f ) {
	//		getCore()->logMessage("Got Y CRAP: %2.5f", pos.y);
	//	} else {
	//		getCore()->logMessage("Got Y: %2.5f", pos.y);
	//	}
	//}
	//if (packet->data_type != 601) return;

	/// RECEIVE NETWORK PACKETS AS NPC
	if (/*!this->_phFreeFall->isSleeping() &&*/ !_player) {
		
		// position vector NxVec3
		if (packet->data_type == 1) {
			NxVec3 pos;
			memcpy(&pos, packet->data, sizeof NxVec3);
			if (pos.x > -1000000000000000000000.0f && pos.x < 1000000000000000000000.0f &&
				pos.y > -1000000000000000000000.0f && pos.y < 1000000000000000000000.0f &&
				pos.z > -1000000000000000000000.0f && pos.z < 1000000000000000000000.0f) {


				// damping
				NxVec3 gpos = _phFreeFall->getGlobalPosition();
					//getCore()->logMessage("Now position: %2.4f %2.4f %2.4f", gpos.x, gpos.y, gpos.z);
					//getCore()->logMessage("Got position: %2.4f %2.4f %2.4f", pos.x, pos.y, pos.z);
				NxVec3 dPos = (pos - gpos) * 0.3f;
				gpos += dPos;

					//getCore()->logMessage("Gave position: %2.4f %2.4f %2.4f", gpos.x, gpos.y, gpos.z);
				_phFreeFall->setGlobalPosition(gpos);
				timestamp = packet->timestamp;
			} else return;
		// global pose matrix
		} else if (packet->data_type == 2) {
			NxMat34 pose;
			memcpy(&pose, packet->data, sizeof NxMat34);
			NxVec3 check;
			for (int i = 0; i < 3; ++i) {
				check = pose.M.getColumn(i);
				if (check.x < -1000000000000000000000.0f || check.x > 1000000000000000000000.0f &&
					check.y < -1000000000000000000000.0f || check.y > 1000000000000000000000.0f &&
					check.z < -1000000000000000000000.0f || check.z > 1000000000000000000000.0f) {
						return;
				}
			}
			check = pose.t;
			if (check.x < -1000000000000000000000.0f || check.x > 1000000000000000000000.0f &&
					check.y < -1000000000000000000000.0f || check.y > 1000000000000000000000.0f &&
					check.z < -1000000000000000000000.0f || check.z > 1000000000000000000000.0f) {
						return;
				}
			
			_phFreeFall->setGlobalPose(pose);

		// controls
		} else if (packet->data_type == 3) {
			float controls[4];

			memcpy(controls, packet->data, sizeof(float)*4);
			_spinalCord->left = controls[0];
			_spinalCord->right = controls[1];
			_spinalCord->up = controls[2];
			_spinalCord->down = controls[3];

		// linear velocity
		} else if (packet->data_type == 4) {
			NxVec3 vel;
			memcpy(&vel, packet->data, sizeof NxVec3);
			if (vel.x > -1000000000000000000000.0f && vel.x < 1000000000000000000000.0f &&
				vel.y > -1000000000000000000000.0f && vel.y < 1000000000000000000000.0f &&
				vel.z > -1000000000000000000000.0f && vel.z < 1000000000000000000000.0f) {
				_phFreeFall->setLinearVelocity(vel);
			} else return;
		// angular velocity
		} else if (packet->data_type == 5) {
			return;
			//NxVec3 vel;
			//memcpy(&vel, packet->data, sizeof NxVec3);
			//_phFreeFall->setAngularVelocity(vel);
		// jumper pose
		} else if (packet->data_type == 6) {
			return;
			//int position;
			//memcpy(&position, packet->data, sizeof position);
			//if (actionIs(Jumper::Tracking)) {
			//	Jumper::Tracking *tracking = dynamic_cast<Jumper::Tracking*>( _action );
			//	tracking->changePose(position, 1.0f/0.5f, true);
			//} else {
			//	_default_pose = position;
			//}

		// experimental
		} else if (packet->data_type == 601) {
			/*NxVec3 pos = _phFreeFall->getGlobalPosition();
			pos.x = 0;
			pos.z = 0;
			memcpy(&pos.y, packet->data, sizeof pos.y);
			if (pos.y > -1000000000000000000000.0f && pos.y < 1000000000000000000000.0f ) {
				_phFreeFall->setGlobalPosition(pos);
			}*/
		}

		// was waiting but got something, so we can begin
		if (_phFreeFall->isSleeping()) {
			//_phFreeFall->setGlobalPosition(NxVec3(0.0f, 4000.0f, 0.0f));
			//_phFreeFall->setLinearVelocity(NxVec3(0.0f, 0.0f, 0.0f));
			beginFreefall();
		}
	}
}

void Jumper::onUpdatePhysics(void)
{
	if (_tandemStudent != NULL && _tandemJoint[0] == NULL) {
		_tandemStudent->getSpinalCord()->phase = _spinalCord->phase;
		_tandemStudent->getSpinalCord()->trigger_phase = _spinalCord->trigger_phase;
	}

	if (_phase == jpCreating) {
		return;
	}

	// tandem test
//	if (_spinalCord->leftReserve) {
//		for( JumperI jumperI = _jumperL.begin(); jumperI != _jumperL.end(); jumperI++ ) {
//			if( (*jumperI) != this ) {
//				if ((*jumperI)->getFreefallActor()->isSleeping()) {
//					continue;
//				}
//
//				if (_phJoint) {
//					_scene->getPhScene()->releaseJoint( *_phJoint );
//				}
//				NxDistanceJointDesc jointDesc;
//				if (!_phFreeFall->isSleeping()) {
//					jointDesc.actor[0] = _phFreeFall;
//				} else {
//					jointDesc.actor[0] = _phFlight;
//				}
//const float maxDistance = 0.006f;
//const float y = 0.3f;
//
//				jointDesc.actor[1] = (*jumperI)->getFreefallActor();
//				jointDesc.maxDistance = maxDistance;
//				jointDesc.minDistance = 0.0f;
//				jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
//				jointDesc.localAnchor[0] = NxVec3(-0.2f,0.0f,0.2f);
//				jointDesc.localAnchor[1] = NxVec3(-0.2f,y,0.2f);
//				jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
//				_phJoint = _scene->getPhScene()->createJoint( jointDesc ); 
//				
//
//				//NxDistanceJointDesc jointDesc;
//				if (!_phFreeFall->isSleeping()) {
//					jointDesc.actor[0] = _phFreeFall;
//				} else {
//					jointDesc.actor[0] = _phFlight;
//				}
//
//				jointDesc.actor[1] = (*jumperI)->getFreefallActor();
//				jointDesc.maxDistance = maxDistance;
//				jointDesc.minDistance = 0.0f;
//				jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
//				jointDesc.localAnchor[0] = NxVec3( 0.2f,0.0f,0.2f);
//				jointDesc.localAnchor[1] = NxVec3( 0.2f,y,0.2f);
//				jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
//				_phJoint = _scene->getPhScene()->createJoint( jointDesc ); 
//
//				//NxDistanceJointDesc jointDesc;
//				if (!_phFreeFall->isSleeping()) {
//					jointDesc.actor[0] = _phFreeFall;
//				} else {
//					jointDesc.actor[0] = _phFlight;
//				}
//
//				jointDesc.actor[1] = (*jumperI)->getFreefallActor();
//				jointDesc.maxDistance = maxDistance;
//				jointDesc.minDistance = 0.0f;
//				jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
//				jointDesc.localAnchor[0] = NxVec3( 0.2f,0.0f,-0.2f);
//				jointDesc.localAnchor[1] = NxVec3( 0.2f,y-0.05f,-0.2f);
//				jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
//				_phJoint = _scene->getPhScene()->createJoint( jointDesc ); 
//
//				//NxDistanceJointDesc jointDesc;
//				if (!_phFreeFall->isSleeping()) {
//					jointDesc.actor[0] = _phFreeFall;
//				} else {
//					jointDesc.actor[0] = _phFlight;
//				}
//
//				jointDesc.actor[1] = (*jumperI)->getFreefallActor();
//				jointDesc.maxDistance = maxDistance;
//				jointDesc.minDistance = 0.0f;
//				jointDesc.flags = NX_DJF_MAX_DISTANCE_ENABLED | NX_DJF_MIN_DISTANCE_ENABLED;      
//				jointDesc.localAnchor[0] = NxVec3(-0.2f,0.0f,-0.2f);
//				jointDesc.localAnchor[1] = NxVec3(-0.2f,y-0.05f,-0.2f);
//				jointDesc.jointFlags = NX_JF_VISUALIZATION | NX_JF_COLLISION_ENABLED;
//				_phJoint = _scene->getPhScene()->createJoint( jointDesc ); 
//				break;
//			}
//		}
//		_spinalCord->leftReserve = false;
//	}





    // save ghost state
    if( _saveGhost ) _saveGhost->update( ::simulationStepTime );

    // bugfix
    if( _phase == ::jpFreeFalling || _phase == ::jpFlight )
    {
        NxActor* phActor = NULL;
        
        if( !_phFreeFall->isSleeping() ) 
        {
            phActor = _phFreeFall;
        }
        else if( !_phFlight->isSleeping() )
        {
            phActor = _phFlight;
        }
        
        if( _player && phActor && !phActor->isSleeping() )
        {
            float maxDist = phActor->getLinearVelocity().magnitude() * ::simulationStepTime;

            NxRay worldRay;         
            worldRay.orig = phActor->getGlobalPosition();
            worldRay.dir  = phActor->getLinearVelocity();
            worldRay.dir.normalize();
            worldRay.orig = worldRay.orig - worldRay.dir * maxDist;

            NxShape** terrainShapes = _scene->getPhTerrain()->getShapes();
            NxTriangleMeshShape* triMeshShape = terrainShapes[0]->isTriangleMesh();
            assert( triMeshShape );

            NxRaycastHit raycastHit;
			if( _tandemJoint[0] == NULL && triMeshShape->raycast( worldRay, maxDist, NX_RAYCAST_DISTANCE | NX_RAYCAST_NORMAL, raycastHit, true ) )
            {
                NxVec3 penaltyDir = raycastHit.worldNormal; penaltyDir.normalize();
                float  penaltyValue = -penaltyDir.dot( _phFreeFall->getLinearVelocity() );
                phActor->setGlobalPosition( phActor->getGlobalPosition() - worldRay.dir * maxDist );
                phActor->addForce( penaltyDir * penaltyValue, NX_VELOCITY_CHANGE );
                phActor->addForce( penaltyDir * 10.0f, NX_VELOCITY_CHANGE );

                // damage jumper
                float force = _virtues->appearance.weight * phActor->getLinearVelocity().magnitude() / ::simulationStepTime;
            
                //getCore()->logMessage( "penetration force: %3.2f", force );

                onDamage( 
                    force, 
                    0.0f, 
                    phActor->getLinearVelocity().magnitude() 
                );
            }
        }
    }

    // invoke action
    _action->updatePhysics();

    // calculate burden    
    if( _phase != jpRoaming )
    {
        _bcStep++;
        if( _bcStep >= bcStepout )
        {
            NxVec3 measureVel( 0,0,0 );
            if( !_phFreeFall->isSleeping() )
            {
                measureVel = _phFreeFall->getLinearVelocity();
            }
            else
            {
                measureVel = _phFlight->getLinearVelocity();
            }
            _bcBurden.set( 0, -9.8f, 0 );
            _bcBurden -= ( measureVel - _bcPrevVel ) / ( float( _bcStep ) * ::simulationStepTime );
            _bcPrevVel = measureVel;
            _bcStep = 0;
        }
        
        // overburden is damage equipped rig
        if( _phase == jpFlight )
        {
            _virtues->onJumperOverburden( ( _bcBurden.magnitude() / 9.8f ), ::simulationStepTime );
        }
    }

    onUpdateHealthStatus();
    onUpdateSkills();

    // trajectory influence
    NxActor* phActor = NULL;
    switch( _phase )
    {
    case jpFreeFalling:
        phActor = _phFreeFall;
        break;
    case jpFlight:
        phActor = _phFlight;
        break;
    }
    if( phActor )
    {
        // ... perturbation ...
    }
}

void Jumper::onUpdateHealthStatus(void)
{
    // adrenaline rush
    if( _phase == jpRoaming )
    {
        // adrenaline level will raise when character approach the abyss
        if( _distanceToAbyss >= 0 )
        {
            float maxDistance = 500.0f;
            float maxRush = 0.0f;
            float minDistance = 50.0f;
            float minRush = 0.0125f;

            float factor = ( _distanceToAbyss - minDistance ) / ( maxDistance - minDistance );
            factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );

            float rush = minRush * ( 1 - factor ) + maxRush * factor;

            _adrenaline += rush * ::simulationStepTime;
            
            if( _adrenaline > _jerkLimit ) _adrenaline = _jerkLimit;
        }
        else
        {
            // adrenalize outflow
            float outflow = 0.005f;
            _adrenaline -= outflow * ::simulationStepTime;
            if( _adrenaline < 0 ) _adrenaline = 0;

            // adrenalize rush caused by fast running
            if( _spinalCord->modifier && actionIs(WalkForward) )
            {
                float runRush = 0.025f;
                _adrenaline += runRush * ::simulationStepTime;
                if( _adrenaline > _jerkLimit ) _adrenaline = _jerkLimit;
            }
        }
    }
    else if( _phase == jpFreeFalling || ( _phase == jpFlight && _phFlight->isSleeping() ) )
    {
        float burden = _bcBurden.magnitude() / 9.8f;
        
        float minBurden = 0.0f;
        float minRush   = 1.0f;
        float maxBurden = 0.1f;
        float maxRush   = 0.0f;

        float factor = ( burden - minBurden ) / ( maxBurden - minBurden );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );

        float rush = minRush * ( 1 - factor ) + maxRush * factor;

        _adrenaline += rush * ::simulationStepTime;
        
        if( _adrenaline > _fallLimit ) _adrenaline = _fallLimit;
    }
    else if( !_phFlight->isSleeping() )
    {
        float burden = _bcBurden.magnitude() / 9.8f;
        
        float minBurden = 0.0f;
        float minRush   = 0.5f;
        float maxBurden = 1.0f;
        float maxRush   = -0.01f;

        float factor = ( burden - minBurden ) / ( maxBurden - minBurden );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );

        float rush = minRush * ( 1 - factor ) + maxRush * factor;

        if( rush > 0 && _adrenaline < _jerkLimit )
        {
            _adrenaline += rush * ::simulationStepTime;
            if( _adrenaline > _jerkLimit ) _adrenaline = _jerkLimit;
        }
        else if( rush < 0 )
        {
            _adrenaline += rush * ::simulationStepTime;
            if( _adrenaline < 0 ) _adrenaline = 0;
        }
    }

    // detect "flying-near-wall" condition (hazard state)
    if( _phase == ::jpFreeFalling )
    {
        NxSphere worldSphere;
        worldSphere.center = _phFreeFall->getGlobalPosition();
        worldSphere.radius = 0.5f * _phFreeFall->getLinearVelocity().magnitude();
        if( _scene->getPhTerrainShape()->checkOverlapSphere( worldSphere ) )
        {
            _hazardState = true;
        }    
    }

    // adrenaline rush caused by hazard situation
    if( _hazardState )
    {
        float hazardRush = 2.0f;
        _adrenaline += hazardRush * ::simulationStepTime;
        if( _adrenaline > 1 ) _adrenaline = 1;
    }

    // pulse
    float minAdrenaline = 0.0f;
    float minPulse      = 60.0f;
    float maxAdrenaline = 1.0f;
    float maxPulse      = 120.0f;
    float pulseIncVel   = 5.0f;
    float pulseDecVel   = 0.25f;

    float factor = ( _adrenaline - minAdrenaline ) / ( maxAdrenaline - minAdrenaline );
    factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
    float rushPulse = float( int( minPulse * ( 1.0f - factor ) + maxPulse * factor ) );

    if( _pulse < rushPulse )
    {
        _pulse += pulseIncVel * ::simulationStepTime;
        _pulse = ( _pulse > rushPulse ) ? rushPulse : _pulse;
    }
    if( _pulse > rushPulse )
    {
        _pulse -= pulseDecVel * ::simulationStepTime;
        _pulse = ( _pulse < rushPulse ) ? rushPulse : _pulse;
    }

    // time effect;
    // when adrenaline rushes certain treshold, 
    // time effect appeared with certain probability
    float minRush  = 0.5f;    
    float minCons  = 0.03125f;
    float maxRush  = 1.0f;
    float maxCons  = 0.0625f;
    if( _adrenaline > minRush )
    {
        // adrenaline consumption
        float factor  = ( _adrenaline - minRush ) / ( maxRush - minRush );        
        float cons    = minCons * ( 1 - factor ) + maxCons * factor;

		if (_player) {
			float vel = _phFreeFall->getLinearVelocity().magnitude();
			//if (vel > 50.0f) {
			//	float adrenaline = vel / 100.0f;
			//	if (vel > 1.0f) adrenaline = 1.0f;
			//	_scene->setTimeSpeed( _virtues->getTimeShift( 1.0f + adrenaline ) );
			//}
		}
        // time shift
        //if( _player ) _scene->setTimeSpeed( _virtues->getTimeShift( _adrenaline ) );


        // consume adrenaline
        _adrenaline -= cons * ::simulationStepTime;
    }

    // reset hazard state
    _hazardState = false;

    // shock caused by overburden
    float overburden = getOverburden().magnitude() / 9.8f;
    float overburdenShock = _virtues->getShock( overburden );
    if( _shock < overburdenShock )
    {
        _shock += 4 * overburdenShock * ::simulationStepTime;
        if( _shock > overburdenShock ) _shock = overburdenShock;
    }

    // pain shock (caused by damage)
    float pain = pow( ( 1 - _virtues->evolution.health ), 2 );
    if( _shock < pain )
    {
        _shock += pain * ::simulationStepTime;
        if( _shock > pain ) _shock = pain;
    }

    // normal shock restoration
    float minRestoration = 0.125f;
    float maxRestoration = 0.03125f;
    float restoration = minRestoration * ( 1 - _shock ) + maxRestoration * _shock;
    _shock -= restoration * ::simulationStepTime;
    if( _shock < 0 ) _shock = 0;

    // damage caused by overburden
    if( _phase != ::jpRoaming )
    {
        float minOverburden = _virtues->getMaxEndurableOverburden();
        float minDamage = 0.0f;
        float maxOverburden = 24.0f;    
        float maxDamage = 6.0f;
        factor = ( overburden - minOverburden ) / ( maxOverburden - minOverburden );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        float damage = minDamage * ( factor - 1 ) + maxDamage * factor;

        if( _virtues->evolution.health > 0 && damage > 0.05f )
        {
            if( damage < 0.5f ) happen( this, EVENT_EASY_CRY, NULL );
            else if( damage < 0.75f ) happen( this, EVENT_MEDIUM_CRY, NULL );
            else happen( this, EVENT_HARD_CRY, NULL );
        }

        _virtues->evolution.health -= damage * ::simulationStepTime;
        if( _virtues->evolution.health < 0 ) _virtues->evolution.health = 0;
    }
}

void Jumper::onUpdateSkills(void)
{
    if( !_virtues->equipment.experience ) return;

    // character doesn't receive experience in landing/bad landing
    if( _phase == ::jpFlight && !actionIs(Flight) ) return;

    // in adrenaline rush character receives perception experience
    if( _adrenaline > 0.5f )
    {
        _virtues->skills.perception += _virtues->predisp.perception * ::simulationStepTime * _adrenaline;
    }
    // in torture character receives endurance experience
    if( _shock > 0 )
    {
        // shock level caused by damage (doesn't considered)
        float pain = pow( ( 1 - _virtues->evolution.health ), 2 );
        float rewardedShock = _shock - pain;
        if( rewardedShock < 0 ) rewardedShock = 0;
        // finalize experience
        _virtues->skills.endurance += _virtues->predisp.endurance * ::simulationStepTime * rewardedShock;
    }
    if( _phase == ::jpFreeFalling )
    {
        // in action during freefall character receives tracking experience
        if( _spinalCord->modifier ||
            ( _spinalCord->left + _spinalCord->right + _spinalCord->down + _spinalCord->up ) > 0 )
        {
            _virtues->skills.tracking += _virtues->predisp.tracking * ::simulationStepTime;
        }
    }
}

void Jumper::damage(float normalForce, float frictionForce, float velocity)
{
    onDamage( normalForce, frictionForce, velocity );
}

void Jumper::onDamage(float normalForce, float frictionForce, float velocity)
{
    // sum of damage forces
    float force = normalForce + 0.5f * frictionForce;

    // PHYSICS BUG
    // - To avoid body penetrarion, sometimes physics engine applies a huge force
    //   to the body, and this cause corresponding damage.
    // - To solve this problem:
    //    - first, we are assume, thet damage depends on velocity of body
    //    - second, we are inctroduce per-action damage multiplier

    // first step of bug solution
    float minVelocity = 0.0f;
    float minFactor   = 0.0f;
    float maxVelocity = 5.0f;
    float maxFactor   = 1.0f;
    float factor = ( velocity - minVelocity ) / ( maxVelocity - minVelocity );
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
    force *= factor;

    // second step of bug solution
    force *= _action->getDamageFactor();

    float damage = 0;

    if( _finite( force ) )
    {
        float minForce  = 5000.0f;
        float minDamage = 0.0f;
        float maxForce  = 100000.0f;
        float maxDamage = 1.0f;
        float factor = ( force - minForce ) / ( maxForce - minForce );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        factor = pow( factor, 1.5f );
        damage = minDamage * ( 1 - factor ) + maxDamage * factor;
    }
    else
    {
        damage = 1.0f;
    }

    // reverse filter damage
    damage *= _virtues->evolution.health;

    // handle damage by equipped gear
    damage = _virtues->onJumperDamage( damage );

    // jumper reaction is crying
    if( _virtues->evolution.health > 0 && damage > 0.05f )
    {
        if( damage < 0.5f ) happen( this, EVENT_EASY_CRY, NULL );
        else if( damage < 0.75f ) happen( this, EVENT_MEDIUM_CRY, NULL );
        else happen( this, EVENT_HARD_CRY, NULL );
    }

    // apply damage
    _virtues->evolution.health -= damage;
    if( _virtues->evolution.health < 0 ) _virtues->evolution.health = 0;

    // in freefall - put baser in out of control for certain time
    if( _phase == ::jpFreeFalling && !actionIs(OutOfControl) )
    {
        // determine out of control penalty
        float minDamage  = 0;
        float minPenalty = 0;
        float maxDamage  = 1;
        float maxPenalty = 10;
        float factor = ( damage - minDamage ) / ( maxDamage - minDamage );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        float penalty = minPenalty * ( 1 - factor ) + maxPenalty * factor;
        // determine influence by endurance 
        float minEndurance = 0.0f;
        float minInfluence = 1.0f;
        float maxEndurance = 1;
        float maxInfluence = 0.25f;
        factor = ( _virtues->getEnduranceSkill() - minEndurance ) / ( maxEndurance - minEndurance );
        factor = ( factor < 0 ) ? 0 : ( ( factor > 1 ) ? 1 : factor );
        float influence = minInfluence * ( 1 - factor ) + maxInfluence * factor;
        penalty *= influence;
        // start action
        delete _action;
        _action = new OutOfControl( this, _phFreeFall, &_mcFreeFall, penalty );
    }

    if( _virtues->evolution.health<0 || _virtues->evolution.health>1 )
    {
        assert( !"HEALTH IS OUT OF RANGE!" );
    }
}

void Jumper::onCameraIsActual(void)
{
    if( _signatureType == stNone )
    {
        _signature->getPanel()->setVisible( false );
        return;
    }

    // update signature window
    Vector3f screenPos = Gameplay::iEngine->getDefaultCamera()->projectPosition( _clump->getFrame()->getPos() );
    if( screenPos[2] > 1 )
    {
        _signature->getPanel()->setVisible( false );
        return;
    }
    else
    {
        _signature->getPanel()->setVisible( true );
    }

    gui::Rect oldRect = _signature->getPanel()->getRect();
    gui::Rect newRect(
        int( screenPos[0] ), int( screenPos[1] ),
        int( screenPos[0] ) + oldRect.getWidth(),
        int( screenPos[1] ) + oldRect.getHeight()
    );
    _signature->getPanel()->setRect( newRect );

    // jumper name
    gui::IGuiPanel* panel = _signature->getPanel()->find( "JumperName" ); assert( panel && panel->getStaticText() );
    if( _player )
    {
        panel->getStaticText()->setText( asciizToUnicode( _scene->getCareer()->getName() ).c_str() );
    }
    else
    {
        NPC* npc = dynamic_cast<NPC*>( _parent );
		if( npc ) {
			if (network_id == -1 || network_id == -2) {
				panel->getStaticText()->setText( npc->getNPCName() );
			} else {
				panel->getStaticText()->setText( wstrformat(L"Network player #%d", network_id).c_str() );
			}
		}
    }

    // distance to jumper
    Vector3f distance = Gameplay::iEngine->getDefaultCamera()->getFrame()->getPos() - _clump->getFrame()->getPos();
    panel = _signature->getPanel()->find( "Distance" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( 
        wstrformat( Gameplay::iLanguage->getUnicodeString(219), distance.length() * 0.01f ).c_str()
    );

    // canopy info
    panel = _signature->getPanel()->find( "Canopy" ); assert( panel && panel->getStaticText() );
    if( _canopySimulator && _signatureType == stFull && _canopySimulator->getInflation() == 0 )
    {
        database::Canopy* canopyInfo = database::Canopy::getRecord( _virtues->equipment.canopy.id );
        std::wstring text = Gameplay::iLanguage->getUnicodeString(canopyInfo->nameId);
        text += L" - ";
        text += Gameplay::iLanguage->getUnicodeString(190);
        text += L" ";
        switch( _virtues->equipment.sliderOption )
        {
        case soRemoved:
            text += Gameplay::iLanguage->getUnicodeString(201);
            break;
        case soDown:
            text += Gameplay::iLanguage->getUnicodeString(200);
            break;
        case soUp:
            text += Gameplay::iLanguage->getUnicodeString(199);
            break;
        default:
            text += L"#ERR";
        }
        panel->getStaticText()->setText( text.c_str() );
    }
    else
    {
        panel->getStaticText()->setText( L"" );
    }    

    // piliotchute info
    panel = _signature->getPanel()->find( "PC" ); assert( panel && panel->getStaticText() );
    if( _canopySimulator && _signatureType == stFull && _pilotchuteSimulator && !_pilotchuteSimulator->isPulled() )
    {
        database::Canopy* canopyInfo = database::Canopy::getRecord( _virtues->equipment.canopy.id );
        database::Pilotchute* pcInfo = canopyInfo->pilots + _virtues->equipment.pilotchute;
        std::wstring text = wstrformat(
            Gameplay::iLanguage->getUnicodeString(384),
            int( pcInfo->size )
        );
        panel->getStaticText()->setText( text.c_str() );
    }
    else
    {
        panel->getStaticText()->setText( L"" );
    }
}

Vector3f Jumper::getVel(void)
{
    switch( _phase )
    {    
    case ::jpFreeFalling:
        return wrap( _phFreeFall->getLinearVelocity() );        
    case ::jpFlight:
        if( !actionIs(Pull) && !actionIs(PullReserve) )
        {
            return wrap( _phFlight->getLinearVelocity() );
        }
        else
        {
            return wrap( _phFreeFall->getLinearVelocity() );
        }
	case ::jpRoaming:
		if (_airplane) {
			return _airplane->getVel();
		}
    }
    return defaultVel;
}

/**
 * tricklist methods
 */

Jumper* Jumper::getPositionConflict(Vector3f position, float* distance)
{
    for( JumperI jumperI = _jumperL.begin(); jumperI != _jumperL.end(); jumperI++ )
    {
        // filter roaming jumpers
        if( (*jumperI)->_phase != jpRoaming ) continue;

        // center of jumper roaming sphere 
        Vector3f jumperPosition = (*jumperI)->_clump->getFrame()->getPos() + Vector3f( 0, jumperRoamingSphereSize, 0 );

        // distance positions
        float value = ( jumperPosition - position ).length();

        // checkout
        if( value < jumperRoamingSphereSize )
        {
            if( distance ) *distance = value;
            return (*jumperI);
        }
    }

    // no conflict
    return NULL;
}

Jumper* Jumper::getPositionConflict(engine::IFrame* airplaneExit)
{
    for( JumperI jumperI = _jumperL.begin(); jumperI != _jumperL.end(); jumperI++ )
    {
        // filter roaming jumpers
        if( (*jumperI)->_phase != jpRoaming ) continue;

        // filter BASE jumpers
        if( (*jumperI)->_airplane == NULL ) continue;

        // checkout
        if( (*jumperI)->_airplaneExit == airplaneExit )
        {
            return (*jumperI);
        }
    }

    // no conflict
    return NULL;
}

/**
 * class behaviour
 */

bool Jumper::getDistanceToImpact(float& inOutDistance)
{
    if( _phase != ::jpFreeFalling ) return false;

    NxRay worldRay;
    worldRay.orig = _phFreeFall->getGlobalPosition();
    worldRay.dir  = _phFreeFall->getLinearVelocity();
    worldRay.dir.normalize();

    float maxDist = inOutDistance;

    NxShape** terrainShapes = _scene->getPhTerrain()->getShapes();

    NxTriangleMeshShape* triMeshShape = terrainShapes[0]->isTriangleMesh();
    assert( triMeshShape );

    NxRaycastHit raycastHit;
    if( triMeshShape->raycast( worldRay, maxDist, NX_RAYCAST_DISTANCE, raycastHit, true ) )
    {
        inOutDistance = raycastHit.distance;
        return true;
    }
    else
    {
        return false;
    }
}

bool Jumper::getDistanceToSurface(float& inOutDistance)
{
    if( _phase != ::jpFlight ) return false;

    NxRay worldRay;
    worldRay.orig = _phFlight->getGlobalPosition();
    worldRay.dir  = NxVec3( 0,-1,0 );
    worldRay.dir.normalize();

    float maxDist = inOutDistance;

    NxShape** terrainShapes = _scene->getPhTerrain()->getShapes();
    NxTriangleMeshShape* triMeshShape = terrainShapes[0]->isTriangleMesh();
    assert( triMeshShape );

    NxRaycastHit raycastHit;
    if( triMeshShape->raycast( worldRay, maxDist, NX_RAYCAST_DISTANCE, raycastHit, true ) )
    {
        inOutDistance = raycastHit.distance;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * unique appearance
 */

void Jumper::setLicensedCharacterAppearance(void)
{
    // select appearance preset    
    setHead( _clump, 4, this );

    // setup Valery Rosov
    #ifdef GAMEPLAY_EDITION_ND    
        _renderCallback.setFace( "valeryrosov", "./res/x/textures/valeryrosov.dds" );
    #endif

    // setup Felix Baumgartner
    #ifdef GAMEPLAY_EDITION_ATARI         
        _renderCallback.setFace( "Felix_Head01_02", "./res/x/textures/Felix_Head01_02.dds" );
        Jumper::getLeftEye( _clump )->setFlags( 0 );
        Jumper::getRightEye( _clump )->setFlags( 0 );
    #endif
}

/**
 * overburden calculation
 */

void Jumper::initOverburdenCalculator(NxVec3& velocity)
{
    _bcStep    = 0;
    _bcPrevVel = velocity;
    _bcBurden  = NxVec3( 0, -9.8f, 0 );
}

/**
 * cat toys support
 */

void Jumper::registerCatToy(CatToy* catToy)
{
    _catToys.push_back( catToy );
}

void Jumper::unregisterCatToy(CatToy* catToy)
{
    for( CatToyI catToyI = _catToys.begin();
                 catToyI != _catToys.end();
                 catToyI++ )
    {
        if( (*catToyI) == catToy )
        {
            _catToys.erase( catToyI );
            break;
        }
    }
}