/**
 * This source code is a part of D3 game project. 
 * (c) Digital Dimension Development, 2004-2005
 */

#define _WIN32_WINDOWS 0x0410
#define WINVER 0x0400

#pragma warning(disable:4786)

// evade windows min/max macroses
#define NOMINMAX

// common includes
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
#include <tchar.h>

// directx includes
#include "d3d9.h"
#include "d3d9types.h"
#include "d3dx9effect.h"
#include "d3dx9shader.h"
#include "ddraw.h"
#include "d3dx9math.h"

// XML includes
#include "tinyxml.h"

// debug issue
#include "NxDebugRenderable.h"