#include "pose.h"

/**
 * class implementation
 */

// this means that even though Pose::jointnames will get declared in multiple object files,
// the linker will only pick up one
__declspec(selectany)
const char *Pose::jointnames[] = {"Head",			// head
								  "Poyasnitsa",		// torso
								  "joint32",		// larm
								  "joint8",			// rarm
								  "Hip_Joint",		// lleg
								  "Hip_Joint1",		// rleg
								  "Knee_Joint",		// lknee
								  "Knee_Joint1",	// rknee
								  "RKluchitsa1",	// lshoulder
								  "RKluchitsa"		// rshoulder
								  };

Pose::Pose() {
	this->A = 0;
	this->L = 0;

	for (int i = 0; i < POSEANIM_JOINTS_ALL; ++i) {
		if (i < POSEANIM_JOINTS) {
			this->airfoils[i] = NxVec3(0,0,0);
			this->airfoils_area[i] = 0;
		}
		this->jointaxis[i] = NxVec3(0,0,0);
		this->jointangles[i] = 0;
	}

	this->restPose = NULL;
	this->targetPose = NULL;
	this->target_rate = 0.0f;
	this->target_prog = 0.0f;

	this->torque = NxVec3(0,0,0);
	this->force = NxVec3(0,0,0);
	this->reaction = this->reaction_now = 0.0f;

	this->animTrack = 0;
	this->animTrackWeight = 1.0f;
}

void Pose::resetPose(double dt) {
	// morph instead of reset
	if (targetPose != NULL) {
		float prog = this->target_prog;
		float antiprog = 1.0f - prog;

		this->L_now = (this->restPose->L * antiprog) + (this->targetPose->L * prog);
		this->reaction_now = (this->restPose->reaction * antiprog) + (this->targetPose->reaction * prog);
		this->force = (this->restPose->force * antiprog) + (this->targetPose->force * prog);
		this->torque = (this->restPose->torque * antiprog) + (this->targetPose->torque * prog);

		for (int i = 0; i < POSEANIM_JOINTS_ALL; ++i) {
			if (i < POSEANIM_JOINTS) {
				this->airfoils_now[i] = (this->restPose->airfoils[i] * antiprog) + (this->targetPose->airfoils[i] * prog);
				this->airfoils_area_now[i] = (this->restPose->airfoils_area_now[i] * antiprog) + (this->targetPose->airfoils_area_now[i] * prog);
			}
			this->jointaxis[i] = (this->restPose->jointaxis[i] * antiprog) + (this->targetPose->jointaxis[i] * prog);
			this->jointangles[i] = (this->restPose->jointangles[i] * antiprog) + (this->targetPose->jointangles[i] * prog);
		}

		this->target_prog += this->target_rate * (float)dt;

	// reset 
	} else {
		this->L_now = this->L;
		
		for (int i = 0; i < POSEANIM_JOINTS; ++i) {
			this->airfoils_now[i] = this->airfoils[i];
			this->airfoils_area_now[i] = this->airfoils_area[i];
		}
		this->force = NxVec3(0,0,0);
		this->torque = NxVec3(0,0,0);
		this->reaction_now = this->reaction;
	}
}

void Pose::morphToPose(Pose *targetPose, float rate, bool force) {
	// bail if already morphing, trying to morph to NULL or trying to morph at zero rate
	if ((!force && this->targetPose != NULL) || targetPose == NULL || rate <= 0.0f) return;

	this->targetPose = targetPose;
	this->restPose = this;
	this->target_prog = 0.0f;
	this->target_rate = rate;
}
void Pose::controlPose(double dt) {
	this->resetPose(dt);

	// no control during morph
	if (this->target_prog > 0.0f) {
		//this->targetPose->controlPose(dt);
		//return;
	}

	// traverse anims
	PoseAnim *d = &this->anims;
	while (d != NULL) {
		// blocks
		if (d->only_if_not_zero != NULL) {
			if (*d->only_if_not_zero == 0.0f) {
				d = d->next;
				continue;
			}
		}
		if (d->only_if_zero != NULL) {
			if (*d->only_if_zero != 0.0f) {
				d = d->next;
				continue;
			}
		}

		// store previous animation value
		d->prev_value = d->value;

		// calculate animation value
		float value = 0.0f;
		int valueidx = 0;

		for (int i = 0; i < d->var_c; ++i) {
			// max value
			if (d->var_op == POSEANIM_OP_MAX) {
				if (i == valueidx) continue;
				if (*d->var[i] > *d->var[valueidx]) {
					valueidx = i;
				}
			// min value
			} else if (d->var_op == POSEANIM_OP_MIN) {
				if (i == valueidx) continue;
				if (*d->var[i] < *d->var[valueidx]) {
					valueidx = i;
				}
			// sum value OR average value
			} else if (d->var_op == POSEANIM_OP_SUM || d->var_op == POSEANIM_OP_AVG) {
				value += *d->var[i];
			// subtraction
			} else if (d->var_op == POSEANIM_OP_SUB) {
				if (i == 0) {
					value = *d->var[0];
				} else {
					value -= *d->var[i];
				}
			}
		}
		
		// get final value
		if (d->var_c > 0) {
			if (d->var_op == POSEANIM_OP_MAX || d->var_op == POSEANIM_OP_MIN) {
				value = *d->var[valueidx];
			} else if (d->var_op == POSEANIM_OP_AVG) {
				value /= (float)d->var_c;
			}
		}
		
		// control!
		d->value = value;
		for (int i = 0; i < POSEANIM_JOINTS; ++i) {
			// move airfoils
			if (!d->airfoils[i].isZero()) {
				this->airfoils_now[i] += d->airfoils[i] * value;
			}
			// set areas
			if (d->airfoils_area[i] != 0.0f) {
				this->airfoils_area_now[i] += d->airfoils_area[i] * value;
			}
		}

		// set lift
		if (d->dL != 0.0f) {
			this->L_now = this->L + d->dL * value;
		}
		// set torque
		if (!d->torque.isZero()) {
			this->torque += d->torque * value;
		}
		// set reaction
		if (d->dReaction != 0.0f) {
			this->reaction_now = this->reaction + d->dReaction * value;
		}
		// set force
		if (!d->force.isZero()) {
			this->force += d->force * value;
		}
		d = d->next;
	}



}

bool Pose::animBodyPart(const char *frame_title) {
	return true;
}

PoseAnim::PoseAnim() {
	this->dA = 0;
	this->dL = 0;
	this->dReaction = 0;
	this->var_op = 0;
	this->var_c = 0;

	for (int i = 0; i < POSEANIM_JOINTS_ALL; ++i) {
		if (i < POSEANIM_JOINTS) {
			this->airfoils[i] = NxVec3(0,0,0);
			this->airfoils_area[i] = 0;
		}
		this->jointaxis[i] = NxVec3(0,0,0);
		this->jointangles[i] = 0;
	}

	this->next = NULL;
	this->targetPose = NULL;
	this->value = this->prev_value = 0.0f;
	this->torque = NxVec3(0,0,0);
	this->force = NxVec3(0,0,0);

	this->only_if_not_zero = NULL;
	this->only_if_zero = NULL;

	this->animTrack = -1;		// -1 is no change
	this->animTrackWeight = 1.0f;
}
PoseAnim::~PoseAnim() {
	this->var_c = 0;
}