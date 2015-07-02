#version 330

#ifdef VERTEX_SHADER
    uniform mat4 mvpMatrix;                     //!< uniform matrice transformation complete : model, view et projection
    uniform mat4 normalMatrix;                  //!< uniform matrice transformation des normales 
    
    layout(location= 0) in vec3 position;       //!< attribut numero 0
    layout(location= 1) in vec3 texcoord;       //!< attribut numero 1
    layout(location= 2) in vec3 normal;         //!< attribut numero 2
    
    out vec3 vertex_normal;
    out vec2 vertex_texcoord;

    void main( )
    {
        gl_Position= mvpMatrix * vec4(position, 1.0);
        vertex_normal= mat3(normalMatrix) * normal;
        vertex_texcoord= texcoord.st;
    }
#endif

#ifdef FRAGMENT_SHADER
    uniform sampler2D texture0; //!< parametres standards 
    uniform sampler2D texture1;
    uniform float time;
    
    in vec3 vertex_normal;      //!< varyings calcules par le vertex shader
    in vec2 vertex_texcoord;
    
    out vec4 fragment_color;    //!< sortie standard du fragment shader

    void main( )
    {
        fragment_color.rgb= abs(normalize(vertex_normal).zzz) * texture(texture0, vertex_texcoord).rgb;
    }
#endif

