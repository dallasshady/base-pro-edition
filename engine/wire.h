/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description wire renderer
 *
 * @author bad3p
 */

#ifndef WIRE_ROUTINE_INCLUDED
#define WIRE_ROUTINE_INCLUDED

#include "headers.h"
#include "engine.h"

#define MAX_LINES      4*1024
#define SPHERE_DETAILS 48
#define SPHERE_BELTS   3

void dxRenderLine(Line* line, const Color* color, Matrix* ltm = NULL);
void dxRenderLines(int numLines, Line* lines, const Color* color, Matrix* ltm = NULL);
void dxRenderAABB(AABB* box, const Color* color, Matrix* ltm = NULL);
void dxRenderSphere(Sphere* sphere, const Color* color, Matrix* ltm = NULL);

#endif
