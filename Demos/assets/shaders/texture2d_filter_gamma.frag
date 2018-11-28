#version 100

#ifdef GL_ES
  precision mediump float;
#endif

#define GAMMA         2.2
#define GAMMA_1       1.0/GAMMA

uniform sampler2D uniform_texture;
varying vec2      v_texCoord_out;

vec4 GammaCorrection(vec4 color)
{
    return vec4(pow(color.r, GAMMA_1), pow(color.g, GAMMA_1), pow(color.b, GAMMA_1), color.a);
}

void main()
{
    vec4 color   = texture2D(uniform_texture, v_texCoord_out);
    gl_FragColor = GammaCorrection(color);
}
