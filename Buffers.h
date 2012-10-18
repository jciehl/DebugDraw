
#ifndef _BUFFERS_H
#define _BUFFERS_H

#include "glimports.hpp"


GLuint create_buffer( const GLenum target, const GLint64 length, const void *data= NULL, const GLenum usage= GL_STATIC_DRAW );
GLuint create_vertex_array( );

struct Mesh
{
    GLuint positions;
    GLuint normals;
    GLuint indices;
    
    int count;
    
    Mesh( )
        :
        positions(0),
        normals(0),
        indices(0),
        count(0)
    {}
};

Mesh read_OBJ( const char *filename );

#endif
