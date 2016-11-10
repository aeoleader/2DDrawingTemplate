#version 330 core
//in vec2 TexCoord;
in vec4 out_position;
out vec4 c;

//uniform sampler2D ourTexture1;
//uniform sampler2D ourTexture2;

void main()
{
    //gl_FragColor = vec4(1.0f, 0.5f, 1.0f, 1.0f);
    if (out_position.x == 0.2 && out_position.y == 0.2) c = vec4(1.0f, 0.0f, 0.0f, 1.0f);
    c = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
