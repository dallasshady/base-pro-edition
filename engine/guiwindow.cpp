
#include "headers.h"
#include "gui.h"

/**
 * class implementation
 */

GuiWindow::GuiWindow(TiXmlNode* node)
{
    assert( node->Type() == TiXmlNode::ELEMENT && strcmp( node->Value(), "window" ) == 0 );

    // initialize window name
    _name = static_cast<TiXmlElement*>( node )->Attribute( "name" );

    // read repeat if available
	int repeat_times = 0;
	RECT repeat_rect = {0,0,0,0};// = ::xmlRect( node, "repeat_rect" );
    static_cast<TiXmlElement*>( node )->Attribute( "repeat_times", &repeat_times );

	initializePanel( node );
}

GuiWindow::~GuiWindow()
{
}

/**
 * IGuiWindow
 */

gui::IGuiPanel* GuiWindow::getPanel(void)
{
    return this;
}

void GuiWindow::align(gui::AlignmentType verticalAlignment, int verticalIndent, gui::AlignmentType horizontalAlignment, int horizontalIndent)
{
    if( !_parent ) return;

    int width  = _rect.right - _rect.left;
    int height = _rect.bottom - _rect.top;

    RECT parentRect = _parent->rect();
    switch( verticalAlignment )
    {
    case gui::atTop:
        _rect.top    = verticalIndent;
        _rect.bottom = _rect.top + height;
        break;
    case gui::atBottom:
        _rect.bottom = parentRect.bottom - verticalIndent;
        _rect.top    = _rect.bottom - height;
        break;
    case gui::atCenter:
        _rect.top = ( parentRect.bottom - parentRect.top - height ) / 2;
        _rect.bottom = _rect.top + height;
        break;
    }
    switch( horizontalAlignment )
    {
    case gui::atLeft:
        _rect.left = horizontalIndent;
        _rect.right = _rect.left + width;
        break;
    case gui::atRight:
        _rect.right = parentRect.right - horizontalIndent;
        _rect.left = _rect.right - width;
        break;
    case gui::atCenter:
        _rect.left = ( parentRect.right - parentRect.left - width ) / 2;
        _rect.right = _rect.left + width;
        break;
    }

    // update caption panel
}

void GuiWindow::setTopWindow(void)
{
    if( _parent ) _parent->setTopPanel( this );
}