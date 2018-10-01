#ifdef GL_ES
precision mediump float;
#endif

varying vec4 Color;

void main(void)
{
    gl_FragColor = Color;
}
