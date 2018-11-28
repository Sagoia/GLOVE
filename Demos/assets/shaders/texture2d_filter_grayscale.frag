#version 100

#ifdef GL_ES
  precision mediump float;
#endif

uniform sampler2D uniform_texture;
varying vec2      v_texCoord_out;

vec4 GrayScaleColor(vec4 color)
{
    float  avg = 0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b;
    return vec4(vec3(avg), 1.0);
}

void main()
{
    vec4 color   = texture2D(uniform_texture, v_texCoord_out);
    gl_FragColor = GrayScaleColor(color);
}
