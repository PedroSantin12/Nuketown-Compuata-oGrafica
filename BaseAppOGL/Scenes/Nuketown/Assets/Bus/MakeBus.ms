-- Run in a new, empty 3ds Max scene. The bus faces -Y and rests on Z = 0.
(
    local scriptFile = getSourceFileName()
    if scriptFile == undefined or scriptFile == "" do
        throw "Abra este arquivo pelo menu Scripting > Run Script."

    local folder = getFilenamePath scriptFile
    local sceneFile = folder + "Bus.max"
    local exportTarget = folder + "BUS.3DS"
    local paintFile = folder + "BUSPAINT.BMP"

    if objects.count != 0 or maxFileName != "" or getSaveRequired() do
        throw "Abra uma cena nova e vazia antes de executar MakeBus.ms."
    if doesFileExist sceneFile or doesFileExist exportTarget do
        throw "Bus.max ou BUS.3DS ja existe nesta pasta. Abra o .max para editar o modelo existente."
    if not (doesFileExist paintFile) do
        throw "Coloque BUSPAINT.BMP ao lado de MakeBus.ms."

    local meter = units.decodeValue "1m"
    local hasInterface = not (maxOps.isInNonInteractiveMode())
    local busParts = #()

    local paint = standard name:"Paint" diffuse:(color 237 174 41) \
        diffuseMap:(bitmapTexture filename:paintFile)
    paint.diffuseMapEnable = true
    local lightPaint = standard name:"Roof" diffuse:(color 187 185 162)
    local trim = standard name:"Trim" diffuse:(color 40 39 33)
    local glass = standard name:"Glass" diffuse:(color 54 84 92) glossiness:64
    local rubber = standard name:"Rubber" diffuse:(color 31 30 28)
    local hub = standard name:"Hub" diffuse:(color 127 121 97)
    local headlamp = standard name:"Lamp" diffuse:(color 238 217 158)
    local tailLamp = standard name:"TailLamp" diffuse:(color 161 37 29)
    local marker = standard name:"Marker" diffuse:(color 229 106 31)

    fn finishPart part targetCenter materialValue scale allParts =
    (
        local bounds = nodeGetBoundingBox part (matrix3 1)
        local center = (bounds[1] + bounds[2]) * 0.5
        in coordsys world part.pos = part.pos + (targetCenter * scale - center)
        part.material = materialValue
        append allParts part
        part
    )

    fn boxPart label targetCenter size materialValue scale allParts corner:0.0 =
    (
        local part = if corner > 0 then
            (chamferBox name:label width:(size.x * scale) length:(size.y * scale) \
                height:(size.z * scale) fillet:(corner * scale) fillet_segments:2 mapCoords:true)
        else
            (box name:label width:(size.x * scale) length:(size.y * scale) \
                height:(size.z * scale) mapCoords:true)
        finishPart part targetCenter materialValue scale allParts
    )

    fn cylinderPart label targetCenter radius depth axis materialValue scale allParts sides:24 =
    (
        local part = cylinder name:label radius:(radius * scale) height:(depth * scale) \
            sides:sides mapCoords:true
        -- A Max cylinder starts on Z. Point its axis across the axle, or toward the road.
        if axis == #axle then
            in coordsys world rotate part (angleaxis 90 [0,1,0])
        else
            in coordsys world rotate part (angleaxis 90 [1,0,0])
        finishPart part targetCenter materialValue scale allParts
    )

    fn unwrapPaintedPart part showPanel =
    (
        local mapping = Unwrap_UVW()
        addModifier part mapping
        mapping.unwrap.setMapChannel 1
        if showPanel do
        (
            select part
            modPanel.setCurrentObject mapping node:part ui:true
        )
        local directions = #([1,0,0], [-1,0,0], [0,1,0], [0,-1,0], [0,0,1], [0,0,-1])
        mapping.unwrap2.flattenMap 45.0 directions 0.02 true 0 true true
    )

    -- The roof, belt line and windows are separate pieces so their silhouette is editable.
    local body = boxPart "Body" [0,0.06,1.10] [2.39,7.76,1.04] paint meter busParts corner:0.13
    local cabin = boxPart "Cabin" [0,0.11,2.03] [2.35,7.58,0.93] paint meter busParts corner:0.09
    local nose = boxPart "Nose" [0,-3.71,1.13] [2.35,0.76,1.06] paint meter busParts corner:0.14
    boxPart "Roof" [0,0.10,2.57] [2.47,7.64,0.30] lightPaint meter busParts corner:0.12
    boxPart "Frame" [0,0.12,0.56] [2.26,7.74,0.23] trim meter busParts corner:0.06
    boxPart "FrontBar" [0,-4.045,0.67] [2.54,0.22,0.22] trim meter busParts corner:0.04
    boxPart "RearBar" [0,3.944,0.66] [2.54,0.21,0.21] trim meter busParts corner:0.04
    boxPart "Grille" [0,-4.103,1.08] [1.25,0.075,0.34] trim meter busParts corner:0.025
    boxPart "Header" [0,-3.704,2.38] [1.68,0.075,0.23] paint meter busParts corner:0.025

    for slat = -4 to 4 do
        boxPart ("Grille" + ((slat + 5) as string)) [slat * 0.123,-4.15,1.08] \
            [0.031,0.035,0.26] hub meter busParts

    for side in #(-1, 1) do
    (
        local suffix = if side < 0 then "L" else "R"
        boxPart ("Shield" + suffix) [side * 0.53,-3.72,1.98] [0.98,0.058,0.57] \
            glass meter busParts corner:0.025
        boxPart ("BackWin" + suffix) [side * 0.53,3.941,2.01] [0.96,0.048,0.50] \
            glass meter busParts corner:0.02
        cylinderPart ("Lamp" + suffix) [side * 0.92,-4.101,1.11] 0.139 0.09 \
            #front headlamp meter busParts
        cylinderPart ("Tail" + suffix) [side * 0.94,3.966,1.07] 0.111 0.055 \
            #front tailLamp meter busParts
        cylinderPart ("Marker" + suffix) [side * 0.96,-3.74,2.31] 0.075 0.06 \
            #front marker meter busParts sides:16

        for stripe = 1 to 2 do
            boxPart ("Stripe" + suffix + (stripe as string)) \
                [side * 1.202,0.05,1.23 + stripe * 0.15] [0.038,7.50,0.075] \
                trim meter busParts

        for windowIndex = 1 to 8 do
        (
            local rowY = -3.15 + (windowIndex - 1) * 0.90
            boxPart ("Win" + suffix + (windowIndex as string)) \
                [side * 1.178,rowY,2.01] [0.064,0.71,0.58] glass meter busParts corner:0.024
            boxPart ("Rail" + suffix + (windowIndex as string)) \
                [side * 1.218,rowY,2.08] [0.035,0.71,0.03] lightPaint meter busParts
        )

        for axle = 1 to 2 do
        (
            local axleY = if axle == 1 then -2.40 else 2.65
            local wheelName = suffix + (axle as string)
            cylinderPart ("Tyre" + wheelName) [side * 1.095,axleY,0.525] 0.525 0.36 \
                #axle rubber meter busParts
            cylinderPart ("Hub" + wheelName) [side * 1.267,axleY,0.525] 0.285 0.09 \
                #axle hub meter busParts
            cylinderPart ("Cap" + wheelName) [side * 1.316,axleY,0.525] 0.115 0.026 \
                #axle lightPaint meter busParts sides:16
        )
    )

    for part in #(body, cabin, nose) do unwrapPaintedPart part hasInterface
    if hasInterface do
    (
        select body
        showTextureMap paint paint.diffuseMap true
        max tool zoomextents all
    )

    if not (saveMaxFile sceneFile quiet:true) do
        throw "O 3ds Max nao conseguiu salvar Bus.max."
    select busParts
    -- Preserve MAX's Texture Coordinates is enabled by default in the 3DS exporter.
    exportFile exportTarget #noPrompt selectedOnly:true
    if not (doesFileExist exportTarget) do
        throw "Bus.max foi salvo. Exporte BUS.3DS pelo menu File > Export e mantenha Preserve MAX's Texture Coordinates."

    select body
    format "Onibus editavel salvo em: %\nModelo para OpenGL: %\n" sceneFile exportTarget
)
