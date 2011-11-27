
#include "headers.h"
#include "asset.h"

/**
 * right-handed to left-handed matrix conversion
 */

static void convertLTMs(Frame*  frame)
{
	frame->LTM._13 *= -1;
	frame->LTM._23 *= -1;
	frame->LTM._33 *= -1;
	frame->LTM._43 *= -1;

	Vector at;
	D3DXVec3Cross( &at, &dxRight( &frame->LTM ), &dxUp( &frame->LTM ) );	
	frame->LTM._31 = at.x, frame->LTM._32 = at.y, frame->LTM._33 = at.z;

	if( frame->pFrameSibling ) convertLTMs( static_cast<Frame*>( frame->pFrameSibling ) ); 
	if( frame->pFrameFirstChild ) convertLTMs( static_cast<Frame*>( frame->pFrameFirstChild ) );
}

static void convertMatrices(Frame* frame)
{
	// first, update children frames
	if( frame->pFrameFirstChild ) convertMatrices( static_cast<Frame*>( frame->pFrameFirstChild ) );

	// second, update sibling frames
	if( frame->pFrameSibling ) convertMatrices( static_cast<Frame*>( frame->pFrameSibling ) );

	// last, perform transformation matrix conversion	
	if( frame->pParentFrame == NULL )
	{
		frame->TransformationMatrix = frame->LTM;
	}
	else
	{
		// LTM = TM * pLTM
		// LTM * ipLTM = TM * pLTM * ipLTM
		Matrix pLTM = frame->pParentFrame->LTM;
		Matrix ipLTM;
		D3DXMatrixInverse( &ipLTM, NULL, &pLTM );
		dxMultiply( &frame->TransformationMatrix, &frame->LTM, &ipLTM );
	}
}

static void rh2lh(Frame* frame)
{
	// calculate all LTM's in hierarchy
	frame->synchronizeSafe();
	// convert all LTM's from RH to LH
	convertLTMs( frame );
	// calculate transformation matrices from LTMs
	convertMatrices( frame );
}

/**
 * ImportTemplate implementation
 */
  
ImportAsset::ImportAsset(const char* resourcePath)
{
    import::IImport* iImport = NULL;
    queryInterface( "Import", &iImport );

    import::IImportStream* iImportStream = iImport->importRws( resourcePath );
    
    while( iImportStream->getType() != import::itNULL )
    switch( iImportStream->getType() )
    {
    case import::itTexture:
        {
            import::ImportTexture* importData = iImportStream->importTexture();
            // search for texture in current dictionary
            if( Texture::textures.find( importData->name ) == Texture::textures.end() )
            {
                if( strcmp( importData->name, "House01" ) == 0 )
                {
                    int iiii=0;
                }
                // create compatible texture
                Texture* texture = Texture::createDynamicTexture( 
                    importData->width, 
                    importData->height, 
                    importData->depth, 
                    importData->name
                );
                // lock texture top-level surface
                D3DLOCKED_RECT lockedRect;
                texture->iDirect3DTexture()->LockRect( 0, &lockedRect, NULL, 0 );
                // copy pixels
                int offset = 0;
                unsigned char* destPixels = (unsigned char*)(lockedRect.pBits);    
                for( int i=0; i<importData->width*importData->height; i++ )
                {
                    destPixels[offset + 0] = importData->pixels[offset + 2],
                    destPixels[offset + 1] = importData->pixels[offset + 1],
                    destPixels[offset + 2] = importData->pixels[offset + 0],
                    destPixels[offset + 3] = importData->pixels[offset + 3];
                    offset += 4;
                }
                texture->iDirect3DTexture()->UnlockRect( 0 );
                // setup filetering, addressing, etc
                switch( importData->addressMode )
                {
                case import::iamWrap: 
                    texture->setAddressTypeU( engine::atWrap );
                    texture->setAddressTypeV( engine::atWrap );
                    break;
                case import::iamMirror:
                    texture->setAddressTypeU( engine::atMirror );
                    texture->setAddressTypeV( engine::atMirror );
                    break;
                case import::iamClamp:
                    texture->setAddressTypeU( engine::atClamp );
                    texture->setAddressTypeV( engine::atClamp );
                    break;
                case import::iamBorder:
                    texture->setAddressTypeU( engine::atBorder );
                    texture->setAddressTypeV( engine::atBorder );
                    break;
                }
                switch( importData->filterMode )
                {
                case import::ifmNearest:
                    texture->setMagFilter( engine::ftPoint );
                    texture->setMinFilter( engine::ftPoint );
                    texture->setMipFilter( engine::ftNone );
                    break;
                case import::ifmLinear:
                    texture->setMagFilter( engine::ftLinear );
                    texture->setMinFilter( engine::ftLinear );
                    texture->setMipFilter( engine::ftNone );
                    break;
                case import::ifmMipNearest:
                    texture->setMagFilter( engine::ftPoint );
                    texture->setMinFilter( engine::ftPoint );
                    texture->setMipFilter( engine::ftPoint );
                    break;
                case import::ifmMipLinear:
                    texture->setMagFilter( engine::ftPoint );
                    texture->setMinFilter( engine::ftPoint );
                    texture->setMipFilter( engine::ftLinear );
                    break;
                case import::ifmLinearMipNearest:
                    texture->setMagFilter( engine::ftLinear );
                    texture->setMinFilter( engine::ftLinear );
                    texture->setMipFilter( engine::ftPoint );
                    break;
                case import::ifmLinearMipLinear:
                    texture->setMagFilter( engine::ftLinear );
                    texture->setMinFilter( engine::ftLinear );
                    texture->setMipFilter( engine::ftLinear );
                    break;
                }
                // override filtering
                texture->setMagFilter( engine::ftLinear );
                texture->setMinFilter( engine::ftLinear );
                texture->setMipFilter( engine::ftLinear );
                texture->setMaxAnisotropy( 8 );
                /*
                // create normal map
                iImport->createNormalMap(
                    importData->width, 
                    importData->height, 
                    importData->depth, 
                    importData->pixels,
                    importData->stride,
                    true,
                    5.0f
                );
                // create compatible texture
                Texture* normalMap = Texture::createDynamicTexture(
                    importData->width, 
                    importData->height, 
                    importData->depth, 
                    ( std::string( importData->name ) + "_nmap" ).c_str()
                );
                normalMap->setMagFilter( engine::ftLinear );
                normalMap->setMinFilter( engine::ftLinear );
                normalMap->setMipFilter( engine::ftLinear );
                // lock texture top-level surface
                lockedRect;
                normalMap->iDirect3DTexture()->LockRect( 0, &lockedRect, NULL, 0 );
                // copy pixels
                offset = 0;
                destPixels = (unsigned char*)(lockedRect.pBits);    
                for( i=0; i<importData->width*importData->height; i++ )
                {
                    destPixels[offset + 0] = importData->pixels[offset + 2],
                    destPixels[offset + 1] = importData->pixels[offset + 1],
                    destPixels[offset + 2] = importData->pixels[offset + 0],
                    destPixels[offset + 3] = importData->pixels[offset + 3];
                    offset += 4;
                }
                normalMap->iDirect3DTexture()->UnlockRect( 0 );
                */
                // put texture into importer storage
                _textures.insert( TextureT( importData->id, texture ) );
            }
            // release import data
            iImport->release( importData );
        }
        break;
    case import::itMaterial:
        {
            import::ImportMaterial* importData = iImportStream->importMaterial();

            Shader* shader = NULL;
            
            if( importData->textureId == 0 && importData->dualPassTextureId == 0 )
            {
                shader = new Shader( 0, importData->name );
            }
            else if( importData->textureId != 0 && importData->dualPassTextureId == 0 )
            {
                shader = new Shader( 1, importData->name );
                TextureI textureI = _textures.find( importData->textureId );
                if( textureI != _textures.end() ) 
                {
                    shader->setLayerTexture( 0, textureI->second );
                }
                shader->setLayerUV( 0,0 );
                /*
                ::TextureI normalMapI = Texture::textures.find( ( std::string( textureI->second->getName() ) + "_nmap" ).c_str() );
                if( normalMapI != Texture::textures.end() )
                {
                    shader->setNormalMap( normalMapI->second );
                    shader->setNormalMapUV( 0 );
                }
                */
            }
            else
            {
                shader = new Shader( 2, importData->name );
                TextureI textureI = _textures.find( importData->textureId );
                if( textureI != _textures.end() ) 
                {
                    shader->setLayerTexture( 0, textureI->second );
                }
                textureI = _textures.find( importData->dualPassTextureId );
                if( textureI != _textures.end() ) 
                {
                    shader->setLayerTexture( 1, textureI->second );
                }
                shader->setLayerUV( 1,1 );
                switch( importData->dualPassBlendType )
                {
                case import::ImportMaterial::btAdd:
                    shader->setLayerBlending( 1, engine::btAdd );
                    break;
                case import::ImportMaterial::btModulate:
                    shader->setLayerBlending( 1, engine::btModulate );
                    break;
                case import::ImportMaterial::btBlendTextureAlpha:
                    shader->setLayerBlending( 1, engine::btBlendTextureAlpha );
                    break;
                default:
                    shader->setLayerBlending( 1, engine::btOver );
                }

                textureI = _textures.find( importData->textureId );
                assert( textureI != _textures.end() );
                ::TextureI normalMapI = Texture::textures.find( ( std::string( textureI->second->getName() ) + "_nmap" ).c_str() );
                if( normalMapI != Texture::textures.end() )
                {
                    shader->setNormalMap( normalMapI->second );
                    shader->setNormalMapUV( 0 );
                }
            }

            shader->setDiffuseColor( importData->color );
            // put texture into importer storage
            _shaders.insert( ShaderT( importData->id, shader ) );
            // release import data
            iImport->release( importData );
        }
        break;
    case import::itFrame:
        {
            import::ImportFrame* importData = iImportStream->importFrame();

            Frame* frame = new Frame( importData->name );
            frame->TransformationMatrix = wrap( importData->modeling );	
			
            FrameI parentI = _frames.find( importData->parentId );
            if( parentI != _frames.end() ) 
            {
                frame->setParent( parentI->second );
            }
            _frames.insert( FrameT( importData->id, frame ) );
            iImport->release( importData );
        }
        break;
    case import::itGeometry:
        {
            import::ImportGeometry* importData = iImportStream->importGeometry();

            Geometry* geometry = new Geometry(
                importData->numVertices,
                importData->numTriangles,
                importData->numUVs,
                importData->numMaterials,
                ( importData->prelights != NULL ) ? 1 : 0,
                false,
                importData->name
            );

            for( int i=0; i<importData->numVertices; i++ )
            {
                geometry->getVertices()[i] = wrap( importData->vertices[i] );				
                geometry->getNormals()[i] = wrap( importData->normals[i] );
				// rh2lh conversion
                /*
				geometry->getVertices()[i][2] *= -1;
				geometry->getNormals()[i][2] *= -1;
                */
                geometry->getUVSet(0)[i] = wrap( importData->uvs[0][i] );
                if( importData->numUVs > 1 ) 
                {
                    geometry->getUVSet(1)[i] = wrap( importData->uvs[1][i] );
                }
                if( importData->prelights )
                {
                    geometry->getPrelights(0)[i] = wrap( importData->prelights[i] );
                }
            }
            for( i=0; i<importData->numTriangles; i++ )
            {
                /*
                geometry->getTriangles()[i].set(
                    importData->triangles[i].vertexId[0],
					importData->triangles[i].vertexId[2],
					importData->triangles[i].vertexId[1],					
                    importData->triangles[i].materialId
                );
                */
                geometry->getTriangles()[i].set(
                    importData->triangles[i].vertexId[0],
                    importData->triangles[i].vertexId[1],
					importData->triangles[i].vertexId[2],					
                    importData->triangles[i].materialId
                );
            }
            for( i=0; i<importData->numMaterials; i++ )
            {
                ShaderI shaderI = _shaders.find( importData->materials[i] );
                assert( shaderI != _shaders.end() );
                geometry->setShader( i, shaderI->second );
            }
            geometry->instance();
            _geometries.insert( GeometryT( importData->id, geometry ) );
            iImport->release( importData );
        }
        break;
    case import::itAtomic:
        {
            import::ImportAtomic* importData = iImportStream->importAtomic();
            Atomic* atomic = new Atomic;
            FrameI frameI = _frames.find( importData->frameId );
            assert( frameI != _frames.end() );
            atomic->setFrame( frameI->second );
            GeometryI geometryI = _geometries.find( importData->geometryId );
            assert( geometryI != _geometries.end() );
            atomic->setGeometry( geometryI->second );            
            TextureI textureI = _textures.find( importData->lightmapId );
            if( textureI != _textures.end() ) 
            {
                atomic->setLightMap( textureI->second );
            }
            _atomics.insert( AtomicT( importData->id, atomic ) );
            iImport->release( importData );
        }
        break;
    case import::itClump:
        {
            import::ImportClump* importData = iImportStream->importClump();
            Clump* clump = new Clump( importData->name );
            FrameI frameI = _frames.find( importData->frameId );
            assert( frameI != _frames.end() );
            clump->setFrame( frameI->second );
            frameI->second->dirty();
            for( int i=0; i<importData->numAtomics; i++ )
            {
                AtomicI atomicI = _atomics.find( importData->atomics[i] );
                assert( atomicI != _atomics.end() );
                clump->add( atomicI->second );
            }
            for( i=0; i<importData->numLights; i++ )
            {
                LightI lightI = _lights.find( importData->lights[i] );
                assert( lightI != _lights.end() );
                clump->add( lightI->second );
            }
            iImport->release( importData );
            /*
			rh2lh( clump->frame() );
            */
            _clumps.push_back( clump );
        }
        break;
    case import::itWorldSector:
        {
            import::ImportWorldSector* importData = iImportStream->importWorldSector();
            
            BSPI bspI = _bspM.find( importData->worldId ); assert( bspI != _bspM.end() );
            BSPSectorI parentSectorI = _bspSectorM.find( importData->parentId );
            BSPSector* parentSector = NULL;
            if( parentSectorI != _bspSectorM.end() ) parentSector = parentSectorI->second;
            AABB boundingBox;
            boundingBox.inf = wrap( importData->aabbInf );
            boundingBox.sup = wrap( importData->aabbSup );
			// rh2lh conversion
            /*
			float temp = boundingBox.inf[2] * -1;
			boundingBox.inf[2] = boundingBox.sup[2] * -1;
			boundingBox.sup[2] = temp;
            */

            int numPrelights = 0; if( importData->prelights ) numPrelights++;

            Geometry* geometry = NULL;

            if( importData->numVertices && importData->numTriangles )
            {
                geometry = new Geometry( 
                    importData->numVertices, 
                    importData->numTriangles, 
                    importData->numUVs, 
                    bspI->second->getNumShaders(),
                    numPrelights,
                    true,
                    strformat( "BSPSector_%x_Shape", importData->id ).c_str()
                );

                geometry->setShaders( bspI->second->getShaders() );

                for( int i=0; i<importData->numVertices; i++ )
                {
                    geometry->getVertices()[i] = wrap( importData->vertices[i] );
                    geometry->getNormals()[i] = wrap( importData->normals[i] );
					// rh2lh conversion
                    /*
					geometry->getVertices()[i][2] *= -1;
                    geometry->getNormals()[i][2] *= -1;
                    */
                    for( int j=0; j<importData->numUVs; j++ ) geometry->getUVSet(j)[i] = wrap( importData->uvs[j][i] );
                    if( numPrelights ) geometry->getPrelights(0)[i] = wrap( importData->prelights[i] );
                }
                for( i=0; i<importData->numTriangles; i++ )
                {
                    /*
                    geometry->getTriangles()[i].set(
                        importData->triangles[i].vertexId[0],
                        importData->triangles[i].vertexId[2],
						importData->triangles[i].vertexId[1],
                        importData->triangles[i].materialId
                    );
                    */
                    geometry->getTriangles()[i].set(
                        importData->triangles[i].vertexId[0],
                        importData->triangles[i].vertexId[1],
                        importData->triangles[i].vertexId[2],
                        importData->triangles[i].materialId
                    );
                }
                geometry->instance();
            }
            BSPSector* sector = new BSPSector( bspI->second, parentSector, boundingBox, geometry );

            TextureI textureI = _textures.find( importData->lightmapId );
            if( textureI != _textures.end() ) 
            {
                sector->setLightMap( textureI->second );
            }

            _bspSectorM.insert( BSPSectorT( importData->id, sector ) );
            iImport->release( importData );
        };
        break;
    case import::itWorld:
        {
            import::ImportWorld* importData = iImportStream->importWorld();
			AABB boundingBox; 
			boundingBox.inf = wrap( importData->aabbInf );
			boundingBox.sup = wrap( importData->aabbSup );			
			// rh2lh conversion
            /*
			float temp = boundingBox.inf[2] * -1;
			boundingBox.inf[2] = boundingBox.sup[2] * -1;
			boundingBox.sup[2] = temp;
            */

            BSP* bsp = new BSP( 
                importData->name,
                boundingBox, 
                importData->numMaterials 
            );
            for( int i=0; i<importData->numMaterials; i++ )
            {
                ShaderI shaderI = _shaders.find( importData->materials[i] );
                assert( shaderI != _shaders.end() );
                bsp->setShader( i, shaderI->second );
            }
            _bspM.insert( BSPT( importData->id, bsp ) );
            _bsps.push_back( bsp );
            iImport->release( importData );
        };
        break;
    case import::itLight:
        {
            import::ImportLight* importData = iImportStream->importLight();

            engine::LightType lightType;
            switch( importData->type )
            {
            case import::ImportLight::ltAmbient:
                lightType = engine::ltAmbient;
                break;
            case import::ImportLight::ltDirectional:
                lightType = engine::ltDirectional;
                break;
            case import::ImportLight::ltPoint:
                lightType = engine::ltPoint;
                break;
            case import::ImportLight::ltSpot:
                lightType = engine::ltSpot;
                break;
            default:
                assert( !"shouldn't be here!" );
            }

            Light* light = new Light( lightType );
            light->setRange( importData->radius );
            light->setDiffuseColor( importData->color );
            light->setSpecularColor( importData->color );
            light->setPhi( importData->coneAngle );
            light->setTheta( importData->coneAngle );
            light->setAttenuation( Vector3f( 0,0.0001f,0 ) );
            
            FrameI frameI = _frames.find( importData->frameId );
            assert( frameI != _frames.end() );
            light->setFrame( frameI->second );

            _lights.insert( LightT( importData->id, light ) );

            iImport->release( importData );
        }
        break;
    default:
        assert( !"shouldn't be here!" );
    }

    iImport->release( iImportStream );
}

ImportAsset::~ImportAsset(void)
{
}

void ImportAsset::release(void)
{
    delete this;
}

void ImportAsset::serialize(void)
{
    assert( !"method is not supported for this class!" );
}