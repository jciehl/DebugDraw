DebugDraw
=========

DebugDraw is a prototype implementation of a "pipeline stage viewer": a tool to debug openGL applications.
it can display automatically:

- attribute buffer content (vertex shader input),
- vertex shader output,
- geometry shader output, if present (or solid color),
- primitive culler output (or solid color when culling is disabled),
- and fragment shader output,

more or less similar to http://msdn.microsoft.com/en-us/library/hh873194.aspx

the code runs with opengl >= 3.3 core and compatibility profiles, and needs glew and freeglut to build.

you have to call the debug function:
- gk::DebugDrawArrays(mode, first, count);
- gk::DebugDrawElements(mode, count, type, offset);

browse to debug_main.cpp to see an example.


more details are on the wiki (and some screenshots, too).
