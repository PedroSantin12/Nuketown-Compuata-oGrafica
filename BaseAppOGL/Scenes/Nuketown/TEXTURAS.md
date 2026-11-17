# Materiais da Nuketown

A cena continua usando C++, OpenGL, CPrimitives, CShader e CTextures da base das aulas.
Nenhuma biblioteca foi adicionada para estes materiais.

- `Scenes/Textures/grass.bmp`: gramado.
- `Scenes/Textures/mud.bmp`: terra e falhas secas do gramado.
- `Scenes/Textures/P1.jpg`: madeira de cercas e portas.
- `Scenes/Textures/R1.bmp`: telhas.

Essas quatro imagens já faziam parte do material disponibilizado pelo professor.
O carregamento utiliza CTextures::LoadTextureAnisotropic, com repetição e mipmaps.
A grama mistura duas texturas no fragment shader, com uma máscara procedural de manchas.

O asfalto, as juntas do concreto e os frisos da pintura são padrões calculados em
Nuketown.frag. As coordenadas do chão usam metros no mundo para a textura continuar
entre peças vizinhas. Paredes e telhados usam coordenadas locais em metros, acompanhando
a rotação da casa. Surface em SceneNuketown.h identifica o material de cada peça;
os seus valores correspondem aos ramos de uSurface em Nuketown.frag.

As janelas têm moldura, montantes e peitoril modelados com primitivas. O vidro é opaco,
com cor, brilho especular e uma aproximação da cor do céu conforme o ângulo da câmera.
Ainda não é environment mapping com cubemap, refração nem transparência de interiores.

O ônibus mantém seus materiais e UVs importados do arquivo .3DS, independentes
 dos materiais do terreno e da casa.
