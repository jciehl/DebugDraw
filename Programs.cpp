
#include <cstring>
#include <cstdio>

#include "glproc.hpp"

#include "Logger.h"
#include "Programs.h"


std::string read_source( const char *filename )
{
    std::string source;
    FILE *in= fopen(filename, "rt");
    if(in == NULL)
    {
        ERROR("error loading source '%s'.\n", filename);
        return source;
    }
    
    char tmp[1024];
    for(;;)
    {
        tmp[0]= 0;
        if(fgets(tmp, sizeof(tmp), in) == NULL)
            break;
        source.append(tmp);
    }
    
    fclose(in);
    MESSAGE("loading source '%s'... done.\n", filename);
    return source;
}

GLuint create_shader( const GLenum type, const char *source )
{
    if(source == NULL)
        return 0;
    
    GLuint shader= glCreateShader(type);
    const char *sources= source;
    glShaderSource(shader, 1, &sources, NULL);
    glCompileShader(shader);

    GLint code;
    glGetShaderiv(shader, GL_COMPILE_STATUS,  &code);
    if(code == GL_TRUE)
        return shader;
    
    GLint length= 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if(length == 0)
        ERROR("error compiling shader (no info log).\n");
    else
    {
        std::vector<GLchar> log(length, 0);
        glGetShaderInfoLog(shader, (GLsizei) length, NULL, &log.front());
        ERROR("error compiling shader:\n%s\nfailed.\n", &log.front());
    }
    
    return 0;
}

GLuint create_shader( const GLenum type, const std::string& source )
{
    if(source.empty())
        return 0;
    
    return create_shader(type, source.c_str());
}


GLuint create_program( const GLuint vertex, const GLuint fragment )
{
    if(vertex == 0 || fragment == 0)
        return 0;
    
    GLuint program= glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    if(link_program(program) == 0)
        return program;
    
    return 0;
}

GLuint create_program_from_string( const char *vertex, const char *fragment )
{
    GLuint vertex_shader= create_shader(GL_VERTEX_SHADER, vertex);
    GLuint fragment_shader= create_shader(GL_FRAGMENT_SHADER, fragment);
    
    return create_program(vertex_shader, fragment_shader);
}

GLuint create_program_from_file( const char *vertex, const char *fragment )
{
    std::string vertex_source= read_source(vertex);
    std::string fragment_source= read_source(fragment);
    
    return create_program_from_string(vertex_source.c_str(), fragment_source.c_str());
}

int link_program( const GLuint program )
{
    glLinkProgram(program);
    
    GLint code;
    glGetProgramiv(program, GL_LINK_STATUS, &code);
    if(code == GL_TRUE)
        return 0;
    
    GLint length= 0;
    glGetShaderiv(program, GL_INFO_LOG_LENGTH, &length);
    if(length == 0)
        ERROR("error linking shader program (no info log).\n");
    else
    {
        std::vector<GLchar> log(length, 0);
        glGetProgramInfoLog(program, (GLsizei) length, NULL, &log.front());
        ERROR("error linking shader program:\n%s\nfailed.\n", &log.front());
    }
    
    return -1;
}

