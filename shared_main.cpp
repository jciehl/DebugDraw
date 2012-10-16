
#include <cstdlib>
#include <cassert>

#include "glws.hpp"
#include "glproc.hpp"

#include "Logger.h"
#include "DebugDraw.h"
#include "DebugDrawShaders.h"
#include "Buffers.h"
#include "Transform.h"


Mesh mesh;
GLuint program;
GLuint attributes;

int setUniform( const char *name, const gk::Matrix4x4& matrix )
{
    int location= glGetUniformLocation(program, name);
    if(location < 0)
    {
        WARNING("uniform mat4 '%s': not found.\n", name);
        return -1;
    }
    glUniformMatrix4fv(location, 1, GL_TRUE, matrix);
    return 0;
}

int setUniform( const char *name, const float x, const float y, const float z, const float w )
{
    int location= glGetUniformLocation(program, name);
    if(location < 0)
    {
        WARNING("uniform '%s': not found.\n", name);
        return -1;
    }
    glUniform4f(location, x, y, z, w);
    return 0;
}


void draw( )
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw something
    glBindVertexArray(attributes);
    glUseProgram(program);
    
    setUniform("color", .8f, .8f, .8f, 1.f);
    
    gk::Transform model= gk::RotateY(30.f);
    gk::Transform view= gk::Translate( gk::Vector(0.f, 0.f, -30.f) );
    gk::Transform projection= gk::Perspective(50.f, 1.f, 1.f, 1000.f);
    gk::Transform mvp= projection * view * model;
    setUniform("mvpMatrix", mvp.matrix());
    
    if(mesh.indices > 0)
    {
        // usual openGL draw call:
        // glDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, 0);
        // replaced by:
        gk::DebugDrawElements(GL_TRIANGLES, mesh.count, GL_UNSIGNED_INT, 0, "position");
    }
    else
    {
        // usual openGL draw call:
        // glDrawArrays(GL_TRIANGLES, 0, mesh.count);
        // replaced by:
        gk::DebugDrawArrays(GL_TRIANGLES, 0, mesh.count);
    }
    
    glUseProgram(0);
    glBindVertexArray(0);
}


int init( )
{
    using namespace gk::debug;  // use available shader helpers from DebugDraw.
    
    mesh= read_OBJ("bigguy.vbo.obj");   // read a mesh
    if(mesh.count == 0)
        return -1;
    
    // compile some shaders
    program= create_program_from_file("vertex.vsl", "fragment.fsl");
    if(program == 0)
        return -1;

    // core profile : use a vertex array
    attributes= create_vertex_array();
    glBindBuffer(GL_ARRAY_BUFFER, mesh.positions);
    {
        int location= glGetAttribLocation(program, "position");
        glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indices);
    
    //clean up
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    
    // set up state
    glEnable(GL_CULL_FACE);
    return 0;
}

// clean up
void quit( )
{
    return;
}


// define a callback to use with opengl debug context
void APIENTRY debuglog( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, void* userParam )
{
    ERROR("openGL: %s\n", message);
}


int main( )
{
    // create a context using glws
    glws::init();
    glws::Visual *visual= glws::createVisual(true, glws::PROFILE_CORE);
    //~ glws::Visual *visual= glws::createVisual(true);
    assert(visual != NULL);
    glws::Drawable *drawable= glws::createDrawable(visual, 1280, 768);
    assert(drawable != NULL);
    glws::Context *context= glws::createContext(visual, NULL, glws::PROFILE_CORE, true);
    //~ glws::Context *context= glws::createContext(visual, NULL);
    assert(context != NULL);
    
    glws::makeCurrent(drawable, context);
    drawable->show();
    //~ glws::processEvents();
    
    // simple check
    MESSAGE("openGL version: '%s'\nGLSL version: '%s'\n",
        glGetString(GL_VERSION),
        glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    // install debug logger
    {
        MESSAGE("debug output.\n");
        glDebugMessageCallbackARB(debuglog, NULL);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
    }
    
    // init shaders, load objects, etc.
    if(init() < 0)
    {
        ERROR("failed.\n");
        return 1;
    }
    
    // create a shared context
    glws::Context *shared= glws::createContext(visual, context, glws::PROFILE_CORE, true);
    if(shared == NULL)
        ERROR("error creating shared context.\n");
    
    // test: shared object namespace ?
    glws::makeCurrent(drawable, shared);
    GLuint shared_program= gk::debug::create_program_from_file("vertex.vsl", "fragment.fsl");
    MESSAGE("shared program id %d\n", shared_program);
    GLuint shared_program2= gk::debug::create_program_from_file("vertex.vsl", "fragment.fsl");
    MESSAGE("shared program2 id %d\n", shared_program2);
    glFinish();
    
    glws::makeCurrent(drawable, context);
    GLuint context_program= gk::debug::create_program_from_file("vertex.vsl", "fragment.fsl");
    MESSAGE("context program id %d\n", context_program);
    glFinish();
    // yes, can't isolate debug draw context from application context... may have to build a map of object ids or ... export every required object to debug draw context :-(

    //~ glws::makeCurrent(drawable, shared);
    
    // go
    for(;;)
    {
        glws::processEvents();
        
        draw();
        drawable->swapBuffers();
    }

    // clean up
    quit();
    glws::cleanup();
    MESSAGE("done.\n");
    return 0;
}
