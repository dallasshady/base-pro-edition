
#include "headers.h"
#include "canopy.h"
#include "jumper.h"

/**
 * class implementation
 */

CanopySimulator::PAB::PAB(engine::IClump* clump, database::PABDesc* pabDesc)
{
    assert( clump );
    assert( pabDesc );
    _frame = Gameplay::iEngine->findFrame( clump->getFrame(), pabDesc->pabName ); assert( _frame );
    _pabDesc = pabDesc;
    _angle = 0;
    _constraint = NULL;
}

void CanopySimulator::PAB::setupConstraint(unsigned int numPABs, PAB** pabs)
{
    if( !_pabDesc->cstName ) return;
    
    for( unsigned int i=0; i<numPABs; i++ )
    {
        if( this == pabs[i] ) continue;
        if( strcmp( _pabDesc->cstName, pabs[i]->getPABDesc()->pabName ) == 0 )
        {
            _constraint = pabs[i];
            break;
        }
    }
}

void CanopySimulator::PAB::update(float dt)
{
    onUpdate( dt );

    float targetAngle = getTargetAngle();
    if( _constraint ) targetAngle += _constraint->getAngle() * _pabDesc->cstWeight;

    // clamp
    if( targetAngle < -_pabDesc->pabLimit ) targetAngle = -_pabDesc->pabLimit;
    if( targetAngle > _pabDesc->pabLimit ) targetAngle = _pabDesc->pabLimit;

    // update animation angle
    float dir = sgn( targetAngle - _angle );
    _angle += dir * _pabDesc->pabSpeed * dt;
    if( ( dir < 0 && _angle < targetAngle ) ||
        ( dir > 0 && _angle > targetAngle ) ) _angle = targetAngle;

    // update frame    
    Matrix4f matrix = _frame->getMatrix();
    Vector3f pos( matrix[3][0], matrix[3][1], matrix[3][2] );
    matrix[3][0] = matrix[3][1] = matrix[3][2] = 0.0f;
    matrix = Gameplay::iEngine->rotateMatrix( matrix, _pabDesc->pabAxis, _angle );
    matrix[3][0] = pos[0], matrix[3][1] = pos[1], matrix[3][2] = pos[2];
    _frame->setMatrix( matrix );
    _frame->getLTM();
}

/**
 * Flap
 */

float CanopySimulator::Flap::getTargetAngle(void)
{
    return _pabDesc->pabLimit * ( *_deep );
}

void CanopySimulator::Flap::onUpdate(float dt)
{
}

/**
 * Section
 */

static engine::IFrame* getFirstChildCB(engine::IFrame* frame, void* data)
{
    *((engine::IFrame**)(data)) = frame;
    return NULL;
}

float CanopySimulator::Section::getTargetAngle(void)
{   
    if( !_canopySimulator->getActor() ) return 0.0f;

    // transformation in to local coordinate system of canopy simulator    
    NxMat34 pose = _canopySimulator->getActor()->getGlobalPose();
    NxMat34 iPose;
    bool iResult = pose.getInverse( iPose ); assert( iResult );
    Matrix4f iLTM = wrap( iPose );

    // collapse detection frame
    engine::IFrame* detectionFrame = NULL;
    _frame->forAllChildren( getFirstChildCB, &detectionFrame );
    if( !detectionFrame ) detectionFrame = _frame;
        
    float deviance = 0;
    float distance;
    unsigned int numDeviances = 0;

    // determine deviance
    Vector3f localCollapsePos;
    Vector3f localBonePos = Gameplay::iEngine->transformCoord( detectionFrame->getPos(), iLTM );    
    for( unsigned int i=0; i<_canopySimulator->getGearRecord()->Cnum; i++ ) 
    {
        if( _canopySimulator->getCollapseAreas()[i].radius > 0 )
        {
            localCollapsePos = wrap( _canopySimulator->getCollapseAreas()[i].center );
            distance = ( localCollapsePos - localBonePos ).length() / 100.0f;
            if( distance <= _canopySimulator->getCollapseAreas()[i].radius )
            {
                numDeviances++;
                deviance += distance / _canopySimulator->getCollapseAreas()[i].radius;
            }
        }
    }

    // retrieve line over weight
    float lineOver = 0.0f;
    switch( _pabDesc->loSide )
    {
    case database::losLeft:
        lineOver = _canopySimulator->getLeftLOW();
        break;
    case database::losRight:
        lineOver = _canopySimulator->getRightLOW();
        break;
    }

    // blend collapse & lineover animation
    if( numDeviances )
    {
        float collapseAngle = deviance / numDeviances * _pabDesc->pabLimit;
        float lineOverAngle = lineOver * _pabDesc->loWeight * _pabDesc->pabLimit;
        return fabs( collapseAngle ) > fabs( lineOverAngle ) ? collapseAngle : lineOverAngle;
    }

    // lineover animation only
    return lineOver * _pabDesc->loWeight * _pabDesc->pabLimit;
}

void CanopySimulator::Section::onUpdate(float dt)
{
}