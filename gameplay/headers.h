/**
 * This source code is a part of Metathrone game project. 
 * (c) Perfect Play 2003.
 */

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

#include "NxP.h"
#include "NxPhysicsSDK.h"
#include "NxSceneDesc.h"
#include "NxScene.h"
#include "NxBounds3.h"
#include "NxActor.h"
#include "NxActorDesc.h"
#include "NxCooking.h"
#include "NxTriangleMeshDesc.h"
#include "NxStream.h"
#include "NxTriangleMeshShapeDesc.h"
#include "NxAllocateable.h"
#include "NxUserOutputStream.h"
#include "NxBoxShapeDesc.h"
#include "NxMaterial.h"
#include "NxMaterialDesc.h"
#include "NxCapsuleShapeDesc.h"
#include "NxSphereShapeDesc.h"
#include "NxUserDebugRenderer.h"
#include "NxDebugRenderable.h"
#include "NxRevoluteJoint.h"
#include "NxRevoluteJointDesc.h"
#include "NxSphericalJoint.h"
#include "NxSphericalJointDesc.h"
#include "NxUserContactReport.h"
#include "NxDistanceJointDesc.h"
#include "NxDistanceJoint.h"
#include "NxPlaneShapeDesc.h"
#include "NxPlaneShape.h"
#include "NxRay.h"
#include "NxSphere.h"