#version 100

#ifdef GL_ES
  precision mediump float;
#endif

uniform sampler2D uniform_texture;
varying vec2      v_texCoord_out;

vec4 InvertColor(vec4 color)
{
    return vec4(1.0) - color;
}

void main()
{
    vec4 color   = texture2D(uniform_texture, v_texCoord_out);
    gl_FragColor = InvertColor(color);
}
