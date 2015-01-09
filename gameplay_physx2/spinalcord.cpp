
#include "headers.h"
#include "jumper.h"
#include "gameplay.h"

float max(float a, float b) {
	return a > b? a : b;
}
float max(float a, float b, float c) {
	if (a > b && a > c) return a;
	if (b > a && b > c) return b; else return c;
}
void SpinalCord::mapActionChannels(void)
{
	// button values

	left      = max(Gameplay::iGameplay->getActionChannel( iaLeft )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaLeftJoy )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getAmplitude());
    right     = max(Gameplay::iGameplay->getActionChannel( iaRight )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaRightJoy )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getAmplitude());
    up        = Gameplay::iGameplay->getActionChannel( iaForward )->getAmplitude();
    down      = Gameplay::iGameplay->getActionChannel( iaBackward )->getAmplitude();
    leftWarp  = Gameplay::iGameplay->getActionChannel( iaLeftWarp )->getAmplitude();
    rightWarp = Gameplay::iGameplay->getActionChannel( iaRightWarp )->getAmplitude();
    leftRearRiser  = Gameplay::iGameplay->getActionChannel( iaLeftRearRiser )->getAmplitude();
    rightRearRiser = Gameplay::iGameplay->getActionChannel( iaRightRearRiser )->getAmplitude();
	rearBrake = Gameplay::iGameplay->getActionChannel( iaRearBrake )->getAmplitude();
    phase     = Gameplay::iGameplay->getActionChannel( iaPhase )->getTrigger();
    modifier  = Gameplay::iGameplay->getActionChannel( iaModifier )->getAmplitude();
    wlo       = Gameplay::iGameplay->getActionChannel( iaWLO )->getAmplitude();
    hook      = Gameplay::iGameplay->getActionChannel( iaHook )->getAmplitude();

	cutAway   = Gameplay::iGameplay->getActionChannel( iaCutAway )->getTrigger() || Gameplay::iGameplay->getActionChannel( iaCutAwayJoy )->getTrigger();
	pullReserve = Gameplay::iGameplay->getActionChannel( iaPullReserve )->getTrigger();

	leftReserve = Gameplay::iGameplay->getActionChannel( iaReserveLeft )->getAmplitude();
	rightReserve = Gameplay::iGameplay->getActionChannel( iaReserveRight )->getAmplitude();
	leftReserveWarp  = Gameplay::iGameplay->getActionChannel( iaReserveLeftWarp )->getAmplitude();
    rightReserveWarp = Gameplay::iGameplay->getActionChannel( iaReserveRightWarp )->getAmplitude();
	leftReserveRearRiser  = Gameplay::iGameplay->getActionChannel( iaReserveLeftRearRiser )->getAmplitude();
    rightReserveRearRiser = Gameplay::iGameplay->getActionChannel( iaReserveRightRearRiser )->getAmplitude();

	// button triggers
	trigger_left      = Gameplay::iGameplay->getActionChannel( iaLeft )->getTrigger() || Gameplay::iGameplay->getActionChannel( iaLeftJoy )->getTrigger() || Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getTrigger();
    trigger_right     = Gameplay::iGameplay->getActionChannel( iaRight )->getTrigger() || Gameplay::iGameplay->getActionChannel( iaRightJoy )->getTrigger() || Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getTrigger();
    trigger_up        = Gameplay::iGameplay->getActionChannel( iaForward )->getTrigger();
    trigger_down      = Gameplay::iGameplay->getActionChannel( iaBackward )->getTrigger();
    trigger_leftWarp  = Gameplay::iGameplay->getActionChannel( iaLeftWarp )->getTrigger();
    trigger_rightWarp = Gameplay::iGameplay->getActionChannel( iaRightWarp )->getTrigger();
    trigger_leftRearRiser  = Gameplay::iGameplay->getActionChannel( iaLeftRearRiser )->getTrigger();
    trigger_rightRearRiser = Gameplay::iGameplay->getActionChannel( iaRightRearRiser )->getTrigger();
	trigger_rearBrake = Gameplay::iGameplay->getActionChannel( iaRearBrake )->getTrigger();
    trigger_phase     = Gameplay::iGameplay->getActionChannel( iaPhase )->getTrigger();
    trigger_modifier  = Gameplay::iGameplay->getActionChannel( iaModifier )->getTrigger();
    trigger_wlo       = Gameplay::iGameplay->getActionChannel( iaWLO )->getTrigger();
    trigger_hook      = Gameplay::iGameplay->getActionChannel( iaHook )->getTrigger();

	trigger_cutAway   = Gameplay::iGameplay->getActionChannel( iaCutAway )->getTrigger() || Gameplay::iGameplay->getActionChannel( iaCutAwayJoy )->getTrigger();
	trigger_pullReserve = Gameplay::iGameplay->getActionChannel( iaPullReserve )->getTrigger();

	trigger_leftReserve = Gameplay::iGameplay->getActionChannel( iaReserveLeft )->getTrigger();
	trigger_rightReserve = Gameplay::iGameplay->getActionChannel( iaReserveRight )->getTrigger();
	trigger_leftReserveWarp  = Gameplay::iGameplay->getActionChannel( iaReserveLeftWarp )->getTrigger();
    trigger_rightReserveWarp = Gameplay::iGameplay->getActionChannel( iaReserveRightWarp )->getTrigger();
	trigger_leftReserveRearRiser  = Gameplay::iGameplay->getActionChannel( iaReserveLeftRearRiser )->getTrigger();
    trigger_rightReserveRearRiser = Gameplay::iGameplay->getActionChannel( iaReserveRightRearRiser )->getTrigger();
}

void SpinalCord::reset(void)
{
    left      = 0.0f;
    right     = 0.0f;
    up        = 0.0f;
    down      = 0.0f;
    leftWarp  = 0.0f;
    rightWarp = 0.0f;
    phase     = false;
    modifier  = 0.0f;
    wlo       = false;
    hook      = false;
}