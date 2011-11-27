
#include "headers.h"
#include "batch.h"
#include "bsp.h"
#include "asset.h"
#include "camera.h"
#include "collision.h"
#include "wire.h"
#include "gui.h"

/**
 * bachu no sekutoru
 */

Batch::Sector::Sector(IResource* resource)
{
    assert( resource );

    unsigned int opsize = fread( this, 1, sizeof(Sector), resource->getFile() );
    if( opsize != sizeof(Sector) )
    {
        throw Exception( "Batch spatial cache is corrupted!" );
    }

    if( numIndices ) 
    {
        indices = new unsigned int[numIndices];
        opsize = fread( indices, 1, sizeof(unsigned int)*numIndices, resource->getFile() );
        if( opsize != sizeof(unsigned int)*numIndices )
        {
            throw Exception( "Batch spatial cache is corrupted!" );
        }
    }
    else 
    {
        indices = NULL;
    }

    if( unsigned int(left) )
    {
        left = new Sector( resource );
    }
    if( unsigned int(right) )
    {
        right = new Sector( resource );
    }    

    // check for integrity
    if( numIndices ) 
    {
        assert( left == NULL && right == NULL );
    }
    else
    {
        assert( left != NULL || right != NULL );
    }
}

Batch::Sector::Sector(unsigned int leafSize, Geometry* geometryLod0, Matrix* instances, AABB* aabb, DynamicIndices& parentIndices, float treeProgress, float sectorProgress)
{
    boundingBox = *aabb;
    left = right = NULL;

    // local indices
    DynamicIndices dynamicIndices;

    // show progress
    if( Engine::instance->progressCallback )
    {
        Engine::instance->progressCallback(
            Gui::iLanguage->getUnicodeString(7),
            treeProgress,
            Engine::instance->progressCallbackUserData
        );
    }

    // calculate bounding sphere
    Vector corner[8];
    unsigned int i;
    for( i=0; i<8; i++ ) corner[i] = boundingBox.getCorner( i );    
    boundingSphere.calculate( 8, corner );

    // build list of indices inside AABB of this 
    unsigned int instanceId;
    AABB instanceAABB;
    for( DynamicIndex index = parentIndices.begin(); 
                      index != parentIndices.end(); 
                      index++ )
    {
        instanceId = (*index);
        instanceAABB.calculate( geometryLod0->getBoundingBox(), instances+instanceId );
        if( ::intersectionAABBAABB( &boundingBox, &instanceAABB ) )
        {
            dynamicIndices.insert( instanceId );
        }
    }

    // remove this indices from parent indices
    DynamicIndex parentIndex;
    for( index = dynamicIndices.begin(); index != dynamicIndices.end(); index++ )
    {
        parentIndex = parentIndices.find( (*index) );
        assert( parentIndex != parentIndices.end() );
        parentIndices.erase( parentIndex );
    }

    // build subsets
    if( dynamicIndices.size() > leafSize )
    {
        AABB leftAABB;
        AABB rightAABB;
        boundingBox.divide( leftAABB, rightAABB, AABB::maxPlane );
        left = new Sector( leafSize, geometryLod0, instances, &leftAABB, dynamicIndices, treeProgress, sectorProgress * 0.5f );
        if( static_cast<Sector*>(left)->getNumInstancesInHierarchy() == 0 )
        {
            delete left;
            left = NULL;
        }
        right = new Sector( leafSize, geometryLod0, instances, &rightAABB, dynamicIndices, treeProgress + sectorProgress * 0.5f, sectorProgress * 0.5f );
        if( static_cast<Sector*>(right)->getNumInstancesInHierarchy() == 0 )
        {
            delete right;
            right = NULL;
        }        
    }

    if( !left && !right )
    {        
        // build static indices
        numIndices = dynamicIndices.size();
        indices = new unsigned int[numIndices];        
        for( index = dynamicIndices.begin(), i = 0; 
             index != dynamicIndices.end(); 
             index++, i++ )
        {
            indices[i] = *index;
        }
        dynamicIndices.clear();
    }
    else
    {
        numIndices = 0;
        indices = NULL;
    }

    // show progress
    if( Engine::instance->progressCallback )
    {
        Engine::instance->progressCallback(
            Gui::iLanguage->getUnicodeString(7),
            treeProgress + sectorProgress,
            Engine::instance->progressCallbackUserData
        );
    }
}

Batch::Sector::~Sector()
{
    if( numIndices ) delete[] indices;
    if( left ) delete left;
    if( right ) delete right;
}

void Batch::Sector::render(float maxDistance)
{
    if( left == NULL && right == NULL )
    {
        // calculate nearest (rough) distance to this sector
        Vector distance;
        D3DXVec3Subtract( &distance, &boundingSphere.center, &Camera::eyePos );
        float nearestDistance = D3DXVec3Length( &distance ) - boundingSphere.radius;
        
        Color color = yellow;
        // mark too far sectors with red color 
        if( nearestDistance > maxDistance )
        {
            color = red;
        }

        // render AABB
        dxRenderAABB( &boundingBox, &color, NULL );
    }
    else
    {
        if( left ) static_cast<Sector*>(left)->render( maxDistance );
        if( right ) static_cast<Sector*>(right)->render( maxDistance );
    }
}

void Batch::Sector::write(IResource* resource)
{
    fwrite( this, 1, sizeof(Sector), resource->getFile() );
    if( numIndices )
    {
        fwrite( indices, 1, sizeof(unsigned int)*numIndices, resource->getFile() );
    }
    if( left )
    {
        left->write( resource );
    }
    if( right )
    {
        right->write( resource );
    }
}

unsigned int Batch::Sector::getNumInstancesInHierarchy(void)
{
    if( left == NULL && right == NULL ) 
    {
        return numIndices;
    }
    else
    {
        return ( ( left == NULL ) ? 0 : static_cast<Sector*>(left)->getNumInstancesInHierarchy() ) +
               ( ( right == NULL ) ? 0 : static_cast<Sector*>(right)->getNumInstancesInHierarchy() );
    }
}

Batch::Sector* Batch::Sector::createTree(unsigned int leafSize, Geometry* geometryLod0, Matrix* instances, unsigned int numInstances)
{
    DynamicIndices batchIndices;
    for( unsigned int i=0; i<numInstances; i++ ) batchIndices.insert( i );

    AABB batchAABB;
    AABB instanceAABB;
    batchAABB.calculate( geometryLod0->getBoundingBox(), instances );
    for( unsigned int i=1; i<numInstances; i++ )
    {
        instanceAABB.calculate( geometryLod0->getBoundingBox(), instances+i );
        batchAABB.addAABB( &instanceAABB );
    }

    return new Sector( leafSize, geometryLod0, instances, &batchAABB, batchIndices );
}

void Batch::Sector::forAllInstancesInAABB(Geometry* geometryLod0, Matrix* instances, AABB* aabb, engine::IBatchCallback callback, void* data)
{
    if( ::intersectionAABBAABB( &boundingBox, aabb ) )
    {
        if( !left && !right )
        {
            AABB instanceAABB;
            Matrix4f instanceMatrix;
            unsigned int index;
            for( unsigned int i=0; i<numIndices; i++ )
            {
                index = indices[i];
                instanceAABB.calculate( geometryLod0->getBoundingBox(), instances + index );
                if( ::intersectionAABBAABB( &instanceAABB, aabb ) )
                {
                    instanceMatrix = wrap( instances[index] );
                    callback( index, &instanceMatrix, data );
                }
            }
        }
        else
        {
            if( left ) static_cast<Sector*>(left)->forAllInstancesInAABB( geometryLod0, instances, aabb, callback, data );
            if( right ) static_cast<Sector*>(right)->forAllInstancesInAABB( geometryLod0, instances, aabb, callback, data );
        }
    }
}