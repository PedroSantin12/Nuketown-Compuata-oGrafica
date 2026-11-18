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

As janelas têm moldura, caixilhos e peitoril modelados, vidro recuado e reflexos
por cubemap do céu. O céu procedural é desenhado uma vez nas seis faces de uma
textura de ambiente 256x256, com mipmaps; o mesmo cubemap é reutilizado no fundo,
vidros, cromados e pintura dos veículos. Isso evita recalcular nuvens por fragmento.
Os cômodos principais das casas permanecem fechados, sem refração ou transparência
para seu interior. As duas garagens têm aberturas reais na geometria e são acessíveis.

O ônibus remodelado no 3ds Max mantém seus materiais e UVs importados do arquivo
.3DS. Pintura, vidro, cromo e borracha recebem tratamento de material no shader,
integrando a aparência do ônibus aos demais veículos. O modelo editável `.max`
e a exportação `.3ds` acompanham o projeto.

## Expansão para o mapa completo

As placas NUKETOWN.BMP, POLICE.BMP e DELIVERY.BMP têm letras e formas desenhadas
por código em Assets/Signs/MakeLabels.py (Pillow, apenas ferramenta de edição).
Esses BMPs são lidos pelo mesmo CTextures da aplicação; Python não é necessário
para executar ou compilar a cena. A base de pedra da casa amarela usa rock.jpg.
As paredes amarelas usam frisos verticais; as verdes mantêm frisos horizontais.

As sombras de contato dos veículos mantêm mistura alfa e máscara radial suave.
A iluminação agora também utiliza um depth map 2048x2048 do sol, com filtro PCF
bilinear de quatro amostras, com correção da profundidade nas faces inclinadas.
Ele é calculado uma vez para os objetos estáticos. Não é um lightmap produzido
no 3ds Max; é um passe de profundidade em OpenGL.

## Geometria e materiais dos veículos

Os carros usam um corpo por seções, cabine inclinada, peças com cantos arredondados
e pneus toroidais. O caminhão usa as mesmas primitivas, com capô perfilado e seis
rodas detalhadas. As rotinas estão em CPrimitives.cpp; usam os mesmos buffers e
atributos de posição, normal e UV da base. Não foram adicionadas bibliotecas.
As superfícies novas separam pintura automotiva, cromado, vidro e borracha.
O ônibus utiliza o modelo importado do 3ds Max, com geometria e materiais revistos
para acompanhar o acabamento dos carros e do caminhão.

## Garagens acessíveis

As garagens verde e amarela são cascos de paredes, piso e teto, com a entrada
principal aberta e uma porta aberta nos fundos. Caixas e detalhes internos ficam
fora da passagem central. Os colisores acompanham paredes e objetos, permitindo
atravessar cada garagem; a casa principal continua sólida e fechada.

As luminárias internas combinam superfícies emissivas com fontes de luz para
iluminar o piso, as caixas e as paredes. Essa iluminação usa o shader da cena.
Ela não representa um bake de sombras produzido no 3ds Max, que permanece uma
etapa separada dos mapas de iluminação e profundidade gerados pela aplicação.


## Paisagem externa e fechamento do terreno

O fundo usa um anel montanhoso em coordenadas mundiais, com raio interno de 92 m
e externo de 215 m. Sua base é fechada abaixo do solo. O espaço central permanece
livre para o mapa: as montanhas não recebem uma transformação que também desloque
seu centro para perto das casas. O chão externo é amplo, e a névoa dos materiais
de paisagem converge para a cor horizontal do cubemap antes de sua borda distante.

A areia tem variações discretas de cor e grão. As montanhas combinam rocha e terra
conforme a inclinação e a altura. A paisagem além do perímetro contém apenas o
terreno desértico e o anel montanhoso, mantendo os postes e refletores do local.
As cercas mais baixas permitem enxergar a paisagem sem remover os colisores que
fecham o mapa. A câmera permanece em primeira pessoa.
A traseira do caminhão é fechada por painéis metálicos.
A fachada isolada ao fundo da praça e seu colisor foram removidos; a cerca
continua fechando o mapa. Os carros estacionados foram reposicionados para
liberar a entrada da garagem verde e ampliar a passagem junto à cerca no fim da rua.

## Luzes dos postes e refletores

Há quatro postes de rua e quatro refletores. Suas lentes usam o material emissivo,
que permanece visível de dia. As contribuições de luz usam alcance limitado,
atenuação pela distância e um cone suave voltado para baixo; as garagens mantêm
suas luzes próprias.

Para o chão estático, as oito luzes são avaliadas uma vez na CPU e gravadas em uma
textura RGB16F de 512 x 384 pixels. O fragment shader amostra esse mapa para as
superfícies próximas do solo voltadas para cima, evitando repetir oito avaliações
por fragmento nas vistas amplas. Nas superfícies verticais, os cones e a atenuação
continuam sendo calculados analiticamente no shader. Esse mapa representa apenas
a contribuição das luminárias: não é uma textura de sombra nem um lightmap do
3ds Max.

As sombras solares continuam usando o mapa de profundidade estático de 2048 x 2048
em OpenGL, com PCF bilinear de quatro amostras. Esse passe é independente do mapa RGB16F das luminárias.

## Sol visível

O disco solar no céu usa a mesma direção da iluminação das superfícies e do mapa
de profundidade das sombras. Ele faz parte do céu distante e mantém sua direção
no mundo quando o jogador anda pelo mapa.
