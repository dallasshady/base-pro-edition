#ifndef NX_PHYSICS_NXMATERIAL
#define NX_PHYSICS_NXMATERIAL
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "NxMaterialDesc.h"

class NxScene;

/**
Class for describing a shape's surface properties.
*/
class NxMaterial
	{
	protected:
	NX_INLINE					NxMaterial() : userData(NULL)		{}
	virtual						~NxMaterial()	{}

	public:
	/**
	Materials are associated with mesh faces and shapes using 16 bit identifiers called a NxMaterialIndex rather
	than pointers.  The ID of the material can be retrieved using this function.  
	If you release a material while its material ID is still in use by shapes or meshes, the material usage
	of these objects becomes undefined as the material index gets recycled.
	*/
	virtual		NxMaterialIndex getMaterialIndex() = 0;

	/**
	Loads the entire state of the material from a descriptor with a single call.
	*/
	virtual		void			loadFromDesc(const NxMaterialDesc&) = 0;

	/**
	Saves the state of the material into a descriptor.
	Does not fill out the programData and programDataSize fields, this must be queried separately using getProgramData().
	*/
	virtual		void			saveToDesc(NxMaterialDesc&) const	= 0;

	/**
	retrieves owner scene
	*/
	virtual		NxScene&		getScene() = 0;

	/**
	Sets the coefficient of dynamic friction -- should be in [0, 1] and also be less or equal to staticFriction.
	if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)
	*/
	virtual		void			setDynamicFriction(NxReal) = 0;

	/**
	Retrieves the DynamicFriction value.
	*/
	virtual		NxReal			getDynamicFriction() const = 0;

	/**
	Sets the coefficient of static friction -- should be in [0, +inf]
	if flags & NX_MF_ANISOTROPIC is set, then this value is used for the primary direction of anisotropy (U axis)
	*/
	virtual		void			setStaticFriction(NxReal) = 0;

	/**
	Retrieves the StaticFriction value.
	*/
	virtual		NxReal			getStaticFriction() const = 0;

	/**
	Sets the coefficient of restitution --  0 makes the object bounce as little as possible, higher values up to 1.0 result in more bounce.
	*/
	virtual		void			setRestitution(NxReal) = 0;

	/**
	Retrieves the Restitution value.
	*/
	virtual		NxReal			getRestitution() const = 0;

	/**
	Sets the anisotropic dynamic friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags & NX_MF_ANISOTROPIC is set.
	*/
	virtual		void			setDynamicFrictionV(NxReal) = 0;

	/**
	Retrieves the DynamicFrictionV value.
	*/
	virtual		NxReal			getDynamicFrictionV() const = 0;

	/**
	Sets the anisotropic static  friction coefficient for along the secondary (V) axis of anisotropy. 
	This is only used if flags & NX_MF_ANISOTROPIC is set.
	*/
	virtual		void			setStaticFrictionV(NxReal) = 0;

	/**
	Retrieves the StaticFrictionV value.
	*/
	virtual		NxReal			getStaticFrictionV() const = 0;

	/**
	Sets the shape space direction (unit vector) of anisotropy.
	This is only used if flags & NX_MF_ANISOTROPIC is set.
	*/
	virtual		void			setDirOfAnisotropy(const NxVec3 &) = 0;

	/**
	Retrieves the DirOfAnisotropy value.
	*/
	virtual		NxVec3			getDirOfAnisotropy() const = 0;

	/**
	Sets the flags, a combination of the bits defined by the enum ::NxMaterialFlag . 
	*/
	virtual		void			setFlags(NxU32) = 0;

	/**
	Retrieves the flags.
	*/
	virtual		NxU32			getFlags() const = 0;

	/**
	Sets the friction combine mode. See the enum ::NxCombineMode .
	*/
	virtual		void			setFrictionCombineMode(NxCombineMode) = 0;

	/**
	Retrieves the friction combine mode.
	*/
	virtual		NxCombineMode	getFrictionCombineMode() const = 0;

	/**
	Sets the restitution combine mode. See the enum ::NxCombineMode .
	*/
	virtual		void			setRestitutionCombineMode(NxCombineMode) = 0;

	/**
	Retrieves the restitution combine mode.
	*/
	virtual		NxCombineMode	getRestitutionCombineMode() const = 0;

	/**
	Sets additional SDK side user defined data for material programs.  size is the number of bytes to copy.
	*/
	virtual		void			setProgramData(const void * data, NxU32 size) = 0;

	/**
	Retrieves the size of the contact data.
	*/
	virtual		NxU32			getProgramDataSize() const = 0;
	/**
	Retrieves the program data into a destination buffer.  Passing destinationBufferSize < getProgramDataSize() will
	result in an error.
	*/
	virtual		void			getProgramData(void * destinationBuffer, NxU32 destinationBufferSize) const = 0;

	//public variables:
				void*			userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
	};


//typedef NxMaterial * NxMaterialIndex;   //legacy support (problematic because the size used to be 2 bytes)

#endif
