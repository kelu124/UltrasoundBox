#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif
uniform vec3 color_value[256];
varying vec2 v_texCoord;
uniform sampler2D s_texture;
uniform int grayReverse;
void main(void)
{
    vec3 color;
    float index;
    vec4 texture = texture2D (s_texture, v_texCoord);
    index = texture.x * 255.0;
    int t = int(index);
    if(grayReverse == 0)
        color = color_value[t] / 255.0;
    else
        color = color_value[255 - t] / 255.0;
    gl_FragColor = vec4(color, 1);
}
