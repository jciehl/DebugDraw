
#ifndef _GK_DEBUG_DRAW_H
#define _GK_DEBUG_DRAW_H

#include "glimports.hpp"


extern glws::Visual *visual;
extern glws::Drawable *drawable;
extern glws::Context *context;

namespace gk {
    
void DebugDrawArrays( const GLenum  mode, const GLint first, const GLsizei count, const char *position= NULL );
void DebugDrawElements( const GLenum mode, const GLsizei count, const GLenum type, const GLvoid *indices, const char *position= NULL );

}       // namespace

#endif
