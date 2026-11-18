# Exigências e apresentação do trabalho

Referência: [Trabalho Final de Computação Gráfica](https://drive.google.com/file/d/1HcN2D6sCX5Ly1_1TlYofHQRm6YCULwfq/view), disponibilizado pelo professor.

## Onde demonstrar cada técnica

| Item do enunciado | Aplicação na Nuketown | Arquivos principais |
| --- | --- | --- |
| Modelagem de objetos no 3ds Max, formato `.3ds` | Ônibus com modelo editável e exportação carregada pelo Assimp da base. | `Assets/Bus/Bus.max`, `BUS.3ds`, `MakeBus.ms` |
| Mapeamento de texturas com Unwrap UVW no Max | Modificadores Unwrap UVW na carroceria, cabine e capô do ônibus; as coordenadas são preservadas na exportação. | `Assets/Bus/Bus.max`, `MakeBus.ms` |
| Iluminação Point/Spot | Fontes pontuais internas nas garagens; cones com atenuação nos postes e refletores. | `Nuketown.frag`, `NuketownLighting.cpp` |
| Multitextura | Mistura das imagens de grama e terra com máscara de manchas. | `Nuketown.frag`, função `surfaceColor` |
| SkyBox | Céu com nuvens e sol armazenado nas seis faces de um cubemap, usado como fundo distante. | `NuketownSky.frag`, `NuketownRendering.cpp` |
| Environment Mapping, reflexão e refração | Reflexos do cubemap em janelas, vidros e cromados; as janelas laterais das garagens também refratam a imagem real do cenário. | `Nuketown.frag`, `NuketownRendering.cpp` |
| Bump ou Normal Mapping | Mapas RGB de normais no asfalto e concreto, convertidos da tangente para o mundo; bump procedural na pedra e areia. | `Nuketown.frag`, `Assets/Materials/` |
| Neblina (Fog) | Cor dos objetos distantes converge para o horizonte; a paisagem tem transição própria. | `Nuketown.frag` |
| Transparência (Blending) | Janelas das garagens transmitem o cenário com mistura alfa, teste de profundidade e ordenação; sombras de contato também usam mistura alfa. | `SceneNuketown.cpp`, `Nuketown.frag` |
| Sombras estáticas com Bake Texture do Max | Textura de sombra da lâmpada sobre o piso das garagens, produzida e validada no Render To Texture do Max, com caixas e bancada como bloqueadores. | `Assets/Bake/GarageBake.max`, `GarageShadow.bmp`, `MakeGarageBake.ms`, `NuketownMaterials.cpp` |

Os caminhos de código e assets acima são relativos a `BaseAppOGL/Scenes/Nuketown/`.

## Roteiro de demonstração

1. Execute em **Debug / x64**. Caminhe pela praça para mostrar a modelagem, materiais, iluminação e os reflexos dos veículos.
2. Pressione **Enter**. Use **Ver janela e interior da garagem** e feche o menu com Enter. Olhe através da janela lateral: caixas, piso e a outra abertura aparecem através do vidro. Observe a pequena deformação ao mudar o ângulo.
3. No mesmo menu, alterne **Refração dos vidros** para comparar a transmissão com e sem desvio do raio. A transparência e os reflexos continuam presentes.
4. Use **Ver piso e caixas da garagem**. Compare **Relevo das superfícies** com a mesma câmera. A iluminação muda nos pequenos detalhes do concreto, sem deslocar sua geometria.
5. Compare **Sombras das caixas (3ds Max)** para identificar a contribuição da textura pré-calculada da lâmpada. A sombra do sol é uma técnica separada de Shadow Mapping em OpenGL.
6. Use **F6** para observar o mapa, o céu e a névoa a partir da câmera livre; **F11** alterna tela cheia.
7. Abra `Assets/Bus/Bus.max` no Max para mostrar a geometria e os modificadores Unwrap UVW. Os BMPs e arquivos `.3ds` acompanham o projeto; Python e Max não são necessários para executar a aplicação.
8. Abra `Assets/Bake/GarageBake.max` para mostrar o piso receptor, as caixas, a luz pontual e a configuração de Render To Texture. `Assets/Bake/README.md` explica como repetir o processo em uma pasta nova; `bake-report.json` registra os arquivos e a validação do mapa produzido.

As janelas dos cômodos principais das casas continuam com fundo fechado. Os vidros transparentes atravessáveis pelo olhar ficam nas garagens, que têm um interior real. Os colisores continuam impedindo o jogador de atravessar as janelas.

## Verificações da implementação

Compilação **Debug / x64** e execução em OpenGL 4.6 verificadas. As comparações com a mesma câmera confirmaram mudanças na imagem ao alternar refração, normal mapping e bake. O teste também verificou carregamento dos mapas, restauração dos estados de mistura/profundidade e redimensionamento dos destinos de refração ao alternar F11. Caminhos entre os quintais, travessia das garagens, escadas, limites, câmera livre e retorno à caminhada passaram na verificação de regressão.

O shader usa 16 samplers de fragmento. O mapa do Max passou por conferência de faixa claro/escuro, orientação e amostras em regiões conhecidas de luz e sombra. A origem e os hashes dos arquivos estão em `Assets/Bake/bake-report.json`.

## Técnicas adicionais

O enunciado lista física/PhysX, animação, som, partículas e sombras dinâmicas em uma seção separada de técnicas adicionais. O projeto tem colisões próprias e Shadow Mapping da geometria estática em OpenGL. Não utiliza PhysX e não deve ser apresentado como implementação dessa biblioteca. Câmera livre e tela cheia são recursos de navegação.

## Conferência administrativa

- Grupo com no máximo dois alunos.
- Confirmar o envio do e-mail solicitado pelo professor, com título **Trabalho Final em CG**, nomes completos e imagem de referência. O projeto não envia esse e-mail automaticamente.
- Datas escritas no enunciado: turma **2024T2 — 30/11/2026** e turma **2024T1 — 04/12/2026**. Confirmar a turma e eventuais alterações comunicadas em aula.
- Manter juntos os fontes, shaders, texturas, arquivos `.max`/`.3ds` e o SDK ao preparar a entrega.
