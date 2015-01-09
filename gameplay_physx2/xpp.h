
#ifndef XASSET_PREPROCESSOR_INCLUDED
#define XASSET_PREPROCESSOR_INCLUDED

#include "../shared/engine.h"

namespace xpp
{

typedef void (*xppMethod)(engine::IClump* clump);

/**
 * entry function
 * enumerate clump names and apply specific preprocessor for each clump
 */

void preprocessXAsset(engine::IAsset* asset);

/**
 * preprocessing methods
 */

void preprocessBush(engine::IClump* clump);
void preprocessTree(engine::IClump* clump);
void preprocessCrowdMale01(engine::IClump* clump);
void preprocessBaseJumper01(engine::IClump* clump);
void preprocessOstankinoCutscene(engine::IClump* clump);
void preprocessPilotchute(engine::IClump* clump);
void preprocessCanopy02(engine::IClump* clump);
void preprocessSlider(engine::IClump* clump);
void preprocessTraffic(engine::IClump* clump);
void preprocessAirplane(engine::IClump* clump);
void preprocessHelicopter(engine::IClump* clump);

}

#endif