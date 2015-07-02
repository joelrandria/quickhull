#version 330

#ifdef VERTEX_SHADER

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 vertex_color;

uniform mat4 mvpMatrix;

out vec4 fragment_color;

void main()
{
  gl_Position = mvpMatrix * vec4(vertex_position, 1.0);
  fragment_color = vertex_color;
}

#endif
#ifdef FRAGMENT_SHADER

in vec4 fragment_color;

void main()
{
  gl_FragColor = fragment_color;
}

#endif
