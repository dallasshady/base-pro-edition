
#ifndef IRENDERING_INTERFACE_EXTENSION_INCLUDED
#define IRENDERING_INTERFACE_EXTENSION_INCLUDED

/**
 * extension of IRendering interface with render() method
 * all IRendering implementations should be inherited from THIS class!
 */

class Rendering : public engine::IRendering
{
public:
    virtual void render(void) = 0;
};

#endif