// jeanclaude.iehl@free.fr

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <limits>

#include "Logger.h"
#include "DebugDraw.h"
#include "DebugDrawShaders.h"

#include "Transform.h"


namespace gk {
    
namespace debug {

GLint active_cull_test= 0;
GLint active_polygon_modes[2];  //! \bug nvidia driver fills 2 GLenums instead of 1, according to state tables GL 4.3 core profile
GLint active_viewport[4];

GLint active_framebuffer= 0;
    
GLenum shader_types[]= {
    GL_VERTEX_SHADER, 
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_GEOMETRY_SHADER,
    GL_FRAGMENT_SHADER,
    0
};

const char* shader_type_names[]= {
    "vertex",
    "control",
    "evaluation",
    "geometry",
    "fragment",
    NULL
};

enum {
    VERTEX_STAGE_BIT= 1,
    CONTROL_STAGE_BIT= 2,
    EVALUATION_STAGE_BIT= 4,
    GEOMETRY_STAGE_BIT= 8,
    FRAGMENT_STAGE_BIT= 16,
    MAX_STAGES= 5
};

#define TRANSFORM_STAGES_MASK (VERTEX_STAGE_BIT | CONTROL_STAGE_BIT | EVALUATION_STAGE_BIT | GEOMETRY_STAGE_BIT)
#define TESSELATION_STAGES_MASK (CONTROL_STAGE_BIT | EVALUATION_STAGE_BIT)


unsigned int shader_stages[]= {
    VERTEX_STAGE_BIT,
    CONTROL_STAGE_BIT,
    EVALUATION_STAGE_BIT,
    GEOMETRY_STAGE_BIT,
    FRAGMENT_STAGE_BIT
};


GLint active_program= 0;
std::vector<GLuint> active_shaders;
GLint active_shader_count= 0;


struct buffer_binding
{
    GLint buffer;
    GLint enabled;
    GLint size;
    GLint type;
    GLint stride;
    GLint glsl_type;
    GLint normalized;
    GLint integer;
    GLint divisor;
    GLint64 length;
    GLint64 offset;
};

std::vector<buffer_binding> active_buffers;
GLint active_vertex_array= 0;
GLint active_vertex_buffer= 0;
GLint active_index_buffer= 0;

struct attribute
{
    std::vector<GLchar> name;
    GLint array_size;
    GLint glsl_type;
};

std::vector<attribute> active_attributes;
GLint active_attribute_count= 0;


struct draw_call
{
    GLenum primitive;
    GLint first;
    GLsizei count;
    
    GLenum index_type;
    GLint64 index_offset;
};

void draw( const draw_call& params )
{
    if(params.index_type == 0)
        glDrawArrays(params.primitive, params.first, params.count);
    else
    {
        if(active_index_buffer == 0)
        {
            ERROR("glDrawElements called, but there is no index buffer. failed.\n");
            return;
        }
        
        glDrawElements(params.primitive, params.count, params.index_type, (const GLvoid *) params.index_offset);
    }
}


int gl_sizeof( const int size, const GLenum type, const int stride= 0 )
{
    if(stride != 0)
        return stride;
    
    int length= 0;
    switch(type)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            length= 1;
            break;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
        case GL_HALF_FLOAT:
            length= 2;
            break;
        case GL_FIXED:
        case GL_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT:
        case GL_INT:
            length= 4;
            break;
        case GL_FLOAT:
            length= 4;
            break;
        case GL_DOUBLE:
            length= 8;
            break;
        
        default:
            ERROR("unknown type");
    }
    
    return size * length;
}


int get_active_program_stages( )
{
    active_program= 0;
    active_shader_count= 0;
    active_shaders.clear();
    active_shaders.resize(MAX_STAGES);
    
    glGetIntegerv(GL_CURRENT_PROGRAM, &active_program);
    if(active_program == 0)
    {
        ERROR("no shader program.\n");
        return -1;      // no program
    }
    
    //! \todo add support for program pipelines. 
    
    GLint linked;
    glGetProgramiv(active_program, GL_LINK_STATUS, &linked);
    if(linked == GL_FALSE)
    {
        ERROR("shader program is not linked.\n");
        return -1;      // program can't run
    }
    
    glGetProgramiv(active_program, GL_ATTACHED_SHADERS, &active_shader_count);
    if(active_shader_count == 0)
    {
        ERROR("shader program has no shader objects attached, can't display stages.\n");
        return -1;      // no shaders
    }
    
    GLint count= 0;
    std::vector<GLuint> shaders(active_shader_count, 0);
    glGetAttachedShaders(active_program, active_shader_count, &count, &shaders.front());
    
    WARNING("shader program object %d:\n", active_program);
    for(int i= 0; i < count; i++)
    {
        GLint type;
        glGetShaderiv(shaders[i], GL_SHADER_TYPE, &type);
        
        int stage= 0;
        const char *type_name= "<unknown>";
        for(stage= 0; stage < MAX_STAGES; stage++)
        {
            if(shader_types[stage] != (GLenum) type)
                continue;
            type_name= shader_type_names[stage];
            break;
        }
        WARNING("  %s shader object %d (stage %d %s)\n", 
            type_name, shaders[i], 
            stage, shader_type_names[stage]);

        // assert shader order: vertex, control, evaluation, geometry, fragment, has to be compatible with shader_stages[] order.
        active_shaders[stage]= shaders[i];
    }
    
    WARNING("  done.\n");
    return 0;
}

GLuint find_active_shader( const GLenum shader_type )
{
    for(int i= 0; i < MAX_STAGES; i++)
    {
        if(shader_types[i] == shader_type && active_shaders[i] != 0)
            return active_shaders[i];
    }
    
    return 0;
}


const char *display_fragment_source= {
"   #version 330\n\
    layout(location= 0) out vec4 fragment_color;\n\
    void main( ) {\n\
    if(gl_FrontFacing)\n\
            fragment_color= vec4(1.f, 1.f, 1.f, 1.f);\n\
        else\n\
            fragment_color= vec4(.7f, .7f, .7f, 1.f);\n\
    }\n\
"
};

//! create a shader program using active shaders, \param mask indicates which shaders to attach.
GLuint create_display_program( unsigned int mask, const char *fragment_source )
{
    if(mask == 0 || fragment_source == NULL)
        return 0;
    
    GLuint program= glCreateProgram();
    if(program == 0)
        return 0;
    
    // attach active shaders acocrding to mask
    for(int stage= 0; stage < MAX_STAGES; stage++)
    {
        if((mask & (1<<stage)) == 0)
            // not requested
            continue;
        if(active_shaders[stage] == 0)
            // not used by the application
            continue;
        
        // attach selected shader
        glAttachShader(program, active_shaders[stage]);
    }
    
    // attach display fragment shader
    GLuint fragment_shader= create_shader(GL_FRAGMENT_SHADER, fragment_source);
    glAttachShader(program, fragment_shader);
    
    // bind attributes to the same locations
    for(int i= 0; i < active_attribute_count; i++)
        glBindAttribLocation(program, i, &active_attributes[i].name.front());
    
    // link display program
    if(link_program(program) < 0)
    {
        ERROR("error linking display shader program. failed.\n");
        glDeleteProgram(program);
        glDeleteShader(fragment_shader);
        return 0;
    }
    
    return program;
}


struct program
{
    GLuint name;
    std::vector<GLuint> stages;
    const char *fragment_source;        //!< references a static string, nothing to free
    unsigned int mask;
    
    program( )
        :
        name(0),
        stages(MAX_STAGES, 0),
        fragment_source(NULL),
        mask(0)
    {}
    
    program( const GLuint _name, const unsigned int _mask, const std::vector<GLuint>& _stages, const char *_fragment_source )
        :
        name(_name),
        stages(_stages),
        fragment_source(_fragment_source),
        mask(_mask)
    {}
    
    ~program( ) {}
    
    bool match( const unsigned int _mask, const std::vector<GLuint>& _stages, const char *_fragment_source ) const
    {
        if(_mask != mask)
            return false;
        if(_fragment_source != fragment_source) // static strings required
            return false;
        
        // mask and display source match, check shader objects
        if(_stages.size() != MAX_STAGES)
            return false;
        for(int i= 0; i < MAX_STAGES; i++)
            if(stages[i] != _stages[i])
                return false;
        
        // match found
        return true;
            
        //! \todo compute a hash value from concatenated source strings to detect shader object changes.
    }
};


std::vector<program> programs;

//! program cache, retrieve an already built shader program or create a new one
GLuint cache_get_display_program( unsigned int mask, const char *fragment_source )
{
    // build required shader state
    std::vector<GLuint> stages(MAX_STAGES, 0);
    for(int i= 0; i < MAX_STAGES; i++)
        if((mask & (1<<i)) != 0)
            stages[i]= active_shaders[i];
    
    // look up program 
    int count= (int) programs.size();
    for(int i= 0; i < count; i++)
        if(programs[i].match(mask, stages, fragment_source))
            return programs[i].name;    // hit
    
    // cache miss, build a new program
    GLuint program_name= create_display_program(mask, fragment_source);
    if(program_name == 0)
        return 0;
    
    // cache the new program
    programs.push_back( program(program_name, mask, stages, fragment_source) );
    return program_name;
}
    

int get_active_attributes( )
{
    active_attribute_count= 0;
    active_attributes.clear();

    if(active_program == 0)
        return -1;      // no program
    
    glGetProgramiv(active_program, GL_ACTIVE_ATTRIBUTES, &active_attribute_count);
    active_attributes.resize(active_attribute_count);
    
    WARNING("%d attributes:\n", active_attribute_count);
    
    GLint attribute_length= 0;
    glGetProgramiv(active_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attribute_length);
    
    GLint size;
    GLenum glsl_type;
    for(int i= 0; i < active_attribute_count; i++)
    {
        active_attributes[i].name.clear();
        active_attributes[i].name.resize(attribute_length);
        glGetActiveAttrib(active_program, i, attribute_length, NULL, &size, &glsl_type, &active_attributes[i].name.front());
        WARNING("  attribute %d '%s': array size %d, glsl type 0x%x\n", i, 
            &active_attributes[i].name.front(), size, glsl_type);
        
        active_attributes[i].array_size= size;
        active_attributes[i].glsl_type= glsl_type;
    }
    
    WARNING("  done.\n");
    return 0;
}


int get_active_buffer_bindings( )
{
    active_vertex_buffer= 0;
    active_index_buffer= 0;
    active_vertex_array= 0;
    active_buffers.clear();
    
    if(active_attribute_count == 0)
        return -1;
    
    WARNING("active buffers:\n");
    
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &active_vertex_buffer);
    WARNING("  vertex buffer object %d\n", active_vertex_buffer);
    
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &active_index_buffer);
    WARNING("  index buffer object %d\n", active_index_buffer);
    
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &active_vertex_array);
    
    if(active_vertex_array == 0)
        WARNING("  no vertex array object\n");
    else
        WARNING("  vertex array object %d:\n", active_vertex_array);
    
    active_buffers.resize(active_attribute_count);
    for(int i= 0; i < active_attribute_count; i++)
    {
        GLint attribute_buffer= 0;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &attribute_buffer);
        if(glGetError() != GL_NO_ERROR)
            break;
        
        GLint size, type, stride;
        GLint enabled, normalized;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
        if(stride == 0)
            stride= gl_sizeof(size, type);
        
        // determine glsl type from (size, type)
        GLint glsl_type= type;  // not used
        if(size > 1)
        {
            switch(type)
            {
                case GL_FLOAT:
                    glsl_type= GL_FLOAT_VEC2 + size-2;
                    break;
                case GL_DOUBLE:
                    glsl_type= GL_DOUBLE_VEC2 + size-2;
                    break;
                case GL_INT:
                    glsl_type= GL_INT_VEC2 + size-2;
                    break;
                case GL_UNSIGNED_INT:
                    glsl_type= GL_UNSIGNED_INT_VEC2 + size-2;
                    break;
                //!\todo more types
            }
        }
        
        GLvoid *offset= NULL;
        glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &offset);
        
        GLint64 length= 0;
        glBindBuffer(GL_ARRAY_BUFFER, attribute_buffer);
        glGetBufferParameteri64v(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &length);
        
        active_buffers[i].buffer= attribute_buffer;
        active_buffers[i].enabled= enabled;
        active_buffers[i].size= size;
        active_buffers[i].type= type;
        active_buffers[i].glsl_type= glsl_type;
        active_buffers[i].normalized= normalized;
        active_buffers[i].stride= stride;
        active_buffers[i].length= length;
        active_buffers[i].offset= (GLint64) offset;
        
        if(attribute_buffer != 0 /* && enabled != 0 */)
        {
            if(active_program != 0)
                WARNING("    attribute %d '%s': vertex buffer object %d, enabled %d, item size %d, item type 0x%x, glsl type 0x%x, stride %d, offset %lu\n", 
                    i, &active_attributes[i].name.front(), 
                    attribute_buffer, enabled, size, type, glsl_type, stride, offset);
            else
                WARNING("    attribute %d: vertex buffer object %d, enabled %d, item size %d, item type 0x%x, glsl type 0x%0x, stride %d, offset %lu\n", 
                    i, attribute_buffer, 
                    enabled, size, type, glsl_type, stride, offset);
        }
    }
    
    // restore state
    glBindBuffer(GL_ARRAY_BUFFER, active_vertex_buffer);
    WARNING("  done.\n");
    return 0;
}


const char *attribute_vertex_source= {
"   #version 330\n\
    uniform mat4 mvpMatrix;\n\
    layout(location= 0) in vec4 position;\n\
    out vec3 output;\n\
    void main( ) {\n\
        output= position.xyz;\n\
        gl_Position= mvpMatrix * position;\n\
    }\n\
"
};

GLuint attribute_program= 0;
GLuint attribute_program_buffer= 0;
GLuint attribute_program_bindings= 0;

int draw_attribute( const int id, const draw_call& draw_params )
{
    WARNING("draw_attribute(%d):\n", id);
    
    if(id < 0 || id >= active_attribute_count)
        return -1;
    
    if(active_buffers[id].length == 0)
    {
        WARNING("  attribute buffer %d, null length. failed\n", active_buffers[id].buffer);
        return 0;
    }

    // get attribute buffer content in 'standard' vec3 form
    // use a vertex shader and transform feedback to convert the attribute data 
    if(attribute_program == 0)
    {
        attribute_program= create_program_from_string(attribute_vertex_source, display_fragment_source);
        
        const char *varyings= "output";
        //~ const char *varyings= "gl_Position";
        glTransformFeedbackVaryings(attribute_program, 1, &varyings, GL_SEPARATE_ATTRIBS);
        if(link_program(attribute_program) < 0)
        {
            ERROR("error linking attribute display shader program. failed.\n");
            //! \todo delete attribute_program
            return -1;
        }
    }
    if(attribute_program == 0)
    {
        ERROR("error building attribute display shader program. failed.\n");
        return -1;
    }
    
    // resize transform feedback buffer
    if(attribute_program_buffer == 0)
        glGenBuffers(1, &attribute_program_buffer);
    if(attribute_program_buffer == 0)
        return -1;

    GLint active_feedback_buffer= 0;
    GLint64 active_feedback_offset= 0;
    GLint64 active_feedback_length= 0;
    glGetIntegeri_v(GL_TRANSFORM_FEEDBACK_BUFFER_BINDING, 0, &active_feedback_buffer);
    glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_START, 0, &active_feedback_offset);
    glGetInteger64i_v(GL_TRANSFORM_FEEDBACK_BUFFER_SIZE, 0, &active_feedback_length);
    
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, attribute_program_buffer);
    GLint64 feedback_length= 0;
    glGetBufferParameteri64v(GL_TRANSFORM_FEEDBACK_BUFFER, GL_BUFFER_SIZE, &feedback_length);

    GLint64 stride = active_buffers[id].stride;
    GLint64 count= (active_buffers[id].length - active_buffers[id].offset) / stride;
    
    WARNING("  vertex buffer object %d: length %lu, stride %lu, offset %lu, count %d\n", active_buffers[id].buffer, 
        active_buffers[id].length, stride, active_buffers[id].offset, count);
    
    // store count vec3s
    GLint64 length= count * sizeof(float [3]);
    if(feedback_length < length)
    {
        std::vector<unsigned char> zeroes(length, 0);
        glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, length, &zeroes.front(), GL_DYNAMIC_COPY);
        //~ WARNING("  resize feedback buffer: %d < %d\n", feedback_length, length);
    }
    
    // convert buffer content
    if(attribute_program_bindings == 0)
        glGenVertexArrays(1, &attribute_program_bindings);
    if(attribute_program_bindings == 0)
    {
        //! \todo restore application state
        return -1;
    }

    // bind the attribute buffer
    glBindVertexArray(attribute_program_bindings);
    glBindBuffer(GL_ARRAY_BUFFER, active_buffers[id].buffer);
    glVertexAttribPointer(0, active_buffers[id].size, active_buffers[id].type, 
        active_buffers[id].normalized, 
        active_buffers[id].stride, (const GLvoid *) active_buffers[id].offset);
    glEnableVertexAttribArray(0);
    
    if(active_index_buffer != 0)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, active_index_buffer);
    
    // feedback: convert buffer content
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(attribute_program);
    
    GLint64 first= active_buffers[id].offset / stride;
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, first, count);
    glEndTransformFeedback();
    
    glDisable(GL_RASTERIZER_DISCARD);
    
    // read back buffer content / glMap ?
    std::vector<float> positions(count * 3, 0.f);
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, length, &positions.front());

    // restore previous transform feedback 
    if(active_feedback_buffer == 0 || active_feedback_length == 0)
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, active_feedback_buffer);
    else
        glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, active_feedback_buffer, active_feedback_offset, active_feedback_length);
    
    // compute bbox
    Point bmin( std::numeric_limits<float>::infinity() );
    Point bmax( - std::numeric_limits<float>::infinity() );
    for(int i= 0; i < count; i++)
    {
        Point p( positions[3*i], positions[3*i+1], positions[3*i+2] );
        
        bmin.x= std::min(p.x, bmin.x);
        bmin.y= std::min(p.y, bmin.y);
        bmin.z= std::min(p.z, bmin.z);
        
        bmax.x= std::max(p.x, bmax.x);
        bmax.y= std::max(p.y, bmax.y);
        bmax.z= std::max(p.z, bmax.z);
    }
    
    WARNING("  bbox (%f %f %f) (%f %f %f)\n", bmin.x, bmin.y, bmin.z, bmax.x, bmax.y, bmax.z);
    
    // compute a sensible transform to display the data
    float fov= 25.f;
    Point center= (bmin + bmax) * .5f;
    float radius= Distance(center, bmax);
    float distance= radius / tanf(fov / 180.f * M_PI);
    
    Transform view= LookAt( Point(0.f, 0.f, distance), center, Vector(0.f, 1.f, 0.f) );
    Transform projection= Perspective( fov *2.f, 1.f, distance - radius, distance + radius );
    Transform mvp= projection * view;
    
    glUniformMatrix4fv( glGetUniformLocation(attribute_program, "mvpMatrix"), 
        1, GL_TRUE, mvp.matrix() );
    
    // draw the data
    glViewport(0, 0, 256, 256);
    glScissor(0, 0, 256, 256);
    glEnable(GL_SCISSOR_TEST);
    
    glClearColor( .05f, .05f, .05f, 1.f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    
    draw(draw_params);
    
    // restore application state
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, active_vertex_buffer);
    
    glBindVertexArray(active_vertex_array);
    WARNING("  done.\n");
    return 0;
}

int draw_attribute( const char *name, const draw_call& params )
{
    if(name == NULL)
        return -1;
    
    for(int i= 0; i < active_attribute_count; i++)
        if(strcmp(&active_attributes[i].name.front(), name) == 0)
            return draw_attribute(i, params);
    
    ERROR("attribute '%s' does not exist. can't display attribute buffer contents.\n", name);
    return -1;
}


GLuint vertex_program= 0;

int draw_vertex_stage( const draw_call& draw_params )
{
    glViewport(256, 0, 256, 256);
    glScissor(256, 0, 256, 256);
    glEnable(GL_SCISSOR_TEST);
    
    if(find_active_shader(GL_VERTEX_SHADER) == 0)
    {
        // nothing to do, display a solid color background ?
        glClearColor( .15f, 0.f, .15f, 1.f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return 0;
    }

    glClearColor( .05f, .05f, .05f, 1.f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    WARNING("draw_vertex_stage( ):\n");
    vertex_program= cache_get_display_program( VERTEX_STAGE_BIT, display_fragment_source );
    if(vertex_program == 0)
    {
        ERROR("error building vertex display shader program. failed.");
        return -1;
    }
    
    // assign uniforms 
    glUseProgram(vertex_program);
    assign_program_uniforms(vertex_program, active_program);
    
    // draw
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    
    draw(draw_params);
    
    WARNING("  done.\n");
    return 0;
}


GLuint geometry_program= 0;

int draw_geometry_stage( const draw_call& draw_params )
{
    glViewport(512, 0, 256, 256);
    glScissor(512, 0, 256, 256);
    glEnable(GL_SCISSOR_TEST);
    
    if(find_active_shader(GL_GEOMETRY_SHADER) == 0)
    {
        // nothing to do, display a solid color background ?
        glClearColor( .5f, 0.f, .5f, 1.f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return 0;
    }

    glClearColor( .05f, .05f, .05f, 1.f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    WARNING("draw_geometry_stage( ):\n");
    geometry_program= cache_get_display_program( TRANSFORM_STAGES_MASK, display_fragment_source );
    if(geometry_program == 0)
    {
        ERROR("error building geometry display shader program. failed.");
        return -1;
    }
    
    // assign uniforms 
    glUseProgram(geometry_program);
    assign_program_uniforms(geometry_program, active_program);
    
    // draw
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    
    draw(draw_params);
    
    WARNING("  done.\n");
    return 0;
}


GLuint culling_program= 0;

int draw_culling_stage( const draw_call& draw_params )
{
    glViewport(768, 0, 256, 256);
    glScissor(768, 0, 256, 256);
    glEnable(GL_SCISSOR_TEST);
    
    bool todo= true;
    if(active_cull_test == GL_FALSE)
        // nothing to do when culling is disabled
        todo= false;
    
    switch(draw_params.primitive)
    {
        case GL_POINTS:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
        case GL_LINES:
        case GL_LINE_STRIP_ADJACENCY:
        case GL_LINES_ADJACENCY:
            // nothing to cull when drawing lines or points
            todo= false;
            break;
    }
    
    GLuint geometry= find_active_shader(GL_GEOMETRY_SHADER);
    if(geometry != 0)
    {
        GLint output_type= 0;
        glGetProgramiv(active_program, GL_GEOMETRY_OUTPUT_TYPE, &output_type);
        if(output_type != GL_TRIANGLE_STRIP)
            // nothing to cull when the geometry shader outputs lines or points
            todo= false;
    }
    
    if(!todo)
    {
        // nothing to do, display a solid color background ?
        glClearColor( .5f, 0.f, .5f, 1.f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return 0;
    }
    
    glClearColor( .05f, .05f, .05f, 1.f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    WARNING("draw_culling_stage( ):\n");
    
    culling_program= cache_get_display_program( TRANSFORM_STAGES_MASK, display_fragment_source );
    if(culling_program == 0)
    {
        ERROR("error building vertex display shader program. failed.\n");
        return -1;
    }
    
    // assign uniforms 
    glUseProgram(culling_program);
    assign_program_uniforms(culling_program, active_program);
    
    // draw
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_CULL_FACE);
    
    draw(draw_params);
    
    WARNING("  done.\n");
    return 0;    
}


int draw_fragment_stage( const draw_call& draw_params )
{
    glViewport(1024, 0, 256, 256);
    glScissor(1024, 0, 256, 256);
    glEnable(GL_SCISSOR_TEST);

    if(glIsEnabled(GL_RASTERIZER_DISCARD))
    {
        // nothing to do, nothing to rasterize, display a solid color background ?
        glClearColor( .5f, 0.f, .5f, 1.f );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return 0;
    }
    
    glClearColor( .05f, .05f, .05f, 1.f );
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    WARNING("draw_fragment_stage( ):\n");
    
    glUseProgram(active_program);
    glPolygonMode(GL_FRONT_AND_BACK, active_polygon_modes[0]);
    if(active_cull_test == 0)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
    
    // draw
    draw(draw_params);
    
    WARNING("  done.\n");
    return 0;    
}

}       // namespace debug


void DebugDrawArrays( const GLenum  mode, const GLint first, const GLsizei count, const char *position )
{
    // perform regular draw
    glDrawArrays(mode, first, count);
    
    // get required state
    debug::get_active_program_stages();
    debug::get_active_attributes();
    debug::get_active_buffer_bindings();
    
    // store draw call parameters
    debug::draw_call params;
    params.primitive= mode;
    params.first= first;
    params.count= count;
    params.index_type= 0;
    params.index_offset= 0;
    
    // store basic state
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &debug::active_framebuffer);
    //! \todo create a standard framebuffer, when the application uses a special one.
    if(debug::active_framebuffer != 0)
    {
        WARNING("rendering to an application defined framebuffer, will not work.\n");
    }
    
    GLfloat active_clear_color[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, active_clear_color);
    
    glGetIntegerv(GL_VIEWPORT, debug::active_viewport);
    
    GLint active_scissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, active_scissor);
    GLint active_scissor_test= glIsEnabled(GL_SCISSOR_TEST);
    
    debug::active_cull_test= glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_POLYGON_MODE, debug::active_polygon_modes);
    
    // display stages
    if(position == NULL)
    {
        WARNING("using default attribute 0.\n");
        debug::draw_attribute(0, params);       // default attribute
    }
    else
        debug::draw_attribute(position, params);
    debug::draw_vertex_stage(params);
    debug::draw_geometry_stage(params);
    debug::draw_culling_stage(params);
    debug::draw_fragment_stage(params);
    
    // restore application state
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, debug::active_framebuffer);    
    glBindVertexArray(debug::active_vertex_array);
    glUseProgram(debug::active_program);

    if(active_scissor_test == 0) 
        glDisable(GL_SCISSOR_TEST);
    else
        glEnable(GL_SCISSOR_TEST);
    glScissor(active_scissor[0], active_scissor[1], active_scissor[2], active_scissor[3]);
    glViewport(debug::active_viewport[0], debug::active_viewport[1], debug::active_viewport[2], debug::active_viewport[3]);
    glClearColor(active_clear_color[0], active_clear_color[1], active_clear_color[2], active_clear_color[3]);
    
    glPolygonMode(GL_FRONT_AND_BACK, debug::active_polygon_modes[0]);
    if(debug::active_cull_test == 0)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
}

void DebugDrawElements( const GLenum mode, const GLsizei count, const GLenum type, const GLvoid *indices, const char *position )
{
    // get required state
    debug::get_active_program_stages();
    debug::get_active_attributes();
    debug::get_active_buffer_bindings();
    
    // check application bugs
    if(debug::active_index_buffer == 0)
    {
        ERROR("glDrawElements( ): no index buffer.\n");
        return;
    }
    
    // perform regular draw
    glDrawElements(mode, count, type, indices);

    if(mode == GL_PATCHES)
    {
        //! \todo display of patch primitives
        ERROR("glDrawElements(GL_PATCHES): display control points. not implemented.\n");
        return;
    }
    
    // store draw call parameters
    debug::draw_call params;
    params.primitive= mode;
    params.first= 0;
    params.count= count;
    params.index_type= type;
    params.index_offset= (unsigned long int) indices;
    
    // store basic state
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &debug::active_framebuffer);
    //! \todo create a standard framebuffer, when the application uses a special one.
    if(debug::active_framebuffer != 0)
    {
        WARNING("rendering to an application defined framebuffer, will not work.\n");
    }
    
    GLfloat active_clear_color[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, active_clear_color);
    
    glGetIntegerv(GL_VIEWPORT, debug::active_viewport);
    
    GLint active_scissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, active_scissor);
    GLint active_scissor_test= glIsEnabled(GL_SCISSOR_TEST);
    
    debug::active_cull_test= glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_POLYGON_MODE, debug::active_polygon_modes);
    
    // display stages    
    if(position == NULL)
    {
        WARNING("using default attribute 0.\n");
        debug::draw_attribute(0, params);       // default attribute
    }
    else
        debug::draw_attribute(position, params);
    debug::draw_vertex_stage(params);
    debug::draw_geometry_stage(params);
    debug::draw_culling_stage(params);
    debug::draw_fragment_stage(params);
    
    // restore application state
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, debug::active_framebuffer);
    glBindVertexArray(debug::active_vertex_array);
    glUseProgram(debug::active_program);
    
    if(active_scissor_test == 0) 
        glDisable(GL_SCISSOR_TEST);
    else
        glEnable(GL_SCISSOR_TEST);
    glScissor(active_scissor[0], active_scissor[1], active_scissor[2], active_scissor[3]);
    glViewport(debug::active_viewport[0], debug::active_viewport[1], debug::active_viewport[2], debug::active_viewport[3]);
    glClearColor(active_clear_color[0], active_clear_color[1], active_clear_color[2], active_clear_color[3]);
    
    glPolygonMode(GL_FRONT_AND_BACK, debug::active_polygon_modes[0]);
    if(debug::active_cull_test == 0)
        glDisable(GL_CULL_FACE);
    else
        glEnable(GL_CULL_FACE);
}
    
}       // namespace 
