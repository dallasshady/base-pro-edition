
#include "headers.h"
#include "jumper.h"
#include "gameplay.h"

float max(float a, float b) {
	return a > b? a : b;
}
float max(float a, float b, float c) {
	if (a > b && a > c) return a;
	if (b > a && b > c) return b; else return c;
	if (c > b && c > a) return c;
}
void SpinalCord::mapActionChannels(void)
{
	left      = max(Gameplay::iGameplay->getActionChannel( iaLeft )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaLeftJoy )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getAmplitude());
    right     = max(Gameplay::iGameplay->getActionChannel( iaRight )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaRightJoy )->getAmplitude(), Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getAmplitude());
    up        = Gameplay::iGameplay->getActionChannel( iaForward )->getAmplitude();
    down      = Gameplay::iGameplay->getActionChannel( iaBackward )->getAmplitude();
    leftWarp  = Gameplay::iGameplay->getActionChannel( iaLeftWarp )->getAmplitude();
    rightWarp = Gameplay::iGameplay->getActionChannel( iaRightWarp )->getAmplitude();
    leftRearRiser  = Gameplay::iGameplay->getActionChannel( iaLeftRearRiser )->getAmplitude();
    rightRearRiser = Gameplay::iGameplay->getActionChannel( iaRightRearRiser )->getAmplitude();
    phase     = Gameplay::iGameplay->getActionChannel( iaPhase )->getTrigger();
    modifier  = Gameplay::iGameplay->getActionChannel( iaModifier )->getAmplitude() > 0.0f;
    wlo       = Gameplay::iGameplay->getActionChannel( iaWLO )->getTrigger();
    hook      = Gameplay::iGameplay->getActionChannel( iaHook )->getTrigger();
	
	cutAway   = Gameplay::iGameplay->getActionChannel( iaCutAway )->getTrigger() || Gameplay::iGameplay->getActionChannel( iaCutAwayJoy )->getTrigger();
	pullReserve = Gameplay::iGameplay->getActionChannel( iaPullReserve )->getTrigger();

	leftReserve = Gameplay::iGameplay->getActionChannel( iaReserveLeft )->getAmplitude();
	rightReserve = Gameplay::iGameplay->getActionChannel( iaReserveRight )->getAmplitude();
	leftReserveWarp  = Gameplay::iGameplay->getActionChannel( iaReserveLeftWarp )->getAmplitude();
    rightReserveWarp = Gameplay::iGameplay->getActionChannel( iaReserveRightWarp )->getAmplitude();
	leftReserveRearRiser  = Gameplay::iGameplay->getActionChannel( iaReserveLeftRearRiser )->getAmplitude();
    rightReserveRearRiser = Gameplay::iGameplay->getActionChannel( iaReserveRightRearRiser )->getAmplitude();
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
    modifier  = false;
    wlo       = false;
    hook      = false;
}