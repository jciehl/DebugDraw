
#ifndef _GK_DEBUG_DRAW_H
#define _GK_DEBUG_DRAW_H

#include "GL/glew.h"


namespace gk {
    
void DebugDrawArrays( GLenum  mode, GLint first, GLsizei count );
void DebugDrawElements( GLenum mode, GLsizei count, GLenum type, const GLvoid *indices );

}       // namespace

#endif
