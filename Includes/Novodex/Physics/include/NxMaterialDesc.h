#ifndef NX_PHYSICS_NXMATERIALDESC
#define NX_PHYSICS_NXMATERIALDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

enum NxMaterialFlag
	{
	/**
	Flag to enable anisotropic friction computation. 

	For a pair of actors, anisotropic friction is used only if at least one of the two actors' materials are anisotropic.
	The anisotropic friction parameters for the pair are taken from the material which is more anisotropic (i.e. the difference
	between its two dynamic friction coefficients is greater).

	The anisotropy direction of the chosen material is transformed to world space:

	dirOfAnisotropyWS = shape2world * dirOfAnisotropy

	Next, the directions of anisotropy in one or more contact planes (i.e. orthogonal to the contact normal) have to be determined. 
	The two directions are:

	uAxis = (dirOfAnisotropyWS ^ contactNormal).normalize()
	vAxis = contactNormal ^ uAxis

	This way [uAxis, contactNormal, vAxis] forms a basis.

	It may happen, however, that (dirOfAnisotropyWS | contactNormal).magnitude() == 1 
	and then (dirOfAnisotropyWS ^ contactNormal) has zero length. This happens when 
	the contactNormal is coincident to the direction of anisotropy. In this case we perform isotropic friction. 
	*/
	NX_MF_ANISOTROPIC = 1 << 0,

	/**
	If a material with this flag set is involved in the contact, the contact constraints generated behave like a unilateral spring.
	The user must assign the programData member to point to a NxSpringDesc object.
	*/
	NX_MF_SPRING_CONTACT = 1 << 2,			// AM: replace this later with NX_MF_PROGRAMMABLE! : The contact and friction effects become programmable if a material with this flag set is involved in the contact.

	//Note: Bits 16-31 are reserved for internal use!
	};

/**
Flag that determines the combine mode. When two actors come in contact with eachother, they each have
materials with various coefficients, but we only need a single set of coefficients for the pair.

Physics doesn't have any inherent combinations because the coefficients are determined empirically on a case by case
basis. However, simulating this with a pairwise lookup table is often impractical.

For this reason the following combine behaviors are available:

NX_CM_AVERAGE = 2,
NX_CM_MIN = 0,
NX_CM_MULTIPLY = 1,
NX_CM_MAX = 3,

The effective combine mode for the pair is max(material0.combineMode, material1.combineMode).
*/
enum NxCombineMode
	{
	NX_CM_AVERAGE = 0,
	NX_CM_MIN = 1,
	NX_CM_MULTIPLY = 2,
	NX_CM_MAX = 3,
	NX_CM_N_VALUES = 4,	//this a sentinel to denote the number of possible values. We assert that the variable's value is smaller than this.
	NX_CM_PAD_32 = 0xffffffff 
	};


/**
Descriptor of NxMaterial.
*/
class NxMaterialDesc
	{
	public:
	/**
	coefficient of dynamic friction -- should be in [0, 1] and also be less or equal to staticFriction.
	if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)
	*/
	NxReal	dynamicFriction;
	/**
	coefficient of static friction -- should be in [0, +inf]
	if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)
	*/
	NxReal	staticFriction;

	/**
	coefficient of restitution --  0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce.
	*/
	NxReal	restitution;

	/**
	anisotropic dynamic friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags & NX_MF_ANISOTROPIC is set.
	*/
	NxReal dynamicFrictionV;
	/**
	anisotropic static  friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags & NX_MF_ANISOTROPIC is set.
	*/
	NxReal staticFrictionV;
	/**
	shape space direction (unit vector) of anisotropy.
	This is only used if flags & NX_MF_ANISOTROPIC is set.
	*/
	NxVec3 dirOfAnisotropy;

	/**
	Flags, a combination of the bits defined by the enum ::NxMaterialFlag . 
	*/
	NxU32 flags;

	/**
	Friction combine mode. See the enum ::NxCombineMode .
	*/
	NxCombineMode frictionCombineMode;

	/**
	Restitution combine mode. See the enum ::NxCombineMode .
	*/
	NxCombineMode restitutionCombineMode;

	/**
	additional SDK side user defined data for material programs.
	programDataSize bytes will be copied into the SDK from this location.
	*/
	void * programData;

	/**
	size of the programData buffer in bytes.
	*/
	NxU32  programDataSize;

	/**
	constructor sets to default.
	*/
	NX_INLINE NxMaterialDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxMaterialDesc::NxMaterialDesc()
	{
	setToDefault();
	}

NX_INLINE	void NxMaterialDesc::setToDefault()
	{
	dynamicFriction	= 0.0f;
	staticFriction	= 0.0f;
	restitution		= 0.0f;


	dynamicFrictionV= 0.0f;
	staticFrictionV = 0.0f;

	dirOfAnisotropy.set(1,0,0);
	flags = 0;
	frictionCombineMode = NX_CM_AVERAGE;
	restitutionCombineMode = NX_CM_AVERAGE;
	programData = 0;
	programDataSize = 0;
	}

NX_INLINE	bool NxMaterialDesc::isValid()	const
	{
	if(dynamicFriction < 0.0f || dynamicFriction > 1.0f) 
		return false;
	if(staticFriction < 0.0f) 
		return false;
	if(restitution < 0.0f || restitution > 1.0f) 
		return false;

	if (flags & NX_MF_ANISOTROPIC)
		{
		NxReal ad = dirOfAnisotropy.magnitudeSquared();
		if (ad < 0.98f || ad > 1.03f)
			return false;
		if(dynamicFrictionV < 0.0f || dynamicFrictionV > 1.0f) 
			return false;
		if(staticFrictionV < 0.0f) 
			return false;
		}
	/*
	if (flags & NX_MF_MOVING_SURFACE)
		{
		NxReal md = dirOfMotion.magnitudeSquared();
		if (md < 0.98f || md > 1.03f)
			return false;
		}
	*/
	if (frictionCombineMode >= NX_CM_N_VALUES)
		return false;
	if (restitutionCombineMode >= NX_CM_N_VALUES)
		return false;
	if ((programData && programDataSize == 0) || (!programData && programDataSize))
		return false;

	return true;
	}

#endif
