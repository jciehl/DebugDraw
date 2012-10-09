
#include <cstdio>

#include "GL/glew.h"
#include "Buffers.h"
#include "Logger.h"


GLuint create_buffer( const GLenum target, const GLint64 length, const void *data, const GLenum usage )
{
    GLuint buffer= 0;
    glGenBuffers(1, &buffer);
    if(buffer == 0)
        return 0;
    
    glBindBuffer(target, buffer);
    if(data != NULL)
        glBufferData(target, length, data, usage);
    else
    {
        std::vector<unsigned char> zeroes(length, 0);
        glBufferData(target, length, &zeroes.front(), usage);
    }
    
    return buffer;
}

GLuint create_vertex_array( )
{
    GLuint bindings= 0;
    glGenVertexArrays(1, &bindings);
    if(bindings == 0)
        return 0;
    
    glBindVertexArray(bindings);
    return bindings;
}

// simplistic maya obj reader, assumes vertices are already in vbo order
Mesh read_OBJ( const char *filename )
{
    FILE *in= fopen(filename, "rb");
    if(in == NULL)
    {
        ERROR("error loading mesh '%s'.\n", filename);
        return Mesh();
    }
    
    std::vector<float> positions;
    std::vector<float> normals;
    std::vector<int> indices;
    
    char line[1024];
    for(;;)
    {
        line[0]= 0;
        if(fgets(line, sizeof(line), in) == NULL)
            break;
        
        line[1023]= 0;   // ends the string
        if(line[0] == 'v')
        {
            if(line[1] == ' ')  // position
            {
                float x, y, z;
                if(sscanf(line, "v %f %f %f", &x, &y, &z) != 3)
                    break;
                positions.push_back(x);
                positions.push_back(y);
                positions.push_back(z);
            }
            else if(line[1] == 'n')     // normal
            {
                float x, y, z;
                if(sscanf(line, "vn %f %f %f", &x, &y, &z) != 3)
                    break;
                normals.push_back(x);
                normals.push_back(y);
                normals.push_back(z);
            }
        }
        
        else if(line[0] == 'f') // triangle
        {
            int a, b, c;
            if(sscanf(line, "f %d %d %d", &a, &b, &c) != 3
            && sscanf(line, "f %d//%*d %d//%*d %d//%*d", &a, &b, &c) != 3
            && sscanf(line, "f %d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", &a, &b, &c) != 3)
                break;
            indices.push_back(a -1);
            indices.push_back(b -1);
            indices.push_back(c -1);
        }
    }
    
    fclose(in);
    
    if(positions.size() == 0)
    {
        ERROR("error loading mesh '%s'. no positions.\n");
        return Mesh();
    }
    
    if(normals.size() > 0 && normals.size() != positions.size())
    {
        ERROR("error loading mesh '%s'. invalid format (not a vbo).\n");
        return Mesh();
    }
    
    Mesh mesh;
    if(indices.size() > 0)
        mesh.indices= create_buffer(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices.front());
    if(normals.size() > 0)
        mesh.normals= create_buffer(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals.front());
    
    mesh.positions= create_buffer(GL_ARRAY_BUFFER, positions.size() * sizeof(float), &positions.front());
    mesh.count= (int) indices.size() ? (int) indices.size() : (int) positions.size() / 3;
    
    MESSAGE("loading mesh '%s': %d positions, %d normals, %d indices... done.\n", 
        filename, positions.size() / 3, normals.size() / 3, indices.size());
    return mesh;
}

