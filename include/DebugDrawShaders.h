
#ifndef _GK_DEBUGDRAW_SHADER_H
#define _GK_DEBUGDRAW_SHADER_H

#include <string>

#include "glimports.hpp"


namespace gk {

namespace debug {

std::string read_source( const char *filename );

GLuint create_shader( const GLenum type, const char *source );
GLuint create_shader( const GLenum type, const std::string& source );

GLuint create_sahder( const GLenum  type );
void cleanup_shaders( );
    
GLuint create_program( const GLuint vertex, const GLuint fragment );
GLuint create_program_from_string( const char *vertex, const char *fragment );
GLuint create_program_from_file( const char *vertex, const char *fragment );

GLuint create_program( );
void cleanup_programs();

int link_program( GLuint program );

int glsl_sizeof( const int array_size, const GLenum glsl_type );

//! internal use. program must be in use.    
int assign_uniformfv( GLint location, int size, GLenum type, void *data );
//! internal use. program must be in use.    
int assign_uniformiv( GLint location, int size, GLenum type, void *data );
//! internal use. program must be in use.    
int assign_uniformuiv( GLint location, int size, GLenum type, void *data );

//! assign active_program uniform values to program. program must be in use.
int assign_program_uniforms( GLint program, const GLint active_program );
    
}       // namespace debug

}       // namespaec

#endif
