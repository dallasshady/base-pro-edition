/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 */
#pragma once

#define _WIN32_WINDOWS 0x0410
#define WINVER 0x0400

#define NOMINMAX 

#pragma warning(disable:4786)
#include <cassert>
#include <cstdio>
#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <cstdarg>
#include <windows.h>
#include <windef.h>
#include <winuser.h>
#include <windowsx.h>

#include "tinyxml.h"

#include "PxPhysicsAPI.h"
#include "extensions\PxExtensionsAPI.h"
#include "extensions\PxDefaultErrorCallback.h"
#include "extensions\PxDefaultAllocator.h"
#include "extensions\PxDefaultSimulationFilterShader.h"
#include "extensions\PxDefaultCpuDispatcher.h"
#include "extensions\PxShapeExt.h"
#include "extensions\PxSimpleFactory.h"

#include "foundation\PxFoundation.h"

//#pragma comment(lib, "PhysX3_x86.lib")
//#pragma comment(lib, "PhysX3Common_x86.lib")
//#pragma comment(lib, "PhysX3Extensions.lib")
//#pragma comment(lib, "PxTask.lib")

using namespace physx;

//#include "PxPhysics.h"
//#include "foundation/PxErrors.h"
//#include "foundation/PxSimpleTypes.h"
//#include "foundation/PxErrorCallback.h"
//#include "foundation/PxAllocatorCallback.h"


/// statics and globals
static PxFoundation* foundation = NULL;
static PxPhysics* gPhysicsSDK = NULL;
static PxDefaultErrorCallback gDefaultErrorCallback;
static PxDefaultAllocator gDefaultAllocatorCallback;
static PxSimulationFilterShader gDefaultFilterShader = PxDefaultSimulationFilterShader;

static PxDefaultCpuDispatcher*					mCpuDispatcher;
static PxU32									mNbThreads;
#if PX_SUPPORT_GPU_PHYSX
		static PxCudaContextManager*					mCudaContextManager;
#endif

//
//#include "NxP.h"
//#include "NxPhysicsSDK.h"
//#include "NxSceneDesc.h"
//#include "NxScene.h"
//#include "NxBounds3.h"
//#include "NxActor.h"
//#include "NxActorDesc.h"
//#include "NxCooking.h"
//#include "NxTriangleMeshDesc.h"
//#include "NxStream.h"
//#include "NxTriangleMeshShapeDesc.h"
//#include "NxAllocateable.h"
//#include "NxUserOutputStream.h"
//#include "NxBoxShapeDesc.h"
//#include "PxMaterial.h"
//#include "PxMaterialDesc.h"
//#include "NxCapsuleShapeDesc.h"
//#include "NxSphereShapeDesc.h"
//#include "NxUserDebugRenderer.h"
//#include "NxDebugRenderable.h"
//#include "NxRevoluteJoint.h"
//#include "NxRevoluteJointDesc.h"
//#include "NxSphericalJoint.h"
//#include "NxSphericalJointDesc.h"
//#include "NxUserContactReport.h"
//#include "NxDistanceJointDesc.h"
//#include "NxDistanceJoint.h"
//#include "NxPlaneShapeDesc.h"
//#include "NxPlaneShape.h"
//#include "NxRay.h"
//#include "NxSphere.h"