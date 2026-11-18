# Bake de sombras estáticas no 3ds Max

`GarageShadow.bmp` foi produzido pelo **Render To Texture do 3ds Max 2027.2**, com
o Default Scanline Renderer e um elemento **ShadowsMap** em 1024 × 1024. A imagem
é o resultado original do Max, sem pintura, ray casting externo ou substituição
por uma textura calculada pelo OpenGL. O canal vermelho é um multiplicador linear:
branco mantém a luz da garagem, preto indica sombra. O shader aplica o mapa apenas
à contribuição da respectiva luz pontual no piso; as sombras solares continuam
independentes.

As duas garagens instanciam a mesma geometria local e a mesma posição relativa
da luminária. Por isso compartilham o bake, aplicando a transformação inversa de
cada garagem no shader. Paredes, portão levantado, caixas e bancada servem de
oclusores. A janela lateral aberta na parede está representada na cena de bake.

| Arquivo | Finalidade |
| --- | --- |
| `GarageBake.max` | Cena editável: receptor com UVs, oclusores, luz e elementos de bake. O material Shell mantém o original e a prévia do bake. |
| `GarageShadow.bmp` | Multiplicador de sombras usado pela aplicação. |
| `GarageLighting.bmp` | Saída auxiliar LightingMap do mesmo Render To Texture, para inspecionar a iluminação com as sombras. |
| `GARFLOOR.3ds` | Exportação do receptor com as coordenadas UV preservadas; o piso da aplicação continua usando a geometria da cena. |
| `MakeGarageBake.ms` | Script que reconstrói a cena e executa o bake no Max. |
| `ValidateBake.py` | Confere formato, faixa de valores, sentido das UVs e pontos de luz/sombra conhecidos, sem dependências Python extras. |
| `bake-report.json` | Registro da geração, parâmetros, verificações e hashes dos arquivos. |

## Coordenadas e aplicação

As coordenadas locais da aplicação estão em metros. No Max, `(x,y,z)` do OpenGL
vira `(x,-z,y)`, mantendo um sistema de eixos consistente. O receptor cobre:

- X entre −2,33 e 2,02 m;
- Z entre −3,24 e 3,24 m;
- altura do topo do piso: Y = 0,022 m;
- luz: `(−0,16; 2,48; −0,15)` m.

As UVs são `u=(x+2.33)/4.35` e `v=(z+3.24)/6.48`. O carregador da base inverte
a imagem verticalmente ao carregá-la, seguindo a convenção OpenGL do projeto.
Não aplicar correção sRGB ao multiplicador. O shader verifica a altura, a normal
e os limites do receptor para não projetar o bake sobre paredes ou objetos.

O script usa o gerador clássico `shadowMap` interno do Scanline, com resolução
2048, bias absoluto 0,001 e sample range 1. O elemento de bake continua sendo
`ShadowsMap`. No teste mínimo executado nesta instalação, `raytraceShadow` produziu
uma saída uniforme; `shadowMap` produziu as silhuetas corretas. O validador impede
aceitar uma textura uniforme como resultado final.

## Reproduzir o bake

No Max, use uma **cena nova e vazia**, copie o script para uma **nova pasta** e
execute **Scripting → Run Script → MakeGarageBake.ms**. O script não sobrescreve
um bake existente. Mantenha os BMPs junto de `GarageBake.max` para abrir a prévia;
as referências dos materiais na cena salva são relativas.

Também é possível executar pelo PowerShell, estando nesta pasta:

```powershell
$destinoBake = Join-Path $env:TEMP ('NuketownBake-' + [guid]::NewGuid().ToString('N'))
& 'C:\Program Files\Autodesk\3ds Max 2027\3dsmaxbatch.exe' `
  (Join-Path (Get-Location) 'MakeGarageBake.ms') `
  -mxsString "outputFolder:$destinoBake" `
  -listenerlog (Join-Path $destinoBake 'listener.log')
python .\ValidateBake.py (Join-Path $destinoBake 'GarageShadow.bmp')
```

A aplicação usa os arquivos já gerados; não exige abrir o Max nem executar Python.
Ao alterar caixas, paredes, luminária ou dimensões da garagem, é preciso gerar
novamente o bake e manter a transformação/UVs do receptor compatíveis.

## Referências das APIs

O procedimento usa os elementos de bake, `INodeBakeProperties` e
`render rendertype:#bakeSelected` descritos na
[documentação oficial de Render To Texture com MAXScript](https://help.autodesk.com/cloudhelp/2025/ENU/MAXScript-Help/files/MAXScript-Tools-and-Interaction/Interacting-with-the-3ds-Max/Render-To-Texture/GUID-61344DFE-8DA6-4E19-A184-BFD9ED093EFF.html).
A interpretação do RGB como multiplicador segue a
[documentação de ShadowsMap](https://help.autodesk.com/cloudhelp/2021/ENU/MAXScript-Help/files/3ds-Max-Objects-and-Interfaces/BakeElement/GUID-BC65B699-708B-40B5-8871-10EE15A0C933.html).
