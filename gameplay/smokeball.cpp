
#include "headers.h"
#include "smokeball.h"
#include "imath.h"

/**
 * actor abstracts
 */

void SmokeBall::onUpdateActivity(float dt)
{
}

/**
 * class implementation
 */

SmokeBall::SmokeBall(Actor* parent, SmokeBallDesc* desc) : Actor( parent )
{
    // check descriptor
    assert( desc->numParticles > 0 );

    // save descriptor
    _desc = *desc;

    // reset flag
    _userFlag = false;

    // load texture
    engine::ITexture* texture;
    texture = Gameplay::iEngine->getTexture( "smoke" );
    if( !texture )
    {
        texture = Gameplay::iEngine->createTexture( "./res/particles/smoke.dds" );
        texture->setMinFilter( engine::ftLinear );
        texture->setMagFilter( engine::ftLinear );
        texture->setMipFilter( engine::ftLinear );
        assert( texture );
    }

    // create shader
    _shader = Gameplay::iEngine->createShader( 1, "SmokeBallShader" ); assert( _shader );
    _shader->setFlags( engine::sfAlphaBlending | engine::sfAlphaTesting | engine::sfLighting );
    _shader->setAlphaTestFunction( engine::cfGreater );
    _shader->setAlphaTestRef( 0 );
    _shader->setLayerTexture( 0, texture );
    _shader->setSrcBlend( engine::bmSrcAlpha );
    _shader->setDestBlend( engine::bmInvSrcAlpha );
    _shader->setBlendOp( engine::bpAdd );

    // create particle system
    _particleSystem = Gameplay::iEngine->createParticleSystem( _desc.numParticles, _shader, _desc.radius );
    assert( _particleSystem );
    _scene->addParticleSystem( _particleSystem );

    // generate constant properties for each particle
    Vector3f offset;
    engine::Particle* particles = _particleSystem->getParticles();
    for( unsigned int i=0; i<_desc.numParticles; i++ )
    {
        particles[i].visible = true;
        particles[i].color = _desc.color;
        particles[i].size.set( _desc.particleRadius, _desc.particleRadius );
        switch( getCore()->getRandToolkit()->getUniformInt() % 4 )
        {
        case 0:
            particles[i].uv[0].set( 0.5f, 0.5f );
            particles[i].uv[1].set( 0.5f, 0.0f );
            particles[i].uv[2].set( 0.0f, 0.0f );
            particles[i].uv[3].set( 0.0f, 0.5f );
            break;
        case 1:
            particles[i].uv[0].set( 1.0f, 0.5f );
            particles[i].uv[1].set( 1.0f, 0.0f );
            particles[i].uv[2].set( 0.5f, 0.0f );
            particles[i].uv[3].set( 0.5f, 0.5f );
            break;
        case 2:
            particles[i].uv[0].set( 0.5f, 1.0f );
            particles[i].uv[1].set( 0.5f, 0.5f );
            particles[i].uv[2].set( 0.0f, 0.5f );
            particles[i].uv[3].set( 0.0f, 1.0f );
            break;
        case 3:
            particles[i].uv[0].set( 1.0f, 1.0f );
            particles[i].uv[1].set( 1.0f, 0.5f );
            particles[i].uv[2].set( 0.5f, 0.5f );
            particles[i].uv[3].set( 0.5f, 1.0f );
            break;
        }
        // place particle
        particles[i].position = _desc.center;
        offset.set(
            getCore()->getRandToolkit()->getUniform( -1,1 ),
            getCore()->getRandToolkit()->getUniform( -1,1 ),
            getCore()->getRandToolkit()->getUniform( -1,1 )
        );
        offset.normalize();
        offset *= _desc.radius * pow(
            getCore()->getRandToolkit()->getUniform( 0, 1 ), 1.25f
        );
        particles[i].position += offset;
    }

    // calculate bounding sphere
    float radius;
    _sphereCenter = particles[0].position;
    for( i=1; i<_desc.numParticles; i++ )
    {
        _sphereCenter += particles[i].position;
    }
    _sphereCenter *= ( 1.0f / _desc.numParticles );
    _sphereRadius = 0;
    for( i=0; i<_desc.numParticles; i++ )
    {
        radius = ( particles[i].position - _sphereCenter ).length();
        if( _sphereRadius < radius ) _sphereRadius = radius;        
    }
}

SmokeBall::~SmokeBall()
{
    _scene->removeParticleSystem( _particleSystem );
    _particleSystem->release();
    _shader->release();
}

/** 
 * class behaviour
 */

bool SmokeBall::isInside(Vector3f point)
{
    return ( point - _sphereCenter ).length() < _sphereRadius;
}

bool SmokeBall::getUserFlag(void)
{
    return _userFlag;
}

void SmokeBall::setUserFlag(bool value)
{
    _userFlag = value;
}

SmokeBallDesc* SmokeBall::getDescriptor(void)
{
    return &_desc;
}