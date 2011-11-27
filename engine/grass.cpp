
#include "headers.h"
#include "grass.h"
#include "geometry.h"
#include "camera.h"
#include "ixml.h"
#include "../common/istring.h"
#include "wire.h"
#include "gui.h"
#include "../common/unicode.h"

const DWORD maxParticlesPerPass = 8192;
const DWORD particleFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

/**
 * vertex structure
 */
struct GrassParticleVertex
{
    Vector  pos;    // position
    Color   color;  // color
    Flector uv;     // uv
};

/**
 * creation routine
 */

engine::IRendering* Engine::createGrass(const char* resourcePath, engine::IAtomic* templateAtomic, engine::ITexture* texture, engine::GrassScheme* grassScheme, float fadeStart, float fadeEnd)
{
    return new Grass( resourcePath, templateAtomic, texture, grassScheme, fadeStart, fadeEnd );
}

/**
 * GrassCluster
 */

void GrassCluster::updateBoundingSphere(void)
{
    // reset bounding sphere
    boundingSphere.center.x = boundingSphere.center.y = boundingSphere.center.z = 0.0f;
    boundingSphere.radius = 0.0f;

    // bushes aren't present?
    if( numParticles == NULL ) return;

    // initialize bounding sphere center calculations
    boundingSphere.center.x = particles[0].matrix._41,
    boundingSphere.center.y = particles[0].matrix._42,
    boundingSphere.center.z = particles[0].matrix._43;

    // calculate bounding sphere center
    Vector pos;
    Vector temp;
    for( unsigned int i=1; i<numParticles; i++ )
    {
        pos.x = particles[i].matrix._41,
        pos.y = particles[i].matrix._42,
        pos.z = particles[i].matrix._43;
        D3DXVec3Add( &temp, &boundingSphere.center, &pos );
        boundingSphere.center = temp;
    }
    D3DXVec3Scale( &temp, &boundingSphere.center, 1.0f/numParticles );
    boundingSphere.center = temp;

    // initialize bounding sphere radius calculations 
    pos.x = particles[0].matrix._41,
    pos.y = particles[0].matrix._42,
    pos.z = particles[0].matrix._43;
    D3DXVec3Subtract( &temp, &boundingSphere.center, &pos );
    boundingSphere.radius = D3DXVec3LengthSq( &temp );

    // calculate bounding sphere radius
    float distance;
    for( i=1; i<numParticles; i++ )
    {
        pos.x = particles[i].matrix._41,
        pos.y = particles[i].matrix._42,
        pos.z = particles[i].matrix._43;
        D3DXVec3Subtract( &temp, &boundingSphere.center, &pos );
        distance = D3DXVec3LengthSq( &temp );
        if( distance > boundingSphere.radius ) boundingSphere.radius = distance;
    }
    boundingSphere.radius = sqrt( boundingSphere.radius );
}

void GrassCluster::push_back(GrassParticle* particle)
{
    if( particles == NULL )
    {
        particles = new GrassParticle;
        *particles = *particle;
        numParticles++;
    }
    else
    {
        numParticles++;
        GrassParticle* temp = new GrassParticle[numParticles];
        memcpy( temp, particles, sizeof(GrassParticle)*(numParticles-1) );
        temp[numParticles-1] = *particle;
        delete[] particles;
        particles = temp;
    }
}

/**
 * Grass rendering implementation
 */

Grass::Grass(const char* resourcePath, engine::IAtomic* templateAtomic, engine::ITexture* texture, engine::GrassScheme* grassScheme, float fadeStart, float fadeEnd)
{
    _fadeStart = fadeStart;
    _fadeEnd = fadeEnd;
    if( _fadeEnd <= _fadeStart ) _fadeEnd = _fadeStart + 100.0f;

    // create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(GrassParticleVertex) * 4 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );

    // obtain ambient
    _ambientR = (unsigned char)( grassScheme->ambient[0] * 255 );
    _ambientG = (unsigned char)( grassScheme->ambient[1] * 255 );
    _ambientB = (unsigned char)( grassScheme->ambient[2] * 255 );

    // create shader
    _shader = dynamic_cast<Shader*>( Engine::instance->createShader( 1, "GrassShader" ) );
    _shader->_numReferences++;
    _shader->setLayerTexture( 0, texture );
    _shader->setLayerUV( 0, 0 );
    _shader->setFlags( _shader->getFlags() | engine::sfAlphaBlending | engine::sfAlphaTesting | engine::sfLighting );
    _shader->setFlags( _shader->getFlags() & ~engine::sfCulling );
    _shader->setAlphaTestFunction( engine::cfGreater );
    _shader->setAlphaTestRef( 0 );
    _shader->setSrcBlend( engine::bmSrcAlpha );
    _shader->setDestBlend( engine::bmInvSrcAlpha );
    _shader->setBlendOp( engine::bpAdd );
    _shader->setDiffuseColor( Vector4f( 1,1,1,1 ) );
    _shader->setSpecularColor( Vector4f( 1,1,1,1 ) );
    // try to load grass from resource
    IResource* resource = getCore()->getResource( resourcePath, "rb" );
    if( resource )
    {
        // read grass solution from resource
        if( Engine::instance->progressCallback )
        {
            Engine::instance->progressCallback( 
                Gui::iLanguage->getUnicodeString(1), 
                1.0f, 
                Engine::instance->progressCallbackUserData 
            );
        }
        unsigned int numClusters;
        unsigned int numParticles;
        GrassCluster* cluster;
        fread( &numClusters, sizeof(unsigned int), 1, resource->getFile() );
        for( unsigned int i=0; i<numClusters; i++ )
        {
            fread( &numParticles, sizeof(unsigned int), 1, resource->getFile() );            
            cluster = new GrassCluster( numParticles );
            fread( &cluster->boundingSphere, sizeof(Sphere), 1, resource->getFile() );
            fread( cluster->particles, sizeof(GrassParticle), numParticles, resource->getFile() );
            _clusters.push_back( cluster );
        }
    }
    // generate grass and write generation result to resource
    else
    {
        // reset random number generation
        srand( GetTickCount() );

        // make solid storage for particles
        GrassParticles temp;

        // pass grass species
        engine::GrassSpecie* specie = grassScheme->species;
        assert( specie != NULL );
        while( specie->name != NULL )
        {
            // generate particles for this specie
            generateSpecie( specie, templateAtomic, temp );
            // next specie
            specie++;
        }

        // regroup global cluster
        regroupCluster( temp, grassScheme->clisterSize );

        // write grass solution to resource
        if( Engine::instance->progressCallback )
        {
            Engine::instance->progressCallback( 
                Gui::iLanguage->getUnicodeString(2),
                1.0f, 
                Engine::instance->progressCallbackUserData 
            );
        }
        resource = getCore()->getResource( resourcePath, "wb" ); assert( resource );
        unsigned int buffer = _clusters.size();
        fwrite( &buffer, sizeof(unsigned int), 1, resource->getFile() );
        for( GrassClusterI grassClusterI = _clusters.begin();
                           grassClusterI != _clusters.end();
                           grassClusterI++ )
        {
            buffer = (*grassClusterI)->numParticles;
            fwrite( &buffer, sizeof(unsigned int), 1, resource->getFile() );
            fwrite( &(*grassClusterI)->boundingSphere, sizeof(Sphere), 1, resource->getFile() );
            fwrite( (*grassClusterI)->particles, sizeof(GrassParticle), buffer, resource->getFile() );
        }
    }
}

Grass::~Grass()
{
    _shader->release();
    _indexBuffer->Release();
    _vertexBuffer->Release();
    for( GrassClusterI grassClusterI = _clusters.begin();
                       grassClusterI != _clusters.end();
                       grassClusterI++ )
    {
        delete *grassClusterI;
    }
}

/**
 * generation routine
 */

const float oneDivThree = 1.0f/3.0f;
const float oneDivTwo = 1.0f/2.0f;

static Vector e0Scaled, e1Scaled;
static Vector e0Pos, e1Pos;
static Vector eCross, eCrossScaled;

static void generateRandomPosition(Vector* out, Vector* v0, Vector* e0, Vector* v1, Vector* e1)
{
    D3DXVec3Scale( &e0Scaled, e0, getUniform( 0,1 ) );
    D3DXVec3Scale( &e1Scaled, e1, getUniform( 0,1 ) );
    D3DXVec3Add( &e0Pos, &e0Scaled, v0 );
    D3DXVec3Add( &e1Pos, &e1Scaled, v1 );
    D3DXVec3Subtract( &eCross, &e0Pos, &e1Pos );
    D3DXVec3Scale( &eCrossScaled, &eCross, getUniform( 0,1 ) );
    D3DXVec3Add( out, &e1Pos, &eCrossScaled );
}

static void generateRandomPosition(Vector* out, Vector* vertices, Vector* edges)
{
    if( getUniform( 0,1 ) < oneDivThree )
    {
        generateRandomPosition( out, vertices+0, edges+0, vertices+0, edges+1 );
    }
    else if( getUniform( 0,1 ) < oneDivTwo )
    {
        generateRandomPosition( out, vertices+0, edges+0, vertices+1, edges+2 );
    }
    else
    {
        generateRandomPosition( out, vertices+1, edges+2, vertices+0, edges+1 );
    }
}

void Grass::generateSpecie(engine::GrassSpecie* specie, engine::IAtomic* templateAtomic, GrassParticles& solidStorage)
{
    // read specie info
    Flector uv[4];
    Flector size;
    Flector sizeBias;
    float   density;
    uv[0]    = wrap( specie->uv[0] );
    uv[1]    = wrap( specie->uv[1] );
    uv[2]    = wrap( specie->uv[2] );
    uv[3]    = wrap( specie->uv[3] );
    size     = wrap( specie->size );
    sizeBias = wrap( specie->sizeBias );
    density  = specie->density;

    // template data
    Matrix LTM = wrap( templateAtomic->getFrame()->getLTM() );
    Geometry* geometry = dynamic_cast<Geometry*>( templateAtomic->getGeometry() );    
    unsigned int numTriangles = geometry->getNumTriangles();
    Triangle* triangles = geometry->getTriangles();
    Vector* vertices = geometry->getVertices();
    
    Vector vertex[3];
    Vector edge[3];
    Vector edgeN[2];
    Vector normal;
    Vector pos,pos2,sx;
    Flector s;
    float cosA, sinA, angle, square, probability;
    unsigned int numParticlesInTriangle;
    unsigned int i,j;
    GrassParticle grassParticle;

    // no tiling? - iterate template triangles 
    for( i=0; i<numTriangles; i++ )
    {
        // progress
        if( Engine::instance->progressCallback )
        {
            Engine::instance->progressCallback(
                wstrformat( Gui::iLanguage->getUnicodeString(5), asciizToUnicode(specie->name).c_str() ).c_str(),
                float( i ) / float( numTriangles ),
                Engine::instance->progressCallbackUserData
            );
        }

        // transform triangle vertices to world space
        D3DXVec3TransformCoord( vertex+0, vertices+triangles[i].vertexId[0], &LTM );
        D3DXVec3TransformCoord( vertex+1, vertices+triangles[i].vertexId[1], &LTM );
        D3DXVec3TransformCoord( vertex+2, vertices+triangles[i].vertexId[2], &LTM );
        // calculate triangle square value...
        D3DXVec3Subtract( edge+0, vertex+1, vertex+0 );
        D3DXVec3Subtract( edge+1, vertex+2, vertex+0 );
        D3DXVec3Subtract( edge+2, vertex+2, vertex+1 );
        D3DXVec3Normalize( edgeN+0, edge+0 );
        D3DXVec3Normalize( edgeN+1, edge+1 );
        D3DXVec3Cross( &normal, edgeN+0, edgeN+1 );
        cosA = D3DXVec3Dot( edgeN+0, edgeN+1 );
        if( cosA > 1.0f ) angle = 0.0f; 
        else if( cosA < -1.0f ) angle = 180;
        else angle = acosf( cosA );
        sinA = sin( angle );
        square = 0.5f * D3DXVec3Length( edge+0 ) * D3DXVec3Length( edge+1 ) * sinA / 10000.0f;
        // obtain number of particles in this triangle
        numParticlesInTriangle = unsigned int( square * density );
        if( !numParticlesInTriangle )
        {
            // include probability method to decide to place grass on to this triangle
            probability = square / ( 1 / density ); 
            assert( probability >= 0.0f );
            assert( probability <= 1.0f );
            if( getUniform( 0, 1 ) <= probability ) numParticlesInTriangle++;
        }
        // generate particles
        grassParticle.uv[0] = uv[0];
        grassParticle.uv[1] = uv[1];
        grassParticle.uv[2] = uv[2];
        grassParticle.uv[3] = uv[3];
        for( j=0; j<numParticlesInTriangle; j++ )
        {
            // generate particle size
            s.x = size.x + getUniform( -sizeBias.x, +sizeBias.x );
            s.y = size.y + getUniform( -sizeBias.y, +sizeBias.y );
            // generate particle coordinate
            generateRandomPosition( &pos, vertex, edge );
            pos += normal * ( 1 + s.y );
            D3DXMatrixIdentity( &grassParticle.matrix );
            grassParticle.matrix._11 *= s.x,
            grassParticle.matrix._12 *= s.x,
            grassParticle.matrix._13 *= s.x,
            grassParticle.matrix._21 *= s.y,
            grassParticle.matrix._22 *= s.y,
            grassParticle.matrix._23 *= s.y;            
            dxRotate( &grassParticle.matrix, &oY, getUniform( 0,360 ) );
            grassParticle.matrix._41 = pos.x;
            grassParticle.matrix._42 = pos.y;
            grassParticle.matrix._43 = pos.z;
            grassParticle.matrix._44 = 1.0f;            
            // add particle to global cluster
            solidStorage.push_back( grassParticle );
        }
    }
}

void Grass::regroupCluster(GrassParticles& solidStorage, float clusterSize)
{
    GrassCluster* cluster = NULL;
    GrassClusterI clusterI;
    Vector distance, pos;
    bool processed;
    unsigned int progress = 0;
    unsigned int temp;

    // for all incoming particles
    for( unsigned int i=0; i<solidStorage.size(); i++ )
    {
        // progress
        if( Engine::instance->progressCallback )
        {
            temp = unsigned int( 100.0f * float( i ) / float( solidStorage.size() ) );
            if( temp > progress )
            {
                progress = temp;
                Engine::instance->progressCallback( 
                    Gui::iLanguage->getUnicodeString(3),
                    float( progress )/100.0f, 
                    Engine::instance->progressCallbackUserData 
                );
            }
        }

        // search for cluster
        processed = false;
        pos.x = solidStorage[i].matrix._41,
        pos.y = solidStorage[i].matrix._42,
        pos.z = solidStorage[i].matrix._43;
        for( clusterI = _clusters.begin(); clusterI != _clusters.end(); clusterI++ )
        {            
            D3DXVec3Subtract( &distance, &pos, &(*clusterI)->boundingSphere.center );
            if( D3DXVec3Length( &distance ) < clusterSize )
            {
                (*clusterI)->push_back( &solidStorage[i] );
                processed = true;
                break;
            }
        }
        if( !processed )
        {
            cluster = new GrassCluster;
            cluster->boundingSphere.radius = 0;
            cluster->boundingSphere.center = pos;
            cluster->push_back( &solidStorage[i] );
            _clusters.push_back( cluster );
        }        
    }

    // update cluster
    for( clusterI = _clusters.begin(); clusterI != _clusters.end(); clusterI++ )
    {
        (*clusterI)->updateBoundingSphere();
    }
}

/**
 * IRendering
 */

void Grass::setProperty(const char* propertyName, float value)
{
    assert( !"Useless call of Grass::setProperty()" );
}

void Grass::setProperty(const char* propertyName, const Vector3f& value)
{
    assert( !"Useless call of Grass::setProperty()" );
}

void Grass::setProperty(const char* propertyName, const Vector2f& value)
{
    assert( !"Useless call of Grass::setProperty()" );
}

void Grass::setProperty(const char* propertyName, const Vector4f& value)
{
    assert( !"Useless call of Grass::setProperty()" );
}

void Grass::setProperty(const char* propertyName, const Matrix4f& value)
{
    assert( !"Useless call of Grass::setProperty()" );
}

void Grass::release(void)
{
    delete this;
}

static const Vector billboardVertices[4] = 
{
    Vector( -1.0f, -1.0f, 0.0f ),
    Vector( -1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, -1.0f, 0.0f )
};

void Grass::render(void)
{
    float  alpha, dot;
    Vector p,z;
    Vector vector;
    Matrix m;
    unsigned int i,j;

    // culling value
    float cullDot = cos( Camera::fov * D3DX_PI / 180.0f );

    // fill items-to-sort
    _numItems = 0;

    // pass clusters, build list of unsorted particles
    GrassCluster* cluster;
    for( GrassClusterI grassClusterI=_clusters.begin(); 
                       grassClusterI!=_clusters.end(); 
                       grassClusterI++ )
    {
        cluster = *grassClusterI;
        // cluster is in visible area?
        D3DXVec3Subtract( &vector, &Camera::eyePos, &cluster->boundingSphere.center );
        alpha = D3DXVec3Length( &vector );
        if( alpha <= ( _fadeEnd + cluster->boundingSphere.radius ) )
        {
            // pass grass particles
            for( i=0; i<cluster->numParticles; i++ )
            {
                // particle alpha value
                p.x = cluster->particles[i].matrix._41,
                p.y = cluster->particles[i].matrix._42,
                p.z = cluster->particles[i].matrix._43;
                D3DXVec3Subtract( &vector, &p, &Camera::eyePos );
                cluster->particles[i].distance = D3DXVec3Length( &vector );
                alpha = cluster->particles[i].distance;
                alpha = ( alpha - _fadeStart ) / ( _fadeEnd - _fadeStart );               
                if( alpha < 0 ) alpha = 0;  
                // particle culling
                D3DXVec3Normalize( &z, &vector );
                dot = D3DXVec3Dot( &z, &Camera::eyeDirection );
                if( -dot > cullDot && alpha < 1.0f )
                {
                    cluster->particles[i].alpha = alpha;
                    _items[_numItems] = &cluster->particles[i];
                    _numItems++;
                }
                if( _numItems == 65535 ) break;
            }
        }
        if( _numItems == 65535 ) break;
    }

    if( _numItems == 0 ) return;

    // reset nest counters
    memset( _nestSize, 0, sizeof( _nestSize ) );

    // sort particles
    float step = _fadeEnd / 255.0f;
    unsigned char key;
    unsigned char* nestSize;
    for( i=0; i<_numItems; i++ )
    {
        key = 255 - unsigned char( _items[i]->distance / step );
        nestSize = _nestSize + key;
        // search for nearest thin nest
        // a priori, central nests (lower nestId) are more vacant
        while( *nestSize == 255 && key > 0 ) key--, nestSize = _nestSize + key;
        // leave particle unsorted & unrendered if there are no space to sort it
        if( *nestSize == 255 ) continue;
        // add particle to corresponding nest
        _nest[key][*nestSize] = _items[i];
        (*nestSize)++;
    }

    // lock buffers
    void* vertexData = NULL;
    void* indexData = NULL;
    _dxCR( _vertexBuffer->Lock( 0, maxParticlesPerPass * 4 * sizeof( GrassParticleVertex ), &vertexData, D3DLOCK_DISCARD ) );
    _dxCR( _indexBuffer->Lock( 0, maxParticlesPerPass * 6 * sizeof( WORD ), &indexData, D3DLOCK_DISCARD ) );
    assert( vertexData );
    assert( indexData );
    GrassParticleVertex* vertex = (GrassParticleVertex*)( vertexData );
    WORD* index = (WORD*)( indexData );

    // render particles
    unsigned int passParticles = 0;
    for( i=0; i<256; i++ )
    {
        nestSize = _nestSize + i;
        for( j=0; j<*nestSize; j++ )
        {          
            // build billboard matrix
            m = _nest[i][j]->matrix;
            m._41 = m._42 = m._43 = 0.0f, m._44 = 1.0f;
            p.x = _nest[i][j]->matrix._41,
            p.y = _nest[i][j]->matrix._42,
            p.z = _nest[i][j]->matrix._43;
            // transform vertex coordinates by matrix
            D3DXVec3TransformCoord( &vertex[0].pos, &billboardVertices[0], &m );
            D3DXVec3TransformCoord( &vertex[1].pos, &billboardVertices[1], &m );
            D3DXVec3TransformCoord( &vertex[2].pos, &billboardVertices[2], &m );
            D3DXVec3TransformCoord( &vertex[3].pos, &billboardVertices[3], &m );
            vertex[0].pos.x += p.x,
            vertex[0].pos.y += p.y,
            vertex[0].pos.z += p.z,
            vertex[1].pos.x += p.x,
            vertex[1].pos.y += p.y,
            vertex[1].pos.z += p.z,
            vertex[2].pos.x += p.x,
            vertex[2].pos.y += p.y,
            vertex[2].pos.z += p.z,
            vertex[3].pos.x += p.x,
            vertex[3].pos.y += p.y,
            vertex[3].pos.z += p.z;
            // setup uvs
            vertex[0].uv = _nest[i][j]->uv[0];
            vertex[1].uv = _nest[i][j]->uv[1];
            vertex[2].uv = _nest[i][j]->uv[2];
            vertex[3].uv = _nest[i][j]->uv[3];
            // setup colors
            vertex[0].color = 
            vertex[1].color = 
            vertex[2].color = 
            vertex[3].color = D3DCOLOR_RGBA( 
                _ambientR, 
                _ambientG, 
                _ambientB, 
                unsigned int( 255.0f * ( 1.0f - _nest[i][j]->alpha ) ) 
            );
            // indices...
            index[0] = passParticles * 4 + 0;
            index[1] = passParticles * 4 + 1;
            index[2] = passParticles * 4 + 2;
            index[3] = passParticles * 4 + 0;
            index[4] = passParticles * 4 + 2;
            index[5] = passParticles * 4 + 3;
            // next particle
            vertex += 4, index += 6, passParticles++;
            // buffers is full? - render
            if( passParticles == maxParticlesPerPass )
            {
                // unlock buffers
                _vertexBuffer->Unlock();
                _indexBuffer->Unlock();
                // render buffers
                renderBuffers( passParticles );
                // relock buffers
                _dxCR( _vertexBuffer->Lock( 0, maxParticlesPerPass * 4 * sizeof( GrassParticleVertex ), &vertexData, D3DLOCK_DISCARD ) );
                _dxCR( _indexBuffer->Lock( 0, maxParticlesPerPass * 6 * sizeof( WORD ), &indexData, D3DLOCK_DISCARD ) );
                assert( vertexData );
                assert( indexData );
                vertex = (GrassParticleVertex*)( vertexData );
                index = (WORD*)( indexData );
                passParticles = 0;
            }
        }
    }

    // unlock buffers
    _vertexBuffer->Unlock();
    _indexBuffer->Unlock();

    if( passParticles != 0 )
    {        
        // render buffers
        renderBuffers( passParticles );
    }
}

void Grass::renderBuffers(unsigned int numPassParticles)
{
    // render
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, FALSE ) );    
    _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );
    _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, TRUE ) );
    _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 ) );
    _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL ) );
    
    _shader->apply();

    _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
    _dxCR( iDirect3DDevice->SetFVF( particleFVF ) );
    _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( GrassParticleVertex ) ) );
    _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
    _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numPassParticles * 4, 0, numPassParticles * 2 ) );

    _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, TRUE ) ); 
}

void Grass::onLostDevice(void)
{
    _indexBuffer->Release();
    _vertexBuffer->Release();
}

void Grass::onResetDevice(void)
{
    // create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(GrassParticleVertex) * 4 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );
}