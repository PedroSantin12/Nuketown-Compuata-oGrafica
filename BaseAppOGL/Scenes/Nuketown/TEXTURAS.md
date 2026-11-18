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
Os cômodos principais das casas permanecem fechados. As duas garagens têm aberturas
reais na geometria e são acessíveis. Suas janelas laterais transmitem a cena: o
shader usa `refract` com índice 1,52 para deslocar a leitura da imagem opaca, além
de reflexão Fresnel do cubemap. O céu transmitido também é amostrado na direção
refratada. Uma cópia da profundidade impede puxar objetos que estão na frente do vidro.
Os vidros são planos de duas faces desenhados após os opacos, ordenados do mais
distante ao mais próximo, com `GL_SRC_ALPHA / GL_ONE_MINUS_SRC_ALPHA` e sem escrever
profundidade. A imagem é resolvida a partir do framebuffer com MSAA; os destinos
só são realocados ao mudar a resolução. As janelas mantêm seus colisores.

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
Ele é calculado uma vez para os objetos estáticos e continua sendo um passe de
profundidade em OpenGL. Separadamente, o piso das garagens usa um bake verdadeiro
do 3ds Max, descrito abaixo.

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
iluminar o piso, as caixas e as paredes. O piso recebe a textura estática
`Assets/Bake/GarageShadow.bmp`, produzida pelo Render To Texture do 3ds Max 2027.2.
O arquivo `GarageBake.max` contém o receptor UV, as caixas, a bancada, as paredes,
o teto e uma luz pontual na mesma posição local da aplicação. `MakeGarageBake.ms`
reproduz a cena e o bake com o renderer Scanline e seu gerador clássico ShadowMap.

O mapa RGB de 1024 x 1024 é a saída original do elemento `ShadowsMap` do Max:
branco conserva a iluminação da lâmpada, preto a bloqueia. Não é gerado em Python,
na CPU da aplicação ou pelo passe de profundidade OpenGL. As duas garagens têm
a mesma geometria local e compartilham a textura; suas rotações são consideradas
por matrizes inversas próprias. UVs: `(localXZ + (2.33, 3.24)) / (4.35, 6.48)`.
A aplicação limita a textura ao topo do piso receptor, Y local = 0,022 m.
Ela modula apenas a contribuição da lâmpada correspondente; o sol e a iluminação
ambiente permanecem separados, evitando aplicar a mesma sombra duas vezes.
O `.max`, receptor `.3ds`, mapa de iluminação auxiliar, gerador, relatório e
verificador de orientação acompanham os arquivos em `Assets/Bake/`.

## Normal Mapping

`Assets/Materials/CONCRETE_NORMAL.BMP` e `ASPHALT_NORMAL.BMP` são mapas RGB de normais
no espaço da tangente, com 512 x 512 pixels. Foram criados para este projeto por
um gerador determinístico em Python padrão; Python não é uma dependência da aplicação.
O concreto repete a cada 0,64 m; o asfalto, a cada 0,80 m. A `CTextures` da base os
carrega como dados RGB lineares, com mipmaps e filtragem anisotrópica.

O shader decodifica RGB para [-1,1], ajusta a força do relevo e renormaliza. Uma base
de tangente/bitangente calculada a partir das derivadas da posição e dos UVs leva
a normal para o mundo. Isso preserva o formato de vértice da base e mantém as
sombras geométricas e as colisões independentes dos poros. Pedra e areia continuam
com bump procedural. O menu permite comparar o relevo e o bake ligados/desligados.


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
