# Mapas de normais dos pisos

`CONCRETE_NORMAL.BMP` e `ASPHALT_NORMAL.BMP` são imagens de dados RGB de 512 × 512 pixels,
criadas para este projeto por `MakeNormalMaps.py`. Não são fotografias nem modelos
baixados. O gerador usa apenas a biblioteca padrão do Python; a aplicação C++ lê os
BMPs pelo carregador de texturas da base das aulas e não depende de Python.

O concreto combina ondulações suaves e poros esparsos; o asfalto tem agregado mais
irregular. As alturas são expressas em metros. O campo e suas derivadas se repetem
nas duas direções, permitindo usar `GL_REPEAT` sem uma emenda artificial na borda.

## Convenção e aplicação

- Normal no espaço da tangente: `normalize(vec3(-dh/du, -dh/dv, 1))`.
- Vermelho aponta para +U, verde para +V, azul para fora da superfície: convenção
  OpenGL **+Y**, não a variante DirectX com verde invertido.
- Os pixels armazenam `normal * 0.5 + 0.5`; no shader, ler `rgb * 2.0 - 1.0`.
- Ler como **dados lineares**, sem formato sRGB. A `CTextures` existente usa RGB linear.
- Carregar com `flipTexture=true`: o BMP tem altura positiva e sua primeira linha
  armazenada corresponde à parte inferior (+V cresce para cima).
- Repetição inicial sugerida: concreto a cada **0,64 m**, isto é,
  `worldPosition.xz / 0.64`; asfalto a cada **0,80 m**, ou `/ 0.80`.
- Força inicial sugerida ao multiplicar `normal.xy`: **0,65** para o concreto e
  **0,80** para o asfalto; normalizar depois. O relevo fica mais claro sob a luz
  rasante de uma luminária, sem alterar o contorno nem a colisão do piso.
- Converter da tangente para o mundo com uma base TBN coerente com as derivadas das
  coordenadas UV e da posição mundial; normalizar novamente após a conversão.

O JSON `normal-map-stats.json` registra as dimensões, o tamanho real da repetição,
as alturas e a verificação dos comprimentos das normais após a quantização RGB8.

Para regenerar os mesmos arquivos, execute nesta pasta:

```powershell
python MakeNormalMaps.py
```
