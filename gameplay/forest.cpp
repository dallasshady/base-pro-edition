
#include "headers.h"
#include "forest.h"
#include "callback.h"
#include "jumper.h"
#include "imath.h"
#include "NxIntersectionBoxBox.h" 
#include "NxSegment.h"
#include "NxExportedUtils.h"

/**
 * generation routine
 */

const float oneDivThree = 1.0f/3.0f;
const float oneDivTwo   = 1.0f/2.0f;

static Vector3f e0Scaled, e1Scaled;
static Vector3f e0Pos, e1Pos;
static Vector3f eCross, eCrossScaled;

static Vector3f generateRandomPosition(const Vector3f& v0, const Vector3f& e0, const Vector3f& v1, const Vector3f& e1)
{
    e0Scaled = e0 * getCore()->getRandToolkit()->getUniform( 0,1 );
    e1Scaled = e1 * getCore()->getRandToolkit()->getUniform( 0,1 );
    e0Pos = e0Scaled + v0;
    e1Pos = e1Scaled + v1;
    eCross = e0Pos - e1Pos;
    eCrossScaled = eCross * getCore()->getRandToolkit()->getUniform( 0,1 );
    return e1Pos + eCrossScaled;
}

Vector3f generateRandomPosition(Vector3f* vertices, Vector3f* edges)
{
    if( getCore()->getRandToolkit()->getUniform( 0,1 ) < oneDivThree )
    {
        return generateRandomPosition( vertices[0], edges[0], vertices[0], edges[1] );
    }
    else if( getCore()->getRandToolkit()->getUniform( 0,1 ) < oneDivTwo )
    {
        return generateRandomPosition( vertices[0], edges[0], vertices[1], edges[2] );
    }
    else
    {
        return generateRandomPosition( vertices[1], edges[2], vertices[0], edges[1] );
    }
}

/**
 * aux
 */

unsigned int Forest::getClumpLodId(engine::IClump* clump)
{
    unsigned int result = 0xFFFFFFFF;
    if( strstr( clump->getName(), "LOD0" ) )
    {
        result = 0;
    }
    else if( strstr( clump->getName(), "LOD1" ) )
    {
        result = 1;
    }
    else if( strstr( clump->getName(), "LOD2" ) )
    {
        result = 2;
    }
    else if( strstr( clump->getName(), "LOD3" ) ) 
    {
        result = 3;
    }
    else if( strstr( clump->getName(), "LOD4" ) ) 
    {
        result = 4;
    }
    else
    {
        throw Exception( "The LOD id of clump \"%s\" can't be recognised!", clump->getName() );
    }
    return result;
}

/**
 * class implementation
 */

static float getSquaredLength(const Vector3f& v)
{
    return v.dot( v );
}

float Forest::getDistanceToNearestTree(const Vector3f& pos)
{
    if( !_treeMatrix.size() ) return NX_MAX_F32;

    Matrix4f& matrix = _treeMatrix[0];
    float result = getSquaredLength( Vector3f( matrix[3][0], matrix[3][1], matrix[3][2] ) - pos );
    float squaredDistance;
    for( unsigned int i=0; i<_treeMatrix.size(); i++ )
    {
        matrix = _treeMatrix[i];
        squaredDistance = getSquaredLength( Vector3f( matrix[3][0], matrix[3][1], matrix[3][2] ) - pos );
        if( result > squaredDistance ) result = squaredDistance;
    }
    return sqrt( result );
}

Forest::Forest(Actor* parent, ForestDesc* desc) : Actor( parent )
{
    assert( desc );
    assert( desc->surface );
    assert( desc->assetName.length() );
    assert( desc->cache.length() );

    // copy descriptor
    _desc = *desc;    

    // load asset
    _asset = Gameplay::iEngine->createAsset( engine::atBinary, _desc.assetName.c_str() );

    // enumerate clumps
    callback::ClumpL clumps;    
    _asset->forAllClumps( callback::enumerateClumps, &clumps );
    assert( clumps.size() );

    // fill batch schemes
    Matrix4f clumpM;
    MatrixConversion trunkConversion;
    MatrixConversion canopyConversion;
    _canopyScheme.numLods = _trunkScheme.numLods = clumps.size();
    for( callback::ClumpI clumpI = clumps.begin(); clumpI != clumps.end(); clumpI++ )
    {
        // determine lod Id and check for consistency
        unsigned int lodId = getClumpLodId( *clumpI );
        if( _trunkScheme.lodGeometry[lodId] )
        {
            throw Exception( "Clump \"%s\" is a duplication of existing LOD!", (*clumpI)->getName() );
        }
        // fill schemes
        engine::IAtomic* trunkAtomic     = Gameplay::iEngine->getAtomic( *clumpI, Gameplay::iEngine->findFrame( (*clumpI)->getFrame(), "Trunk" ) ); assert( trunkAtomic );
        engine::IAtomic* canopyAtomic    = Gameplay::iEngine->getAtomic( *clumpI, Gameplay::iEngine->findFrame( (*clumpI)->getFrame(), "Canopy" ) ); assert( canopyAtomic );
        _trunkScheme.lodGeometry[lodId]  = trunkAtomic->getGeometry();        
        _canopyScheme.lodGeometry[lodId] = canopyAtomic->getGeometry();
        _trunkScheme.lodDistance[lodId] = _canopyScheme.lodDistance[lodId] = _desc.lodDistance[lodId];
        // calculate conversions for nearest LOD
        if( lodId == 0 )
        {
            clumpM = (*clumpI)->getFrame()->getLTM();
            trunkConversion.setup( clumpM, trunkAtomic->getFrame()->getLTM() );
            canopyConversion.setup( clumpM, canopyAtomic->getFrame()->getLTM() );
        }
    }

    _trunkScheme.flags = _canopyScheme.flags = 0;

    // check schemes
    assert( _trunkScheme.isValid() );
    assert( _canopyScheme.isValid() );

    // create full cache names
    std::string instanceCache  = _desc.cache; instanceCache  += ".matrices";
    std::string trunkBspCache  = _desc.cache; trunkBspCache  += ".trunk";
    std::string canopyBspCache = _desc.cache; canopyBspCache += ".canopy";

    // try to load forest from cache
    IResource* resource = getCore()->getResource( instanceCache.c_str(), "rb" );
    if( resource )
    {
        unsigned int numTrees;
        fread( &numTrees, sizeof(unsigned int), 1, resource->getFile() );
        _treeMatrix.resize( numTrees );
        fread( &_treeMatrix[0], sizeof(Matrix4f), numTrees, resource->getFile() );
        resource->release();
    }
    else
    {
        // obtain surface properties
        Matrix4f ltm = _desc.surface->getFrame()->getLTM();
        engine::IGeometry* geometry = _desc.surface->getGeometry();
        engine::Mesh* mesh = geometry->createMesh();

        float preservedDistance = _desc.collScale * ( geometry->getAABBSup() - geometry->getAABBInf() ).length();

        // iterate surface triangles
        Vector3f vertex[3];
        Vector3f edge[3];
        Vector3f edgeN[2];
        Vector3f normal;
        Vector3f pos;
        float cosA, sinA, angle, square, probability, scale;
        unsigned int i,j,numTreesInTriangle;
        Matrix4f instanceM;    
        for( i=0; i<mesh->numTriangles; i++ )
        {
            // transform triangle vertices to world space
            vertex[0] = Gameplay::iEngine->transformCoord( mesh->vertices[mesh->triangles[i].vertexId[0]], ltm );
            vertex[1] = Gameplay::iEngine->transformCoord( mesh->vertices[mesh->triangles[i].vertexId[1]], ltm );
            vertex[2] = Gameplay::iEngine->transformCoord( mesh->vertices[mesh->triangles[i].vertexId[2]], ltm );
            // calculate triangle square value...
            edge[0] = vertex[1] - vertex[0];
            edge[1] = vertex[2] - vertex[0];
            edge[2] = vertex[2] - vertex[1];
            edgeN[0] = edge[0]; edgeN[0].normalize();
            edgeN[1] = edge[1]; edgeN[1].normalize();
            normal.cross( edgeN[0], edgeN[1] );
            cosA = Vector3f::dot( edgeN[0], edgeN[1] );
            if( cosA > 1.0f ) angle = 0.0f; 
            else if( cosA < -1.0f ) angle = 180; 
            else angle = acosf( cosA );
            sinA = sin( angle );
            square = 0.5f * edge[0].length() * edge[1].length() * sinA / 10000.0f;
            // obtain number of particles in this triangle
            numTreesInTriangle = unsigned int( square * _desc.density );
            if( !numTreesInTriangle )
            {
                // include probability method to decide to place grass on to this triangle
                probability = square / ( 1 / _desc.density );
                assert( probability <= 1.0f );
                if( probability > 0 && getCore()->getRandToolkit()->getUniform( 0, 1 ) <= probability ) numTreesInTriangle++;
            }
            // generate trees
            for( j=0; j<numTreesInTriangle; j++ )
            {
                // generate scale
                scale = getCore()->getRandToolkit()->getUniform( _desc.minScale, _desc.maxScale );
                // generate coordinate
                pos = generateRandomPosition( vertex, edge );                
                // generate matrix
                instanceM.set(
                    clumpM[0][0] * scale, clumpM[0][1] * scale, clumpM[0][2] * scale, 0.0f,
                    clumpM[1][0] * scale, clumpM[1][1] * scale, clumpM[1][2] * scale, 0.0f,
                    clumpM[2][0] * scale, clumpM[2][1] * scale, clumpM[2][2] * scale, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f
                );
                instanceM = Gameplay::iEngine->rotateMatrix( instanceM, Vector3f(0,1,0), getCore()->getRandToolkit()->getUniform(0,360) );
                instanceM[3][0] = pos[0];
                instanceM[3][1] = pos[1];
                instanceM[3][2] = pos[2];
                _treeMatrix.push_back( instanceM );
            }
            Scene::progressCallback( 
                Gameplay::iLanguage->getUnicodeString(844),
                float( i ) / float( mesh->numTriangles ),
                getScene()
            );
        }

        // release temporary resources
        Gameplay::iEngine->releaseMesh( mesh );

        // write solution
        ccor::IResource* resource = getCore()->getResource( instanceCache.c_str(), "wb" );
        unsigned int numTrees = _treeMatrix.size();
        fwrite( &numTrees, sizeof(unsigned int), 1, resource->getFile() );
        fwrite( &_treeMatrix[0], sizeof(Matrix4f), numTrees, resource->getFile() );
        resource->release();
    }
    
    // build batches
    _trunkBatch  = Gameplay::iEngine->createBatch( _treeMatrix.size(), &_trunkScheme ); assert( _trunkBatch );
    _canopyBatch = Gameplay::iEngine->createBatch( _treeMatrix.size(), &_canopyScheme ); assert( _canopyBatch );
    // fill batches
    for( unsigned int i=0; i<_treeMatrix.size(); i++ )
    {
        _trunkBatch->setMatrix( i, trunkConversion.convert( _treeMatrix[i] ) );
        _canopyBatch->setMatrix( i, canopyConversion.convert( _treeMatrix[i] ) );
    }

    // create spatial acceleration structures BSP trees
    _trunkBatch->createBatchTree( _desc.bspLeafSize, trunkBspCache.c_str() );
    _canopyBatch->createBatchTree( _desc.bspLeafSize, canopyBspCache.c_str() );
    
    // add batches to scene
    getScene()->getStage()->add( _trunkBatch );
    getScene()->getStage()->add( _canopyBatch );

    // add layers to scene
    if( _desc.layers )
    {
        getScene()->getStage()->add( _desc.layers );
    }

    // set no sound
    _rustleSound = _squeakSound = NULL;
}

Forest::~Forest()
{
    // remove layers
    if( _desc.layers )
    {
        getScene()->getStage()->remove( _desc.layers );
    }

    // release batches    
    _scene->getStage()->remove( _trunkBatch );
    _scene->getStage()->remove( _canopyBatch );
    _trunkBatch->release();
    _canopyBatch->release();    
}

void Forest::onUpdateActivity(float dt)
{    
    if( _rustleSound && !_rustleSound->isPlaying() )
    {
        _rustleSound->release();
        _rustleSound = NULL;
    }
    if( _squeakSound && !_squeakSound->isPlaying() )
    {
        _squeakSound->release();
        _squeakSound = NULL;
    }
}

void Forest::onUpdatePhysics(void)
{
}

void Forest::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_FOREST_ENUMERATE )
    {
        std::vector<Forest*>* forests = reinterpret_cast<std::vector<Forest*>*>( eventData );
        forests->push_back( this );
    }
    else if( eventId == 0xFABCCBAF )
    {
        //debugRender();
    }
}

void Forest::debugRender(void)
{
    for( unsigned int i=0; i<_debugBoxes.size(); i++ )
    {
        engine::BoundingBox obb;
        obb.inf = wrap( _debugBoxes[i].extents * -1 );
        obb.sup = wrap( _debugBoxes[i].extents );
        NxMat34 transformation;
        transformation.M = _debugBoxes[i].rot;
        transformation.t = _debugBoxes[i].center;
        obb.matrix = wrap( transformation );

        Gameplay::iEngine->renderOBB( obb, Vector4f( 1,1,1,1 ) );
    }
}

/**
 * audible methods
 */

void Forest::playRustleSound(const Vector3f& pos)
{
    if( !_rustleSound )
    {
        const char* rustleSounds[4] =
        {
            "./res/sounds/wood/rustle01.ogg",
            "./res/sounds/wood/rustle02.ogg",
            "./res/sounds/wood/rustle03.ogg",
            "./res/sounds/wood/rustle04.ogg"
        };
        _rustleSound = Gameplay::iAudio->createStaticSound( rustleSounds[ getCore()->getRandToolkit()->getUniformInt() % 4 ] ); 
        _rustleSound->setDistanceModel( 1000.0f, 25000.0f, 1.0f );
        database::LocationInfo::Reverberation* reverb = getScene()->getReverberation();
        if( reverb )
        {
            _rustleSound->setReverberation(
                reverb->inGain,
                reverb->reverbMixDB,
                reverb->reverbTime,
                reverb->hfTimeRatio
            );
        }            
        _rustleSound->play();
        _rustleSound->place( pos );
    }
    else
    {
        _rustleSound->place( pos );
    }
}

void Forest::playSqueakSound(const Vector3f& pos)
{
    if( !_squeakSound )
    {
        const char* squeakSounds[2] =
        {
            "./res/sounds/wood/squeak01.ogg",
            "./res/sounds/wood/squeak02.ogg"
        };
        _squeakSound = Gameplay::iAudio->createStaticSound( squeakSounds[ getCore()->getRandToolkit()->getUniformInt() % 2 ] ); 
        _squeakSound->setDistanceModel( 1000.0f, 25000.0f, 1.0f );
        database::LocationInfo::Reverberation* reverb = getScene()->getReverberation();
        if( reverb )
        {
            _squeakSound->setReverberation(
                reverb->inGain,
                reverb->reverbMixDB,
                reverb->reverbTime,
                reverb->hfTimeRatio
            );
        }            
        _squeakSound->play();
        _squeakSound->place( pos );
    }
    else
    {
        _squeakSound->place( pos );
    }
}

/**
 * interaction with jumper
 */

static NxBox calculateOBB(engine::IGeometry* geometry, const Matrix4f& matrix, float scale)
{
    assert( geometry );
    Vector3f aabbInf = geometry->getAABBInf();
    Vector3f aabbSup = geometry->getAABBSup();    
    Vector3f right( matrix[0][0], matrix[0][1], matrix[0][2] );
    Vector3f up( matrix[1][0], matrix[1][1], matrix[1][2] );
    Vector3f at( matrix[2][0], matrix[2][1], matrix[2][2] );
    Vector3f pos( matrix[3][0], matrix[3][1], matrix[3][2] );    
    aabbInf[0] *= right.length(), aabbSup[0] *= right.length();
    aabbInf[1] *= up.length(), aabbSup[1] *= up.length();
    aabbInf[2] *= at.length(), aabbSup[2] *= at.length();
    right.normalize();
    up.normalize();
    at.normalize();
    Vector3f aabbCenter = aabbInf + ( aabbSup - aabbInf ) * 0.5f;
    aabbInf *= scale;
    aabbSup *= scale;
    NxMat34 transformation = wrap( Matrix4f(
        right[0], right[1], right[2], 0.0f,
        up[0], up[1], up[2], 0.0f,
        at[0], at[1], at[2], 0.0f,
        pos[0], pos[1], pos[2], 1.0f
    ) );
    NxBox result;
    result.center  = wrap( pos ) + wrap( aabbCenter );
    result.extents = wrap( ( aabbSup - aabbInf ) * 0.5f );
    result.rot = transformation.M;
    return result; 
}

static void calculateIntersectionDetails(NxBox& oob1, NxBox& oob2, float& outVolume2, NxVec3& outGlobalIntersectionCenter2)
{
    float step = 0.5f;
    if( step > oob2.extents.x * 2 ) step = oob2.extents.x * 2;
    if( step > oob2.extents.y * 2 ) step = oob2.extents.y * 2;
    if( step > oob2.extents.z * 2 ) step = oob2.extents.z * 2;

    float x = -oob2.extents.x;
    float y = -oob2.extents.y;
    float z = -oob2.extents.z;

    NxVec3 localPoint;
    NxVec3 globalPoint;
    NxVec3 localIntersectionCenter( 0,0,0 );
    unsigned int numTotalPoints = 0;
    unsigned int numIntersectedPoints = 0;

    NxMat34 matrix2;
    matrix2.M = oob2.rot;
    matrix2.t = oob2.center;

    while( x <= oob2.extents.x )
    {
        while( y <= oob2.extents.y )
        {
            while( z <= oob2.extents.z )
            {
                localPoint.set( x,y,z );
                globalPoint = matrix2 * localPoint;
                if( NxBoxContainsPoint( oob1, globalPoint ) )
                {
                    if( numIntersectedPoints )
                    {
                        localIntersectionCenter = localPoint;
                    }
                    else
                    {
                        localIntersectionCenter += localPoint;
                    }
                    numIntersectedPoints++;
                }
                numTotalPoints++;
                z += step;
            }
            z = -oob2.extents.z;
            y += step;
        }
        y = -oob2.extents.y;
        x += step;
    }

    outVolume2 = float( numIntersectedPoints ) / float( numTotalPoints );
        
    if( numIntersectedPoints )
    {
        localIntersectionCenter = localIntersectionCenter / float( numIntersectedPoints );
    }   
    outGlobalIntersectionCenter2 = matrix2 * localIntersectionCenter;

    // getCore()->logMessage( "oobb intersection volume: %3.2f (%d/%d) step: %3.2f", outVolume2, numIntersectedPoints, numTotalPoints, step );
}

unsigned int Forest::onCollideJumper(unsigned int id, Matrix4f* matrix, void* data)
{
    Forest* __this = reinterpret_cast<Forest*>( data );

    // determine obb of instance
    NxBox instanceOBB = calculateOBB( 
        __this->_canopyBatch->getBatchScheme()->lodGeometry[0], 
        *matrix,
        __this->_desc.collScale
    );
    __this->_debugBoxes.push_back( instanceOBB );

    // collide obbs
    if( NxBoxBoxIntersect( instanceOBB, __this->_jumperOBB ) )
    {
        Jumper* jumper = dynamic_cast<Jumper*>( __this->_currentJumper );
        // add impulse to jumper body
        NxVec3 linearVelocity = __this->_currentJumperActor->getLinearVelocity();
        NxVec3 impulse = linearVelocity * getCore()->getRandToolkit()->getUniform( __this->_desc.minImpulseFactor, __this->_desc.maxImpulseFactor ) * -1;
        NxVec3 localPos(
            __this->_jumperOBB.extents.x * getCore()->getRandToolkit()->getUniform( -1, 1 ),
            __this->_jumperOBB.extents.y * getCore()->getRandToolkit()->getUniform( -1, 1 ),
            __this->_jumperOBB.extents.z * getCore()->getRandToolkit()->getUniform( -1, 1 )
        );
        __this->_currentJumperActor->addForceAtLocalPos( impulse, localPos, NX_IMPULSE );
        // damage jumper
        jumper->damage( __this->_desc.damageFactor * impulse.magnitude(), 0.0f, linearVelocity.magnitude() );
        // play rustle sound
        __this->playRustleSound( __this->_currentJumperCollision->getFrame()->getPos() );
    }

    return id;
}

unsigned int Forest::onCollideCanopy(unsigned int id, Matrix4f* matrix, void* data)
{
    Forest* __this = reinterpret_cast<Forest*>( data );

    // determine obb of instance
    NxBox instanceOBB = calculateOBB( 
        __this->_canopyBatch->getBatchScheme()->lodGeometry[0], 
        *matrix,
        __this->_desc.collScale
    );
    __this->_debugBoxes.push_back( instanceOBB );

    // collide obbs
    if( NxBoxBoxIntersect( instanceOBB, __this->_canopyOBB ) )
    {
        CanopySimulator* canopy = dynamic_cast<CanopySimulator*>( __this->_currentCanopy );
        // calculate intersection details
        float volume;
        NxVec3 globalIntersectionCenter;
        calculateIntersectionDetails( instanceOBB, __this->_canopyOBB, volume, globalIntersectionCenter );
        assert( volume <= 1.0f );
        // add force to canopy
        NxVec3 linearVelocity = __this->_currentCanopyActor->getLinearVelocity();
        float linearVelocityMagnitude = linearVelocity.magnitude();
        linearVelocity.normalize();
        NxVec3 force = linearVelocity * 0.2f * sqr( linearVelocityMagnitude ) * __this->_currentCanopyInfo->square * -volume;
        __this->_currentCanopyActor->addForceAtPos( force, globalIntersectionCenter, NX_FORCE );
        // damage canopy
        canopy->rip( __this->_desc.ripFactor * force.magnitude() );
        // entangle canopy
        if( volume > __this->_desc.entangleFactor )
        {            
            canopy->entangle( globalIntersectionCenter );
            __this->playSqueakSound( wrap( globalIntersectionCenter ) );
        }
        else
        {
            __this->playRustleSound( wrap( globalIntersectionCenter ) );
        }
    }

    return id;
}

void Forest::simulateInteraction(Actor* actor)
{
    _debugBoxes.clear();

    _currentJumper = actor;
    Jumper* jumper = dynamic_cast<Jumper*>( actor );
    if( !jumper ) return;
        
	CanopySimulator* canopy = jumper->getDominantCanopy();

    _currentCanopy = canopy;
    _currentCanopyInfo = canopy->getGearRecord();
	_jumperCanopyIsOpened = canopy->isOpened();
    if( _jumperCanopyIsOpened )
    {
        _currentCanopyCollision = CanopySimulator::getCollisionGeometry( canopy->getClump() );
        _currentCanopyActor = canopy->getNxActor();
    }
    else
    {
        _currentCanopyCollision = NULL;
        _currentCanopyActor = NULL;
    }

    // obtain collision atomic
    _currentJumperCollision = NULL;
    _currentJumperActor     = NULL;
    switch( jumper->getPhase() )
    {
    case ::jpFreeFalling:
        _currentJumperActor = jumper->getFreefallActor();
        _currentJumperCollision = Jumper::getCollisionFF( jumper->getClump() );
        assert( _currentJumperCollision );
        break;
    case ::jpFlight:
        if( _jumperCanopyIsOpened )
        {
            _currentJumperActor = jumper->getFlightActor();
            _currentJumperCollision = Jumper::getCollisionFC( jumper->getClump() );
        }
        else
        {
            _currentJumperActor = jumper->getFreefallActor();
            _currentJumperCollision = Jumper::getCollisionFF( jumper->getClump() );
        }
        assert( _currentJumperCollision );
        break;
    }
    if( _currentJumperCollision )
    {
        // collide jumper with forest
        _jumperOBB = calculateOBB( 
            _currentJumperCollision->getGeometry(),
            _currentJumperCollision->getFrame()->getLTM(),
            1.0f
        );
        _jumperOBB.center = _currentJumperActor->getGlobalPosition(); 
        _debugBoxes.push_back( _jumperOBB );

        float testBoxSize = 250;
        Vector3f jumperPos = wrap( _jumperOBB.center );
        _canopyBatch->forAllInstancesInAABB(
            jumperPos - Vector3f( testBoxSize,testBoxSize,testBoxSize ),
            jumperPos + Vector3f( testBoxSize,testBoxSize,testBoxSize ),
            onCollideJumper,
            this
        );
    }
    if( _currentCanopyCollision && !canopy->isCohesionState() )
    {
        _canopyOBB = calculateOBB(
            _currentCanopyCollision->getGeometry(),
            _currentCanopyCollision->getFrame()->getLTM(),
            1.0f
        );
        _canopyOBB.center = _currentCanopyActor->getGlobalPosition();
        _debugBoxes.push_back( _canopyOBB );

        float testBoxSize = 750;
        Vector3f canopyPos = wrap( _canopyOBB.center );
        _canopyBatch->forAllInstancesInAABB(
            canopyPos - Vector3f( testBoxSize,testBoxSize,testBoxSize ),
            canopyPos + Vector3f( testBoxSize,testBoxSize,testBoxSize ),
            onCollideCanopy,
            this
        );
    }

    _currentJumper = NULL;
    _currentJumperCollision = NULL;
    _currentJumperActor = NULL;
    _jumperCanopyIsOpened = false;
    _currentCanopyCollision = NULL;
    _currentCanopyActor = NULL;
}