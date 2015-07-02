// simple shader program
#version 330


#ifdef VERTEX_SHADER
    uniform mat4 mvpMatrix;
    uniform mat4 normalMatrix;
    
    layout (location= 0) in vec3 position;
    layout (location= 1) in vec3 texcoord; 
    layout (location= 2) in vec3 normal; 
    
    out vec3 vertex_normal;
    out vec2 vertex_texcoord;

    void main( )
    {
        gl_Position= mvpMatrix * vec4(position, 1.0);
        vertex_normal= mat3(normalMatrix) * normal;
	vertex_texcoord= texcoord.xy;
    }
#endif

#ifdef FRAGMENT_SHADER
    //~ uniform vec4 color;
    uniform sampler2D color;
    
    in vec2 vertex_texcoord;
    in vec3 vertex_normal;
    out vec4 fragment_color;

    void main( )
    {
        fragment_color= texture(color, vertex_texcoord) * abs(normalize(vertex_normal).z);
        //~ fragment_color= vec4(1.0, 0.1, 0.1, 1.0);
    }
#endif

