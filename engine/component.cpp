
#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/product_version.h"
#include "engine.h"
#include "gui.h"

/**
 * component interior
 */

SIMPLE_COMPONENT_BEGIN(Engine)
    DECLARE_COMPONENT_ENTITY(Engine)
	DECLARE_COMPONENT_ENTITY(Gui)
SIMPLE_COMPONENT_END;