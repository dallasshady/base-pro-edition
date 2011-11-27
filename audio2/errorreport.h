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

static void dsReportResult(const char* file, int line, HRESULT result)
{
    const char* description = "Unknown";

    switch( result )
    {
    case DS_NO_VIRTUALIZATION: 
        description = "The buffer was created, but another 3-D algorithm was substituted."; 
        break;
    case DSERR_ACCESSDENIED: 
        description = "The request failed because access was denied.";
        break;
    case DSERR_ALLOCATED:
        description = "The request failed because resources, such as a priority level, were already in use by another caller.";
        break;
    case DSERR_ALREADYINITIALIZED:
        description = "The object is already initialized.";
        break;
    case DSERR_BADFORMAT:
        description = "The specified wave format is not supported.";
        break;
    case DSERR_BADSENDBUFFERGUID:
        description = "The GUID specified in an audiopath file does not match a valid mix-in buffer.";
        break;
    case DSERR_BUFFERLOST:
        description = "The buffer memory has been lost and must be restored.";
        break;
    case DSERR_BUFFERTOOSMALL:
        description = "The buffer size is not great enough to enable effects processing.";
        break;
    case DSERR_CONTROLUNAVAIL:
        description = "The buffer control (volume, pan, and so on) requested by the caller is not available. Controls must be specified when the buffer is created, using the dwFlags member of DSBUFFERDESC.";
        break;
    case DSERR_DS8_REQUIRED:
        description = "A DirectSound object of class CLSID_DirectSound8 or later is required for the requested functionality. For more information, see IDirectSound8 Interface.";
        break;
    case DSERR_FXUNAVAILABLE:
        description = "The effects requested could not be found on the system, or they are in the wrong order or in the wrong location; for example, an effect expected in hardware was found in software.";
        break;
    case DSERR_GENERIC:
        description = "An undetermined error occurred inside the DirectSound subsystem.";
        break;
    case DSERR_INVALIDCALL:
        description = "This function is not valid for the current state of this object.";
        break;
    case DSERR_INVALIDPARAM:
        description = "An invalid parameter was passed to the returning function.";
        break;
    case DSERR_NOAGGREGATION:
        description = "The object does not support aggregation.";
        break;
    case DSERR_NODRIVER:
        description = "No sound driver is available for use, or the given GUID is not a valid DirectSound device ID.";
        break;
    case DSERR_NOINTERFACE:
        description = "The requested COM interface is not available.";
        break;
    case DSERR_OBJECTNOTFOUND:
        description = "The requested object was not found.";
        break;
    case DSERR_OTHERAPPHASPRIO:
        description = "Another application has a higher priority level, preventing this call from succeeding.";
        break;
    case DSERR_OUTOFMEMORY:
        description = "The DirectSound subsystem could not allocate sufficient memory to complete the caller's request.";
        break;
    case DSERR_PRIOLEVELNEEDED:
        description = "A cooperative level of DSSCL_PRIORITY or higher is required.";
        break;
    case DSERR_SENDLOOP:
        description = "A circular loop of send effects was detected.";
        break;
    case DSERR_UNINITIALIZED:
        description = "The IDirectSound8::Initialize method has not been called or has not been called successfully before other methods were called.";
        break;
    case DSERR_UNSUPPORTED:
        description = "The function called is not supported at this time.";
        break;
    }

    throw ccor::Exception( "DirectSound interface method was failed with result %x\n\nDescription: \"%s\"\nFile: \"%s\" line %d", result, description, file, line );
}

static inline void dsCheckResult(const char* file, int line, HRESULT result)
{
    if( result != DS_OK ) 
    {
        dsReportResult( file, line, result );
    }
}

#define _dsCR(HRESULT) dsCheckResult( __FILE__, __LINE__, HRESULT )
//#define _dxCR(HRESULT) HRESULT

#endif