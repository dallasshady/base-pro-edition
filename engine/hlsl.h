/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description high level shading language compiler
 *
 * @author bad3p
 */

#ifndef HLSL_COMPILER_INCLUDED
#define HLSL_COMPILER_INCLUDED

#include "headers.h"
#include "engine.h"
#include "shader.h"

ID3DXEffect* dxCompileEffect(const char* resourceName, const D3DXMACRO* defines);
ID3DXEffect* dxCompileEffect(const char* effectCode);
    
#endif
