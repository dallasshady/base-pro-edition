
#include "headers.h"
#include "smokejet.h"
#include "imath.h"

/**
 * class implementation
 */

SmokeJet::SmokeJet(Jumper* jumper, Vector4f color, SmokeJetMode mode) : Actor( jumper )
{
    _jumper = jumper;
    _mode   = mode;
    _name   = "SmokeJet";

    // load texture
    engine::ITexture* texture;
    texture = Gameplay::iEngine->getTexture( "smoketrail" );
    if( !texture )
    {
        texture = Gameplay::iEngine->createTexture( "./res/particles/smoketrail.dds" );
        texture->setMinFilter( engine::ftAnisotropic );
        texture->setMagFilter( engine::ftLinear );
        texture->setMipFilter( engine::ftLinear );
        texture->setMaxAnisotropy( 8 );
        assert( texture );
    }

    // create smoke trail shader
    _shader = Gameplay::iEngine->createShader( 1, "SmokeBallShader" ); assert( _shader );
    _shader->setFlags( engine::sfAlphaBlending | engine::sfAlphaTesting | engine::sfLighting );
    _shader->setAlphaTestFunction( engine::cfGreater );
    _shader->setAlphaTestRef( 0 );
    _shader->setLayerTexture( 0, texture );
    _shader->setSrcBlend( engine::bmSrcAlpha );
    _shader->setDestBlend( engine::bmInvSrcAlpha );
    _shader->setBlendOp( engine::bpAdd );
    _shader->addReference();

    // create smoke trail scheme
    _scheme.numParticles = 256;
    _scheme.fissionLERP = engine::SmokeTrailScheme::LERPValue( 0.0f, 25.0f, 5000.0f, 250.0f );
    _scheme.damping = 1.0f;
    _scheme.heat = 125.0f;
    _scheme.lifeTime = 25.0f;
    _scheme.fadeTime = 2.5f;
    _scheme.sizeTimeLERP  = engine::SmokeTrailScheme::LERPValue( 0.0f, 2.5f, 5000.0f, 2.5f );
    _scheme.startSizeLERP = engine::SmokeTrailScheme::LERPValue( 0.0f, 0.0f, 1000.0f, 0.0f );
    _scheme.endSizeLERP   = engine::SmokeTrailScheme::LERPValue( 0.0f, 100.0f, 2500.0f, 500.0f );
    _scheme.uv[0].set( 0.0f, 0.0625f );
    _scheme.uv[1].set( 1.0f, 0.0625f );
    _scheme.uv[2].set( 1.0f, 0.0f );
    _scheme.uv[3].set( 0.0f, 0.0f );
    _scheme.ambient = color;

    // create smoke trail from scheme
    _smokeTrail = Gameplay::iEngine->createSmokeTrail( _shader, &_scheme );

    // start emission
    onUpdateActivity( 0.0f );
    _smokeTrail->setProperty( "enabled", 1.0f );
    _enabled = true;
    _scene->addSmokeTrail( _smokeTrail );
}

SmokeJet::~SmokeJet()
{
    _scene->removeSmokeTrail( _smokeTrail );
    _smokeTrail->release();
    _shader->release();
}

/**
 * Actor behaviour
 */

void SmokeJet::onUpdateActivity(float dt)
{
    _jumper->getClump()->getFrame()->getLTM();

    // update smoke trail
    Vector3f pos;
    Vector3f dir;
    switch( _mode )
    {
    case sjmLeft:
        pos = Jumper::getLeftSmokeJetAnchor( _jumper->getClump() )->getPos();
        dir = _jumper->getClump()->getFrame()->getRight();
        break;
    case sjmRight:
        pos = Jumper::getRightSmokeJetAnchor( _jumper->getClump() )->getPos();
        dir = _jumper->getClump()->getFrame()->getRight() * -1;        
        break;
    }
    dir.normalize();
    dir *= 250.0f;

    _smokeTrail->setProperty( "emissionPoint", pos );
    _smokeTrail->setProperty( "emissionDirection", dir );
    _smokeTrail->setProperty( "windVelocity", wrap( _scene->getWindAtPoint( wrap( _jumper->getClump()->getFrame()->getPos() ) ) ) );
    _smokeTrail->setProperty( "emitterVelocity", _jumper->getVel() );
    _smokeTrail->setProperty( "update", dt );
}

/**
 * class behaviour
 */

bool SmokeJet::isEnabled(void)
{
    return _enabled;
}

void SmokeJet::enable(bool flag)
{
    _enabled = flag;
    _smokeTrail->setProperty( "enabled", ( flag ? 1.0f : 0.0f ) );
}

void SmokeJet::setColor(Vector4f color)
{
    _smokeTrail->setProperty( "ambientColor", Vector3f( color[0], color[1], color[2] ) );
}