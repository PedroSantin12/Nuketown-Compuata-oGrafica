#version 330 core

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 Vertex;

out vec4 FragColor;

// Uniforms Textura
uniform sampler2D Texture0;

// Uniforms Iluminação
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor = vec3(1.0, 1.0, 1.0);
uniform float ambientStrenght = 0.5;

// Uniforms Transparência
uniform float alpha = 1.0;

// Uniform Fog
uniform float FogDensity = 0.05;
uniform float FogStart = 0.1;
uniform float FogEnd = 100.0;
uniform vec4 FogColor = vec4(0.5, 0.5, 0.5, 1.0);
uniform vec3 CameraPos;
uniform int FogMode = 0; // 0: Linear, 1: Exponential, 2: Exponential Squared
uniform bool FogEnable = true;

vec4 CalculeColorFromTexture()
{
    return texture(Texture0, TexCoord);
}

vec3 CalculateColorFromLight()
{
    ///// Reflexão Ambiente  //////////
    // Intensidade da reflexão ambiente
    vec3 ambient = ambientStrenght * lightColor;

    ///// Reflexão Difusa  //////////
    vec3 norm = normalize(Normal);
    // Calcula o vetor do pixel a ser iluminado em relação a fonte de luz
    vec3 lightDir = normalize(lightPos - FragPos);
    // Angulo entre o vetor Normal e o vetor de direção da fonte de luz
    float diff = max(dot(norm, lightDir), 0.0); 
    // Intensidade da reflexão difusa
    vec3 diffuse = diff * lightColor;

    ///// Reflexão Especular  //////////
    // Calcula o vetor de direção do pixel até a câmera
    vec3 viewDir = normalize(viewPos - FragPos);
    // Calcula o vetor de reflexão do raio de luz
    vec3 reflectDir = reflect(-lightDir, norm);
    // Calcula o angulo entre o vetor de direção da câmera e do vetor de reflexão
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8);
    // Intensidade da reflexão especuar
    vec3 specular = spec * lightColor;

    // Modelo de iluminação completo
    vec3 light = ambient + diffuse + specular;

    return light;
}


float GetFogFactor()
{
    float C = distance(CameraPos, Vertex);
    
    if (C >= FogEnd) return 1.0;    
    if (C <= FogStart) return 0.0;

    float fogFactor = 0.0;
    float d = FogDensity;

    switch (FogMode)
    {
        case 0: // Linear
            fogFactor = 1.0 - (FogEnd - C) / (FogEnd - FogStart);
            break;

        case 1: // Exponential
            fogFactor = 1.0 - exp(-d * C);
            break;

        case 2: // Exponential Squared
            fogFactor = 1.0 - exp(-d * d * C * C);
            break;
    }

    return fogFactor;
}


void main()
{
    // Textura
    vec4 texColor = CalculeColorFromTexture();
    
    // Modelo de iluminação completo
    vec3 light = CalculateColorFromLight();


    texColor.rgb *= texColor.a;
    if(texColor.a < 0.1) // Adjust threshold as needed
        discard;

    // Cor final do pixel com iluminação
    vec4 ColorAndLight = texColor * vec4(light, 1.0);

    // Fator de neblina
    float fogFactor = GetFogFactor();

    // Cor final do pixel com iluminação e neblina
    if (FogEnable)
        FragColor = mix(ColorAndLight, FogColor, fogFactor);
    else
        FragColor = ColorAndLight * vec4(1.0, 1.0, 1.0, alpha);
}