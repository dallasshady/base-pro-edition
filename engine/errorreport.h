/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description engine error report tools
 *
 * @author bad3p
 */

#ifndef ERROR_REPORT_INCLUDED
#define ERROR_REPORT_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"

/**
 * DirectX Graphics errors
 */

static void dxReportResult(const char* file, int line, HRESULT result)
{
    const char* description = "Unknown";

    switch( result )
    {
    case D3DOK_NOAUTOGEN: 
        description = "The autogeneration of mipmaps is not supported for this format. This means that resource creation will succeed but the mipmap levels will not be automatically generated."; 
        break;
    case D3DERR_CONFLICTINGRENDERSTATE: 
        description = "The currently set render states cannot be used together.";
        break;
    case D3DERR_CONFLICTINGTEXTUREFILTER:
        description = "The current texture filters cannot be used together.";
        break;
    case D3DERR_CONFLICTINGTEXTUREPALETTE:
        description = "The current textures cannot be used simultaneously.";
        break;
    case D3DERR_DEVICELOST:
        description = "The device has been lost but cannot be reset at this time. Therefore, rendering is not possible.";
        break;
    case D3DERR_DEVICENOTRESET:
        description = "The device has been lost but can be reset at this time.";
        break;
    case D3DERR_DRIVERINTERNALERROR:
        description = "Internal driver error. Applications should generally shut down when receiving this error.";
        break;
    case D3DERR_INVALIDCALL:
        description = "The method call is invalid. For example, a method's parameter may not be a valid pointer.";
        break;
    case D3DERR_INVALIDDEVICE:
        description = "The requested device type is not valid.";
        break;
    case D3DERR_MOREDATA:
        description = "There is more data available than the specified buffer size can hold.";
        break;
    case D3DERR_NOTAVAILABLE:
        description = "This device does not support the queried technique.";
        break;
    case D3DERR_NOTFOUND:
        description = "The requested item was not found.";
        break;
    case D3DERR_OUTOFVIDEOMEMORY:
        description = "Direct3D does not have enough display memory to perform the operation.";
        break;
    case D3DERR_TOOMANYOPERATIONS:
        description = "The application is requesting more texture-filtering operations than the device supports.";
        break;
    case D3DERR_UNSUPPORTEDALPHAARG:
        description = "The device does not support a specified texture-blending argument for the alpha channel.";
        break;
    case D3DERR_UNSUPPORTEDALPHAOPERATION:
        description = "The device does not support a specified texture-blending operation for the alpha channel.";
        break;
    case D3DERR_UNSUPPORTEDCOLORARG:
        description = "The device does not support a specified texture-blending argument for color values.";
        break;
    case D3DERR_UNSUPPORTEDCOLOROPERATION:
        description = "The device does not support a specified texture-blending operation for color values.";
        break;
    case D3DERR_UNSUPPORTEDFACTORVALUE:
        description = "The device does not support the specified texture factor value.";
        break;
    case D3DERR_UNSUPPORTEDTEXTUREFILTER:
        description = "The device does not support the specified texture filter.";
        break;
    case D3DERR_WRONGTEXTUREFORMAT:
        description = "The pixel format of the texture surface is not valid.";
        break;
    case D3DXERR_INVALIDDATA:
        description = "The data is invalid.";
        break;
    case D3DXERR_CANNOTATTRSORT:
        description = "Attribute sort (D3DXMESHOPT_ATTRSORT) is not supported as an optimization technique.";
        break;
    case D3DXERR_CANNOTMODIFYINDEXBUFFER:
        description = "The index buffer cannot be modified.";
        break;
    case D3DXERR_SKINNINGNOTSUPPORTED:
        description = "Skinning is not supported.";
        break;
    case D3DXERR_TOOMANYINFLUENCES:
        description = "Too many influences specified.";
        break;
    case D3DXERR_LOADEDMESHASNODATA:
        description = "The mesh has no data.";
        break;
    case D3DXERR_INVALIDMESH:
        description = "The mesh is invalid.";
        break;
    case E_FAIL:
        description = "An undetermined error occurred inside the Direct3D subsystem.";
        break;
    case E_INVALIDARG:
        description = "An invalid parameter was passed to the returning function.";
        break;
    case E_OUTOFMEMORY:
        description = "Direct3D could not allocate sufficient memory to complete the call.";
        break;
    }

    throw ccor::Exception( "DirectX interface method was failed with result %x\n\nDescription: \"%s\"\nFile: \"%s\" line %d", result, description, file, line );
}

static inline void dxCheckResult(const char* file, int line, HRESULT result)
{
    if( result != D3D_OK ) dxReportResult( file, line, result );
}

//#define _dxCR(HRESULT) dxCheckResult( __FILE__, __LINE__, HRESULT )
#define _dxCR(HRESULT) HRESULT

#endif