#version 330 core

out vec4 FragColor;

in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D Texture0;

void main()
{
	vec4 texColor = texture(Texture0, TexCoord);

    texColor.rgb *= texColor.a;
    if(texColor.a < 0.1) // Adjust threshold as needed
        discard;

    FragColor = texColor;
}