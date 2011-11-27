
#include "headers.h"
#include "sprite.h"
#include "engine.h"

/**
 * rectangle rendering
 */

struct ScreenVertex
{
public:
    float x, y, z, rhw;
    Color color;
    float tu, tv;    
};

const DWORD screenFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

void dxRenderRect(
    unsigned int top, unsigned int left, 
    unsigned int width, unsigned int height,
    Color color
)
{
    ScreenVertex vertices[4] =
    {
        (float)left,         (float)top,          0.0f, 1.0f, color, 0.0f, 0.0f, 
        (float)left + width, (float)top,          0.0f, 1.0f, color, 1.0f, 0.0f,
        (float)left + width, (float)top + height, 0.0f, 1.0f, color, 1.0f, 1.0f,
        (float)left,         (float)top + height, 0.0f, 1.0f, color, 0.0f, 1.0f,
    };

    iDirect3DDevice->SetFVF( screenFVF );
    iDirect3DDevice->DrawPrimitiveUP( D3DPT_TRIANGLEFAN, 2, vertices, sizeof(ScreenVertex) );
}