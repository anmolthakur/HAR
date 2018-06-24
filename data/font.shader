#if VERTEX_SHADER

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * vec4(gl_Vertex.x, gl_Vertex.y, 0, 1);
}

#elif FRAGMENT_SHADER

uniform sampler2DRect texRect;

void main()
{
    gl_FragColor = texture2DRect(texRect, gl_TexCoord[0].st);
}

#endif
