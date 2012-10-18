
#ifndef _PROGRAMS_H
#define _PROGRAMS_H

#include <string>

#include "glimports.hpp"


std::string read_source( const char *filename );
GLuint create_shader( const GLenum type, const char *source );
GLuint create_shader( const GLenum type, const std::string& source );
GLuint create_program( const GLuint vertex, const GLuint fragment );
GLuint create_program_from_string( const char *vertex, const char *fragment );
GLuint create_program_from_file( const char *vertex, const char *fragment );
int link_program( const GLuint program );

#endif
