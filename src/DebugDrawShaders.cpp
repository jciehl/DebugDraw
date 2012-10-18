// jeanclaude.iehl@free.fr

#include <cstdio>

#include "Logger.h"
#include "DebugDrawShaders.h"
#include "glproc.hpp"


namespace gk {
namespace debug {

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


std::vector<GLuint> shader_manager;

GLuint create_shader( const GLenum type )
{
    GLuint shader= glCreateShader(type);
    if(shader > 0)
        shader_manager.push_back(shader);
    
    return shader;
}

void cleanup_shaders( )
{
    const int count= (int) shader_manager.size();
    for(int i= 0; i < count; i++)
        glDeleteShader(shader_manager[i]);
    
    shader_manager.clear();
}


GLuint create_shader( const GLenum type, const char *source )
{
    if(source == NULL)
        return 0;
    
    GLuint shader= create_shader(type);
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


std::vector<GLuint> program_manager;

GLuint create_program( )
{
    GLuint program= glCreateProgram();
    if(program > 0)
        program_manager.push_back(program);
    
    return program;
}

void cleanup_programs( )
{
    const int count= (int) program_manager.size();
    for(int i= 0; i < count; i++)
        glDeleteProgram(program_manager[i]);
    
    program_manager.clear();
}


GLuint create_program( const GLuint vertex, const GLuint fragment )
{
    if(vertex == 0 || fragment == 0)
        return 0;
    
    GLuint program= create_program();
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


//! \todo implement glsl_sizeof()
int glsl_sizeof( const int array_size, const GLenum glsl_type )
{
    int length= sizeof(float [16]);     // big enough for a mat4, the largest type
    return array_size * length;
}


int assign_uniformuiv( GLint location, int size, GLenum type, void *data )
{
    switch(type)
    {
        case GL_UNSIGNED_INT:
            glUniform1uiv(location, size, (GLuint *) data);
            break;
        case GL_UNSIGNED_INT_VEC2:
            glUniform2uiv(location, size, (GLuint *) data);
            break;
        case GL_UNSIGNED_INT_VEC3:
            glUniform3uiv(location, size, (GLuint *) data);
            break;
        case GL_UNSIGNED_INT_VEC4:
            glUniform4uiv(location, size, (GLuint *) data);
            break;
        
        default:
            ERROR("unknown type\n");
            return-1;
    }
    
    return 0;
}

int assign_uniformiv( GLint location, int size, GLenum type, void *data )
{
    switch(type)
    {
        case GL_INT:
        case GL_BOOL:
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
        case GL_SAMPLER_1D_ARRAY:
        case GL_SAMPLER_2D_ARRAY:
        case GL_SAMPLER_2D_RECT:
        case GL_INT_SAMPLER_1D:
        case GL_INT_SAMPLER_2D:
        case GL_INT_SAMPLER_3D:
        case GL_INT_SAMPLER_CUBE:
        case GL_INT_SAMPLER_1D_ARRAY:
        case GL_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_1D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_3D:
        case GL_UNSIGNED_INT_SAMPLER_CUBE:
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        case GL_UNSIGNED_INT:
            glUniform1iv(location, size, (GLint *) data);
            break;
        
        case GL_UNSIGNED_INT_VEC2:
            glUniform2iv(location, size, (GLint *) data);
            break;
        case GL_UNSIGNED_INT_VEC3:
            glUniform3iv(location, size, (GLint *) data);
            break;
        case GL_UNSIGNED_INT_VEC4:
            glUniform4iv(location, size, (GLint *) data);
            break;
        
        default:
            ERROR("unknown type\n");
            return -1;
    }
    
    return 0;
}

int assign_uniformfv( GLint location, int size, GLenum type, void *data )
{
    switch(type)
    {
        case GL_FLOAT:
            glUniform1fv(location, size, (GLfloat *) data);
            break;
        case GL_FLOAT_VEC2:
            glUniform2fv(location, size, (GLfloat *) data);
            break;
        case GL_FLOAT_VEC3:
            glUniform3fv(location, size, (GLfloat *) data);
            break;
        case GL_FLOAT_VEC4:
            glUniform4fv(location, size, (GLfloat *) data);
            break;
        
        case GL_FLOAT_MAT2:
            glUniformMatrix2fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        case GL_FLOAT_MAT3:
            glUniformMatrix3fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        case GL_FLOAT_MAT4:
            glUniformMatrix4fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        
        case GL_FLOAT_MAT2x3:
            glUniformMatrix2x3fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        case GL_FLOAT_MAT2x4:
            glUniformMatrix2x4fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        case GL_FLOAT_MAT3x2:
            glUniformMatrix3x2fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        case GL_FLOAT_MAT3x4:
            glUniformMatrix3x4fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        case GL_FLOAT_MAT4x2:
            glUniformMatrix4x2fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        case GL_FLOAT_MAT4x3:
            glUniformMatrix4x3fv(location, size, GL_FALSE, (GLfloat *) data);
            break;
        
        default:
            ERROR("unknown type\n");
            return -1;
    }
    
    return 0;
}

int assign_program_uniforms( GLint program, GLint active_program )
{
    GLint uniform_count= 0;
    glGetProgramiv(active_program, GL_ACTIVE_UNIFORMS, &uniform_count);
    
    GLint uniform_length= 0;
    glGetProgramiv(active_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_length);
    
    std::vector<GLchar> name(uniform_length, 0);
    std::vector<unsigned char> data;
    for(int i= 0; i < uniform_count; i++)
    {
        GLint array_size= 0;
        GLenum glsl_type= 0;
        glGetActiveUniform(active_program, i, uniform_length, NULL, &array_size, &glsl_type, &name.front());
        
        GLint location= glGetUniformLocation(program, &name.front());
        if(location < 0)
            // skip uniforms used in the other stages
            continue;
        
        if(array_size > 1)
            //! \todo
            ERROR("uniform '%s' is an array (size %d), not implemented.\n", &name.front(), array_size);
        
        // resize temp buffer to store uniform values
        data.clear();
        data.resize(glsl_sizeof(1, glsl_type));
        switch(glsl_type)
        {
            case GL_UNSIGNED_INT:
            case GL_UNSIGNED_INT_VEC2:
            case GL_UNSIGNED_INT_VEC3:
            case GL_UNSIGNED_INT_VEC4:
                // unsigned
                glGetUniformuiv(active_program, i, (GLuint *) &data.front());
                assign_uniformuiv(location, 1, glsl_type, &data.front());
                break;
            
            case GL_INT:
            case GL_INT_VEC2:
            case GL_INT_VEC3:
            case GL_INT_VEC4:
            case GL_BOOL:
            case GL_BOOL_VEC2:
            case GL_BOOL_VEC3:
            case GL_BOOL_VEC4:
            case GL_SAMPLER_1D:
            case GL_SAMPLER_2D:
            case GL_SAMPLER_3D:
            case GL_SAMPLER_CUBE:
            case GL_SAMPLER_1D_ARRAY:
            case GL_SAMPLER_2D_ARRAY:
            case GL_SAMPLER_2D_RECT:
            case GL_INT_SAMPLER_1D:
            case GL_INT_SAMPLER_2D:
            case GL_INT_SAMPLER_3D:
            case GL_INT_SAMPLER_CUBE:
            case GL_INT_SAMPLER_1D_ARRAY:
            case GL_INT_SAMPLER_2D_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_1D:
            case GL_UNSIGNED_INT_SAMPLER_2D:
            case GL_UNSIGNED_INT_SAMPLER_3D:
            case GL_UNSIGNED_INT_SAMPLER_CUBE:
            case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
            case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
                // int + bool + sampler
                glGetUniformiv(active_program, i, (GLint *) &data.front());
                assign_uniformiv(location, 1, glsl_type, &data.front());
                break;
                
            case GL_FLOAT:
            case GL_FLOAT_VEC2:
            case GL_FLOAT_VEC3:
            case GL_FLOAT_VEC4:
            case GL_FLOAT_MAT2:
            case GL_FLOAT_MAT3:
            case GL_FLOAT_MAT4:
            case GL_FLOAT_MAT2x3:
            case GL_FLOAT_MAT2x4:
            case GL_FLOAT_MAT3x2:
            case GL_FLOAT_MAT3x4:
            case GL_FLOAT_MAT4x2:
            case GL_FLOAT_MAT4x3:
                // float + matrix
                glGetUniformfv(active_program, i, (GLfloat *) &data.front());
                assign_uniformfv(location, 1, glsl_type, &data.front());
                break;
            
            default:
                ERROR("unknown type\n");
                break;
        }
    }
    
    //! \todo add support for subroutine uniforms
    //! \todo add support for uniform block bindings
    //! \todo add support for storage block bindings
    return 0;
}

}       // namespace debug
}       // namespace gk
