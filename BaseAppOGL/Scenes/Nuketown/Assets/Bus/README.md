# Ônibus da Nuketown

`Bus.max` é o modelo editável no 3ds Max. `BUS.3ds` é a exportação usada pela aplicação OpenGL, carregada pelo `CAsset`/Assimp da base das aulas.

A versão atual tem cabine com para-brisa inclinado, teto curvo, pneus toroidais, rodas e parafusos, espelhos, porta de passageiros, faróis e placa SCHOOL BUS. As dimensões do script estão em metros; a frente aponta para -Y no Max. A aplicação ajusta o comprimento final para 8,20 m.

Os materiais `BusPaint`, `BusChrom`, `BusGlass`, `BusRubbr` e `BusLight` identificam pintura, metal, vidro, borracha e lentes. O shader da Nuketown usa esses nomes para dar ao modelo importado os mesmos acabamentos dos outros veículos. Os demais modelos importados mantêm seu tratamento próprio.

A carroceria, a cabine e o capô têm modificadores Unwrap UVW no arquivo Max. A exportação preserva as coordenadas UV e os grupos de suavização. `BUSPAINT.BMP` é a textura de pintura e `BUSLABEL.BMP` contém a placa frontal.

Para alterar o modelo, abra `Bus.max` e exporte novamente em `.3ds`, preservando Texture Coordinates. Para gerar uma cópia do zero, use uma cena nova e uma pasta nova com `MakeBus.ms`, `BUSPAINT.BMP` e `BUSLABEL.BMP`; o script evita sobrescrever os arquivos existentes. `MakeBusLabels.py` regenera apenas a placa e usa Pillow como ferramenta de edição. Python não é necessário para compilar ou executar a aplicação.
