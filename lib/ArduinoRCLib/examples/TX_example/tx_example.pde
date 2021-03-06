/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** tx_example.pde
** Complete transmitter example.
** This example demonstrates how to make a 6 channel CCPM heli transmitter
**
** Author: Daniel van den Ouden
** Project: ArduinoRCLib
** Website: http://sourceforge.net/p/arduinorclib/
** -------------------------------------------------------------------------*/

#include <AIPin.h>
#include <Channel.h>
#include <Curve.h>
#include <DIPin.h>
#include <DualRates.h>
#include <Expo.h>
#include <Gyro.h>
#include <InputOutputPipe.h>
#include <InputToInputMix.h>
#include <PPMOut.h>
#include <Swashplate.h>
#include <ThrottleHold.h>
#include <Timer1.h>
#include <util.h>

enum
{
	ChannelCount = 6
};

// input related variables
rc::AIPin g_aPins[4] = 
{
	rc::AIPin(A0, rc::Input_AIL), // we have to specify an input pint
	rc::AIPin(A1, rc::Input_ELE), // and we can optionally specify an index in the centralized
	rc::AIPin(A2, rc::Input_THR), // input buffer where results should be written to
	rc::AIPin(A3, rc::Input_RUD)
};
rc::DIPin g_dPins[2] = { rc::DIPin(3), rc::DIPin(4) };

// Expo/DR, we use one expo and one dr per control and per flightmode
rc::Expo g_ailExpo[2] = {rc::Expo(-30, rc::Input_AIL), rc::Expo(-10, rc::Input_AIL)}; // also specify what index of the input
rc::Expo g_eleExpo[2] = {rc::Expo(-30, rc::Input_ELE), rc::Expo(-10, rc::Input_ELE)}; // buffer the expo should work on (optionally)
rc::Expo g_rudExpo[2] = {rc::Expo(-20, rc::Input_RUD), rc::Expo(0,   rc::Input_RUD)};

rc::DualRates g_ailDR[2] = {rc::DualRates(80, rc::Input_AIL), rc::DualRates(100, rc::Input_AIL)}; // also specify what index of the input
rc::DualRates g_eleDR[2] = {rc::DualRates(80, rc::Input_ELE), rc::DualRates(100, rc::Input_ELE)}; // buffer the dual rates
rc::DualRates g_rudDR[2] = {rc::DualRates(80, rc::Input_RUD), rc::DualRates(100, rc::Input_RUD)}; // should work on (optionally)

// pitch and throttle curves, supply source and destination (both optional)
rc::Curve g_pitCurve[2] = // we'll use throttle as source and pitch as destination
{
	rc::Curve(rc::Curve::DefaultCurve_HalfLinear, rc::Input_THR, rc::Input_PIT), // normal curve, only positive pitch
	rc::Curve(rc::Curve::DefaultCurve_Linear,     rc::Input_THR, rc::Input_PIT)  // idle up curve
};
rc::Curve g_thrCurve[2] = // we'll use throttle as both source and destination
{
	rc::Curve(rc::Curve::DefaultCurve_Linear, rc::Input_THR, rc::Input_THR), // normal curve
	rc::Curve(rc::Curve::DefaultCurve_V,      rc::Input_THR, rc::Input_THR)  // idle up curve, V shaped
};

// throttle hold
rc::Curve        g_pitCurveHold = rc::Curve(rc::Curve::DefaultCurve_Linear, rc::Input_THR, rc::Input_PIT);
rc::ThrottleHold g_throttleHold; // Default throttle level is -256, default in/output is THR

// channel transformations, we can specify a source for the channel
// This acts like "glue", we can tell a channel where to fetch its input from
// Various functions write their output to predefined or user settable locations
// A channel can fetch its input from these locations
// Multiple channels may use the same input
rc::Channel g_channels[ChannelCount] =
{
	rc::Channel(rc::Output_AIL1),
	rc::Channel(rc::Output_ELE1),
	rc::Channel(rc::Output_THR1),
	rc::Channel(rc::Output_RUD1),
	rc::Channel(rc::Output_GYR1),
	rc::Channel(rc::Output_PIT)
};

// swashplate
rc::Swashplate g_swash;

// gyro, we have two sets of settings, for each flightmode one
rc::Gyro g_gyro[2] =
{
	rc::Gyro(rc::Output_GYR1), // Both need to write to the same output
	rc::Gyro(rc::Output_GYR1)  // We only use the one of the active flightmode
};

// Channel values in microseconds
uint16_t g_channelValues[ChannelCount] = {0};

// PPM related variables
uint8_t    g_PPMWork[PPMOUT_WORK_SIZE(ChannelCount)];
rc::PPMOut g_PPMOut(ChannelCount, g_channelValues, g_PPMWork, ChannelCount);

// Set up pipes for direct input to output copying
rc::InputOutputPipe g_throttle(rc::Input_THR, rc::Output_THR1);
rc::InputOutputPipe g_rudder(rc::Input_RUD, rc::Output_RUD1);

// Swash to throttle mixing
// we need to use abs, this will make sure that any negative input from aileron or elevator will
// be treated as positive. Otherwise we would get an decrease in throttle when we move to one side.
rc::InputToInputMix g_ailToThr(5, true, rc::Input_AIL, rc::Input_THR); // 5%, use abs, from aileron, to throttle
rc::InputToInputMix g_eleToThr(5, true, rc::Input_ELE, rc::Input_THR); // 5%, use abs, from elevator, to throttle


void setup()
{
	// Initialize timer1
	rc::Timer1::init();
	
	// set calibration values, these depend on hardware configurations
	g_aPins[0].setCalibration( 90, 515,  930); // Right horizontal, aileron
	g_aPins[1].setCalibration(127, 544,  961); // Right vertical, elevator
	g_aPins[2].setCalibration( 27, 435,  834); // Left vertical, throttle
	g_aPins[3].setCalibration( 67, 502,  924); // Left horizontal, rudder
	g_aPins[3].setReverse(true);               // potentiometer mounted upside down
	
	// swashplate settings
	g_swash.setType(rc::Swashplate::Type_HR3);
	g_swash.setAilMix(50);
	g_swash.setEleMix(50);
	g_swash.setPitMix(50);
	
	// gyro settings
	g_gyro[0].setType(rc::Gyro::Type_AVCS);
	g_gyro[1].setType(rc::Gyro::Type_AVCS);
	
	g_gyro[0].setMode(rc::Gyro::Mode_Normal);
	g_gyro[1].setMode(rc::Gyro::Mode_AVCS);
	
	g_gyro[0] = 50; // set gyro gain to 50%
	g_gyro[1] = 75; // set gyro gain to 75%
	
	// set up normalized -> microseconds conversion
	rc::setCenter(1520); // servo center point
	rc::setTravel(600);  // max servo travel from center point
	// our output signal will lie between 920 and 2120 microseconds (1520 +/- 600)
	
	// fill channel values buffer with sane values, all centered
	g_channelValues[0] = rc::normalizedToMicros(0);
	g_channelValues[1] = rc::normalizedToMicros(0);
	g_channelValues[2] = rc::normalizedToMicros(-256); // Throttle channel, MUST BE AT 0 THROTTLE!
	g_channelValues[3] = rc::normalizedToMicros(0);
	g_channelValues[4] = rc::normalizedToMicros(0);
	g_channelValues[5] = rc::normalizedToMicros(0);
	
	// set up PPM
	g_PPMOut.setPulseLength(448);   // default pulse length used by Esky hardware
	g_PPMOut.setPauseLength(10448); // default pause length used by Esky hardware
	g_PPMOut.start(9); // use pin 9, which is preferred as it's faster
}


void loop()
{
	// read digital values
	uint8_t flightmode   = g_dPins[0].read() ? 1 : 0;
	bool    throttleHold = g_dPins[1].read();
		
	// read analog values, these write to the input system (AIL, ELE, THR and RUD)
	g_aPins[0].read(); // aileron
	g_aPins[1].read(); // elevator
	g_aPins[2].read(); // throttle/pitch
	g_aPins[3].read(); // rudder
	
	// apply expo and dual rates to input, these read from and write to input system
	g_ailExpo[flightmode].apply();
	g_ailDR[flightmode].apply();
	
	g_eleExpo[flightmode].apply();
	g_eleDR[flightmode].apply();
	
	g_rudExpo[flightmode].apply();
	g_rudDR[flightmode].apply();
	
	// apply pitch and throttle curves and handle throttle hold
	// A quick but important note here
	// Because the throttle curve overwrites the throttle input value
	// and the pitch curve also uses the throttle input as source
	// we need to apply the pitch curve BEFORE modifying the throttle
	if (throttleHold)
	{
		g_pitCurveHold.apply();     // reads from THR, writes to PIT
		g_throttleHold.apply(true); // reads from THR, writes to THR
	}
	else
	{
		g_pitCurve[flightmode].apply(); // reads from THR, writes to PIT
		g_thrCurve[flightmode].apply(); // reads from THR, writes to THR
	}
	
	// apply swash to throttle mixing
	g_ailToThr.apply();
	g_eleToThr.apply();
	
	// END OF INPUT HANDLING
	// up to here we've only dealt with filling the input system and applying transformations to it
	// now we will start using the input values to control output
	
	// apply swashplate mixing,
	// will read from input system (AIL, ELE and PIT)
	// will write to output system (AIL1, ELE1 and PIT (and ELE2 for four servo setup))
	g_swash.apply();
	
	// handle gyro, will write to output system (GYR1; see setup() )
	g_gyro[flightmode].apply();
	
	// apply rudder and throttle mapping
	// these take their input from input system (RUD and THR) and write to
	// output system (RUD1 and THR1); see their declaration.
	g_rudder.apply();
	g_throttle.apply();
	
	// END OF OUTPUT HANDLING
	// Now we've filled all the parts of the output system we need (AIL1, ELE1, THR1, RUD1, GYR1 and PIT1)
	// now we can start using the output to generate a signal or data packet
	// Well, almost. Channel does perform some transformations, but these aren't written back into the output system
	// since more than one channel may use the same output as source.
	
	// perform channel transformations and set channel values
	for (uint8_t i = 0; i < ChannelCount; ++i)
	{
		g_channelValues[i] = rc::normalizedToMicros(g_channels[i].apply());
	}
	
	// Tell PPMOut that new values are ready
	g_PPMOut.update();
}
