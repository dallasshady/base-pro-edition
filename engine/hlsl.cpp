
#include "headers.h"
#include "hlsl.h"

/**
 * compilation method
 */

ID3DXEffect* dxCompileEffect(const char* resourceName, const D3DXMACRO* defines)
{
    LPD3DXEFFECT result;
    LPD3DXBUFFER compilationErrors = NULL;

    if( S_OK != D3DXCreateEffectFromFile( 
        iDirect3DDevice,
        resourceName,
        defines,
        NULL,
        0,
        NULL,
        &result, 
        &compilationErrors
    ) )
    {
        const char* text = (const char*)( compilationErrors->GetBufferPointer() );
        MessageBox( 0, text, "dxCompileEffect() failed", MB_OK );
        compilationErrors->Release();
        return NULL;
    }

    return result;
}

ID3DXEffect* dxCompileEffect(const char* effectCode)
{
    LPD3DXEFFECT result;
    LPD3DXBUFFER compilationErrors = NULL;

    if( S_OK != D3DXCreateEffect( 
        iDirect3DDevice,
        effectCode,
        strlen( effectCode ) + 1,
        NULL,
        NULL,
        0,
        NULL,
        &result,
        &compilationErrors        
    ) )
    {
        const char* text = (const char*)( compilationErrors->GetBufferPointer() );
        MessageBox( 0, text, "dxCompileEffect() failed", MB_OK );
        compilationErrors->Release();
        return NULL;
    }

    return result;
}