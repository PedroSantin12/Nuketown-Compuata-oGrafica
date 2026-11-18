# Nuketown — Computação Gráfica

Projeto acadêmico em C++ e OpenGL, desenvolvido a partir da BaseAppOGL fornecida nas aulas. A cena apresenta uma versão simplificada dos exteriores da Nuketown clássica de Call of Duty: Black Ops: duas casas, garagens, quintais e a praça com ônibus e caminhão. Os interiores continuam fechados nesta etapa.

![Prévia da cena](previews/nuketown-ambiente.png)

## Estado atual

- Duas casas e garagens, com fachadas verde e amarela, telhados de duas águas e interiores fechados.
- Dois quintais ligados por rotas laterais, com escadas e decks traseiros acessíveis.
- Abrigo com treliças, galpão amarelo, churrasqueira, varal e canteiros.
- Placa legível de Nuketown, carro da garagem, carro da polícia militar e barreira no fim da rua.
- Fim da rua fechado por cerca, sem casas extras; painéis técnicos ao fundo da praça e torre atrás do perímetro.
- Cercas mais baixas para enxergar a paisagem, mantendo os limites de circulação.
- Fundo em 360° com montanhas afastadas, base fechada e chão amplo unido ao horizonte pela névoa.
- Pequeno rio decorativo e quatro palmeiras além do perímetro, como ambientação do deserto.
- Movimentação em primeira pessoa, colisões e limites do trecho do mapa.
- Câmera opcional em terceira pessoa com personagem provisório.
- Ônibus modelado no 3ds Max, exportado em `.3ds` e carregado com Assimp.
- Grama, terra, cercas e telhados com imagens da base das aulas.
- Asfalto, concreto e pintura com detalhes calculados no shader; mistura de grama e terra.
- Janelas com molduras, montantes, peitoris e aparência de vidro.
- Carros e caminhão com carrocerias curvas, para-brisas inclinados, pneus toroidais e rodas detalhadas; a traseira do caminhão fica fechada.
- Céu com nuvens, reflexos por cubemap nos vidros/metais, sombras do sol, luz nas garagens e antialiasing 4x.
- Quatro postes de rua e quatro refletores com lentes emissivas e iluminação voltada para baixo.

Os vidros refletem um cubemap do céu, gerado na inicialização. Os interiores continuam fechados. O mapa de profundidade do sol é calculado uma vez para os elementos estáticos da cena. A iluminação dos postes no chão também é preparada na inicialização; nas superfícies verticais, ela é calculada no shader. Os arquivos das cenas de exemplo do professor continuam disponíveis como referência; a aplicação inicia apenas a Nuketown.

## Como abrir e executar

1. Clone ou baixe o repositório completo, mantendo `BaseAppOGL` e `OpenGL_SDK` lado a lado.
2. No Visual Studio, instale a carga de trabalho **Desenvolvimento para desktop com C++**, incluindo o Windows SDK e o compilador MSVC **v145**, configurado no projeto.
3. Abra `BaseAppOGL/BaseAppOGL.sln`.
4. Selecione **Debug** e **x64**. Essa é a configuração validada nesta versão.
5. Defina `BaseAppOGL` como projeto de inicialização, se necessário, e pressione **Ctrl + F5**.

Os caminhos do SDK são relativos à pasta do projeto. As DLLs necessárias já estão em `BaseAppOGL/bin/Debug`. Ao executar o `.exe` fora do Visual Studio, use `BaseAppOGL` como diretório de trabalho, pois shaders e modelos são carregados por caminhos relativos.

O programa solicita um contexto **OpenGL 4.6**; o driver de vídeo precisa oferecer suporte. O 3ds Max é usado para editar os modelos e não precisa estar instalado para executar a cena.

## Controles

| Tecla | Ação |
| --- | --- |
| W, A, S, D | Andar |
| Mouse | Olhar ao redor |
| Shift esquerdo | Correr |
| F5 | Alternar primeira e terceira pessoa |
| Enter | Abrir/fechar o menu e liberar/capturar o mouse |
| R | Voltar ao ponto inicial |
| V | Alternar sincronização vertical |
| Caps Lock | Alternar modo de arame |
| Esc | Fechar a aplicação |

O menu também permite ajustar a distância da câmera em terceira pessoa e ir diretamente ao quintal amarelo. As portas das casas permanecem fechadas; use os caminhos laterais e as escadas externas para explorar.

## Organização

```text
BaseAppOGL/
  Engine/                         Base C++/OpenGL das aulas
  Scenes/Nuketown/                 Código e shaders da cena
    Assets/Bus/                   Modelo .max, exportação .3ds, script e textura
    TEXTURAS.md                   Origem e aplicação dos materiais
  Scenes/Textures/                Texturas fornecidas nas aulas
  bin/Debug/                     DLLs necessárias em tempo de execução
  BaseAppOGL.sln                  Solução do Visual Studio
OpenGL_SDK/                      Headers, bibliotecas e licenças das dependências
previews/                        Capturas da aplicação
```

O SDK inclui GLFW, GLEW, GLM, Assimp e FreeType. Dear ImGui e stb_image fazem parte dos fontes da base. As licenças disponibilizadas com as dependências estão preservadas em suas respectivas pastas. Executáveis, caches e preferências locais do Visual Studio não são versionados.

## Referência visual

[Call of duty Black ops 1 | Multiplayer Map Showcase | Nuketown](https://www.youtube.com/watch?v=tlEVTAV2HLA), enviado como referência para a disposição dos exteriores. A recriação usa geometria simplificada e materiais da base das aulas; não é uma reprodução exata de todos os assets do jogo. O rio, as palmeiras e a paisagem distante são adições de ambientação fora da área jogável.
