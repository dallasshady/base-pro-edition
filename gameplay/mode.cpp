
#include "headers.h"
#include "scene.h"

Mode::Mode(Scene* scene) : Actor( scene )
{
    scene->queryMode( this );
}

Mode::~Mode()
{
}