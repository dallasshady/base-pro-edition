
#include "headers.h"
#include "engine.h"
#include "collision.h"
#include "glow.h"

/**
 * kurieitoru
 */

engine::IRendering* Engine::createGlow(engine::IBSP* bsp, const char* textureResource)
{
    return new Glow( dynamic_cast<BSP*>( bsp ), textureResource );
}

/**
 * class implementation
 */

engine::ILight* Glow::collectLights(engine::ILight* light, void* data)
{
    Glow* __this = reinterpret_cast<Glow*>( data );
    Light* l = dynamic_cast<Light*>( light ); assert( l );
    if( l->getType() == engine::ltPoint ) __this->_glowParticles.push_back( GlowParticle( l ) );
    return light;
}

engine::IClump* Glow::collectLights(engine::IClump* clump, void* data)
{
    clump->forAllLights( collectLights, data );
    return clump;
}

/**
 * class implementation
 */

Glow::Glow(BSP* bsp, const char* glowTextureResource)
{
    _bsp = bsp;    

    // collect light sources
    _bsp->forAllLights( collectLights, this );
    _bsp->forAllClumps( collectLights, this );

    // no lights? - no glow
    if( _glowParticles.size() )
    {
        // obtain texture
        Texture* glowTexture = NULL;
        std::string glowTextureName = Texture::getTextureNameFromFilePath( glowTextureResource );
        TextureI glowTextureI = Texture::textures.find( glowTextureName.c_str() );
        if( glowTextureI == Texture::textures.end() )
        {
            glowTexture = Texture::createTexture( glowTextureResource );
            glowTexture->setMagFilter( engine::ftLinear );
            glowTexture->setMinFilter( engine::ftLinear );
            glowTexture->setMipFilter( engine::ftNone );
        }
        else
        {
            glowTexture = glowTextureI->second;
        }
        assert( glowTexture );

        // create shader
        _shader = new Shader( 1, "GlowShader" );
        _shader->setLayerTexture( 0, glowTexture );
        _shader->setLayerUV( 0, 0 );
        _shader->setFlags( _shader->getFlags() | engine::sfAlphaBlending );
        _shader->setSrcBlend( engine::bmOne );
        _shader->setDestBlend( engine::bmOne );
        _shader->setBlendOp( engine::bpAdd );
        _shader->addReference();

        // create particle system
        _particleSystem = new ParticleSystem( _glowParticles.size(), _shader, 0.0f );
    }
    else
    {
        _shader = NULL;
        _particleSystem = NULL;
    }

    // reset properties
    _dt = 0.0f;
    _fadeSpeed.set( 1,1,1,0 );
    _minSizeDistance = 0.0f;
    _minSize.set( 5.0f, 5.0f );
    _maxSizeDistance = 10000.0f;
    _maxSize.set( 50.0f, 50.0f );
}

Glow::~Glow()
{
    if( _particleSystem ) _particleSystem->release();
    if( _shader ) _shader->release();    
}

/**
 * IRendering
 */

void Glow::release(void)
{
    delete this;
}

void Glow::setProperty(const char* propertyName, float value)
{
    if( strcmp( propertyName, "dt" ) == 0 ) 
    {
        _dt = value;
    }
    else if( strcmp( propertyName, "minSizeDistance" ) == 0 ) 
    {
        _minSizeDistance = value;
    }
    else if( strcmp( propertyName, "maxSizeDistance" ) == 0 ) 
    {
        _maxSizeDistance = value;
    }
}

void Glow::setProperty(const char* propertyName, const Vector2f& value)
{
    if( strcmp( propertyName, "minSize" ) == 0 ) 
    {
        _minSize = value;
    }
    else if( strcmp( propertyName, "maxSize" ) == 0 ) 
    {
        _maxSize = value;
    }    
}

void Glow::setProperty(const char* propertyName, const Vector3f& value)
{
}

void Glow::setProperty(const char* propertyName, const Vector4f& value)
{
    if( strcmp( propertyName, "fadeSpeed" ) == 0 )
    {
        _fadeSpeed = value;
        assert( _fadeSpeed[0] >= 0 );
        assert( _fadeSpeed[1] >= 0 );
        assert( _fadeSpeed[2] >= 0 );
        assert( _fadeSpeed[3] >= 0 );
    }
}

void Glow::setProperty(const char* propertyName, const Matrix4f& value)
{
}

void Glow::render(void)
{
    if( !_particleSystem ) return;

    // update particles
    float distance;
    float factor;
    Vector lightPos;
    engine::Particle* particles = _particleSystem->getParticles();
    for( unsigned int i=0; i<_glowParticles.size(); i++ )
    {
        // obtain position of light
        lightPos.x = _glowParticles[i].light->frame()->LTM._41;
        lightPos.y = _glowParticles[i].light->frame()->LTM._42;
        lightPos.z = _glowParticles[i].light->frame()->LTM._43;
        // frustrum test
        _glowParticles[i].state = intersectPointFrustum( &lightPos, Camera::frustrum );
        // update glow color
        Vector4f targetColor = _glowParticles[i].state ? _glowParticles[i].light->getDiffuseColor() : Vector4f(0,0,0,0);
        for( unsigned int j=0; j<4; j++ )
        {
            float diff = targetColor[j] - _glowParticles[i].color[j];
            float sign = diff < 0 ? -1.0f : ( diff > 0 ? 1.0f : 0.0f );
            if( sign != 0.0f )
            {
                float dc = sign * _fadeSpeed[j];
                _glowParticles[i].color[j] += dc * _dt;
                if( (( sign < 1 ) && ( _glowParticles[i].color[j] < targetColor[j] )) ||
                    (( sign > 1 ) && ( _glowParticles[i].color[j] > targetColor[j] )) )
                {
                    _glowParticles[i].color[j] = targetColor[j];
                }
            }
        }
        // update particle
        particles[i].visible  = _glowParticles[i].state;
        particles[i].position = wrap( lightPos );
        particles[i].rotation = 0.0f;
        distance = ( wrap( Camera::eyePos ) - particles[i].position ).length();
        factor = ( distance - _minSizeDistance ) / ( _maxSizeDistance - _minSizeDistance );
        factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
        particles[i].size = _minSize * ( 1 - factor ) + _maxSize * factor;
        particles[i].color = _glowParticles[i].color;
        particles[i].uv[0].set( 1,1 );
        particles[i].uv[1].set( 1,0 );
        particles[i].uv[2].set( 0,0 );
        particles[i].uv[3].set( 0,1 );
    }

    // render particles
    _particleSystem->render();

    // reset timestep
    _dt = 0.0f;
}