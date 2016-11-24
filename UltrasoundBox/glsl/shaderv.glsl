#ifdef GL_ES
precision highp int;
precision highp float;
#endif

uniform    mat4 u_MVPMatrix;
attribute  vec4 a_Position;
attribute  vec2 a_texCoord;
varying    vec2 v_texCoord;
attribute  vec2 a_texCoordc;
varying    vec2 v_texCoordc;
void main()
{
    v_texCoord = a_texCoord;
    v_texCoordc = a_texCoordc;
    gl_Position = u_MVPMatrix * a_Position;
}
