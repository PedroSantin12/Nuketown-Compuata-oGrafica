-- Rebuild in a NEW EMPTY scene and a NEW output folder.
-- Editable Max geometry, dimensions in metres, front = -Y, ground = Z0.
-- The imported 3DS uses named Bus* materials to share the vehicle finish in OpenGL.
(
    local scriptFile = getSourceFileName()
    if scriptFile == undefined or scriptFile == "" do throw "Execute MakeBus.ms pelo menu Scripting > Run Script."
    local folder = getFilenamePath scriptFile
    local sceneFile = folder + "Bus.max"
    local exportTarget = folder + "BUS.3DS"
    local paintFile = folder + "BUSPAINT.BMP"
    local labelFile = folder + "BUSLABEL.BMP"
    if objects.count != 0 or maxFileName != "" or getSaveRequired() do throw "Use uma cena nova e vazia."
    if doesFileExist sceneFile or doesFileExist exportTarget do throw "Use uma nova pasta de saida: Bus.max ou BUS.3DS ja existe."
    if not (doesFileExist paintFile) or not (doesFileExist labelFile) do throw "Mantenha BUSPAINT.BMP e BUSLABEL.BMP junto do script."
    local meter = units.decodeValue "1m"
    local hasInterface = not (maxOps.isInNonInteractiveMode())
    local busParts = #()
    local paint = standard name:"BusPaint" diffuse:(color 237 174 41) diffuseMap:(bitmapTexture filename:paintFile)
    paint.diffuseMapEnable = true
    local roofPaint = standard name:"BusPaintR" diffuse:(color 193 191 171)
    local trim = standard name:"BusRubbr" diffuse:(color 22 25 25)
    local glass = standard name:"BusGlass" diffuse:(color 44 70 80) glossiness:64
    local chrome = standard name:"BusChrom" diffuse:(color 165 177 179)
    local rubber = standard name:"BusRubbrT" diffuse:(color 24 27 27)
    local headlamp = standard name:"BusLight" diffuse:(color 252 233 179)
    local tailLamp = standard name:"BusLightR" diffuse:(color 186 29 20)
    local marker = standard name:"BusLightA" diffuse:(color 231 113 28)
    local labelMat = standard name:"BusPaintL" diffuse:(color 255 255 255) diffuseMap:(bitmapTexture filename:labelFile)
    labelMat.diffuseMapEnable = true

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
            (chamferBox name:label width:(size.x * scale) length:(size.y * scale) height:(size.z * scale) fillet:(corner * scale) fillet_segments:4 mapCoords:true)
        else
            (box name:label width:(size.x * scale) length:(size.y * scale) height:(size.z * scale) mapCoords:true)
        finishPart part targetCenter materialValue scale allParts
    )
    fn cylinderPart label targetCenter radius depth axis materialValue scale allParts sides:32 =
    (
        local part = cylinder name:label radius:(radius * scale) height:(depth * scale) sides:sides heightsegs:1 capsegs:1 smooth:true mapCoords:true
        if axis == #axle then in coordsys world rotate part (angleaxis 90 [0,1,0])
        else in coordsys world rotate part (angleaxis 90 [1,0,0])
        finishPart part targetCenter materialValue scale allParts
    )
    fn beamPart label startPos endPos radius materialValue scale allParts =
    (
        local delta = endPos - startPos
        local part = cylinder name:label radius:(radius * scale) height:((length delta) * scale) sides:12 heightsegs:1 capsegs:1 smooth:true mapCoords:true
        part.dir = normalize delta
        finishPart part ((startPos + endPos) * 0.5) materialValue scale allParts
    )
    fn meshPart label points faces materialValue scale allParts smoothCount:0 =
    (
        local scaled = for p in points collect (p * scale)
        local part = mesh name:label vertices:scaled faces:faces
        for f = 1 to part.numfaces do setFaceSmoothGroup part f (if f <= smoothCount then 1 else 0)
        setNumTVerts part points.count
        for vertexIndex = 1 to points.count do setTVert part vertexIndex points[vertexIndex]
        buildTVFaces part
        for faceIndex = 1 to faces.count do setTVFace part faceIndex faces[faceIndex]
        part.material = materialValue
        update part
        append allParts part
        part
    )
    fn unwrapPaintedPart part showPanel =
    (
        local mapping = Unwrap_UVW()
        addModifier part mapping
        mapping.unwrap.setMapChannel 1
        if showPanel do (select part; modPanel.setCurrentObject mapping node:part ui:true)
        local directions = #([1,0,0], [-1,0,0], [0,1,0], [0,-1,0], [0,0,1], [0,0,-1])
        mapping.unwrap2.flattenMap 45.0 directions 0.02 true 0 true true
    )

    local body = boxPart "Body" [0,0.08,1.08] [2.35,7.78,1.08] paint meter busParts corner:0.15
    -- A shallow taper gives the front glass a real slope rather than a painted box.
    local cabin = meshPart "Cabin" #([-1.17,-3.54,1.58],[1.17,-3.54,1.58],[1.17,3.96,1.58],[-1.17,3.96,1.58],[-1.12,-3.32,2.43],[1.12,-3.32,2.43],[1.12,3.96,2.43],[-1.12,3.96,2.43]) #([1,3,2],[1,4,3],[5,6,7],[5,7,8],[1,2,6],[1,6,5],[4,8,7],[4,7,3],[1,5,8],[1,8,4],[2,3,7],[2,7,6]) paint meter busParts
    local nose = boxPart "Nose" [0,-3.67,1.13] [2.25,0.92,1.04] paint meter busParts corner:0.19
    boxPart "Frame" [0,0.05,0.59] [2.22,7.73,0.20] trim meter busParts corner:0.065
    boxPart "BumperF" [0,-4.145,0.63] [2.56,0.24,0.23] trim meter busParts corner:0.07
    boxPart "BumperR" [0,4.015,0.64] [2.56,0.22,0.23] trim meter busParts corner:0.065
    boxPart "BumpLipF" [0,-4.272,0.68] [2.44,0.025,0.063] chrome meter busParts corner:0.008
    boxPart "BumpLipR" [0,4.133,0.69] [2.44,0.025,0.063] chrome meter busParts corner:0.008

    -- Curved roof: semicircular transverse profile, straight length, closed ends.
    local roofPoints = #()
    local roofFaces = #()
    local roofSteps = 28
    for endY in #(-3.54,4.01) do for step = 0 to roofSteps do
    (
        local angle = 180.0 * step / roofSteps
        append roofPoints [1.245 * cos angle,endY,2.40 + 0.35 * sin angle]
    )
    local ring = roofSteps + 1
    for step = 1 to roofSteps do
    (
        append roofFaces [step,step+ring,step+1]
        append roofFaces [step+1,step+ring,step+ring+1]
    )
    append roofPoints [0,-3.54,2.40]
    append roofPoints [0,4.01,2.40]
    local frontCenter = roofPoints.count - 1
    local rearCenter = roofPoints.count
    for step = 1 to roofSteps do
    (
        append roofFaces [frontCenter,step,step+1]
        append roofFaces [rearCenter,step+ring+1,step+ring]
    )
    append roofFaces [1,ring,ring*2]
    append roofFaces [1,ring*2,ring+1]
    local roof = meshPart "Roof" roofPoints roofFaces roofPaint meter busParts smoothCount:(roofSteps*2)
    -- Narrow eaves and rib caps read clearly without a checker or triangle pattern.
    for side in #(-1,1) do
    (
        boxPart ("Eave" + (side as string)) [side*1.235,0.23,2.40] [0.065,7.63,0.085] roofPaint meter busParts corner:0.025
        for y in #(-2.40,-0.50,1.40,3.25) do
            boxPart ("RoofRib" + (side as string)) [side*0.69,y,2.675] [0.72,0.035,0.025] roofPaint meter busParts corner:0.008
    )

    boxPart "GrilleBed" [0,-4.145,1.11] [1.25,0.075,0.48] trim meter busParts corner:0.07
    for slat = -5 to 5 do
        boxPart ("Grill" + ((slat+6) as string)) [slat*0.102,-4.19,1.11] [0.031,0.027,0.36] chrome meter busParts corner:0.009
    boxPart "GrillTop" [0,-4.19,1.33] [1.16,0.025,0.028] chrome meter busParts corner:0.008
    boxPart "GrillLow" [0,-4.19,0.89] [1.16,0.025,0.028] chrome meter busParts corner:0.008
    boxPart "HoodRidge" [0,-3.63,1.659] [0.032,0.66,0.024] chrome meter busParts corner:0.008
    boxPart "FrontTag" [0,-4.28,0.64] [0.34,0.015,0.12] roofPaint meter busParts corner:0.008

    -- Sloped, split windshield; surrounding pillars remain separately editable.
    for side in #(-1,1) do
    (
        local suffix = if side < 0 then "L" else "R"
        local x0 = if side < 0 then -1.06 else 0.045
        local x1 = if side < 0 then -0.045 else 1.06
        meshPart ("GlassF"+suffix) #([x0,-3.567,1.72],[x1,-3.567,1.72],[x1*0.96,-3.383,2.36],[x0*0.96,-3.383,2.36]) #([1,2,3],[1,3,4]) glass meter busParts
        beamPart ("Wiper"+suffix) [side*0.77,-3.578,1.75] [side*0.34,-3.516,1.96] 0.012 trim meter busParts
        beamPart ("Pillar"+suffix) [side*1.10,-3.559,1.65] [side*1.06,-3.348,2.41] 0.026 roofPaint meter busParts
        cylinderPart ("LampRim"+suffix) [side*0.91,-4.15,1.14] 0.183 0.075 #front chrome meter busParts sides:40
        cylinderPart ("Lamp"+suffix) [side*0.91,-4.197,1.14] 0.147 0.023 #front headlamp meter busParts sides:40
        cylinderPart ("IndF"+suffix) [side*1.01,-4.075,1.48] 0.069 0.025 #front marker meter busParts sides:24
        cylinderPart ("Tail"+suffix) [side*0.94,4.015,1.12] 0.125 0.05 #front tailLamp meter busParts
        cylinderPart ("TailA"+suffix) [side*0.94,4.018,1.43] 0.081 0.05 #front marker meter busParts sides:24
        boxPart ("RearGlass"+suffix) [side*0.56,3.978,2.03] [0.94,0.035,0.54] glass meter busParts corner:0.025
        boxPart ("RearFrame"+suffix) [side*0.56,3.975,2.03] [1.03,0.025,0.63] trim meter busParts corner:0.02
        for stripe = 0 to 2 do
            boxPart ("Stripe"+suffix+(stripe as string)) [side*1.182,0.10,1.20+stripe*0.14] [0.025,7.52,0.052] trim meter busParts corner:0.008
        for windowIndex = 1 to 8 do
        (
            local rowY = -2.98+(windowIndex-1)*0.85
            boxPart ("WinFrame"+suffix+(windowIndex as string)) [side*1.169,rowY,2.015] [0.038,0.79,0.70] trim meter busParts corner:0.02
            boxPart ("Win"+suffix+(windowIndex as string)) [side*1.193,rowY,2.015] [0.023,0.69,0.59] glass meter busParts corner:0.017
            boxPart ("Rail"+suffix+(windowIndex as string)) [side*1.208,rowY,2.065] [0.018,0.69,0.028] chrome meter busParts corner:0.006
        )
        -- Folded support bars keep mirrors close to the vehicle, not across the road.
        beamPart ("MirrorBar"+suffix) [side*1.06,-3.40,2.11] [side*1.33,-3.57,2.18] 0.018 chrome meter busParts
        boxPart ("Mirror"+suffix) [side*1.33,-3.58,2.22] [0.13,0.09,0.22] trim meter busParts corner:0.035
        boxPart ("MirGlass"+suffix) [side*1.33,-3.634,2.22] [0.10,0.013,0.17] chrome meter busParts corner:0.015
        for axle = 1 to 2 do
        (
            local axleY = if axle == 1 then -2.40 else 2.65
            local wheelName = suffix+(axle as string)
            local tyre = torus name:("Tyre"+wheelName) radius1:(0.382*meter) radius2:(0.143*meter) segments:48 sides:16 smooth:2 mapCoords:true
            in coordsys world rotate tyre (angleaxis 90 [0,1,0])
            finishPart tyre [side*1.11,axleY,0.525] rubber meter busParts
            cylinderPart ("Rim"+wheelName) [side*1.252,axleY,0.525] 0.355 0.065 #axle chrome meter busParts sides:40
            cylinderPart ("RimInset"+wheelName) [side*1.291,axleY,0.525] 0.275 0.020 #axle trim meter busParts
            cylinderPart ("Cap"+wheelName) [side*1.309,axleY,0.525] 0.139 0.036 #axle chrome meter busParts
            for bolt = 0 to 5 do
            (
                local angle = bolt*60.0
                cylinderPart ("Bolt"+wheelName+(bolt as string)) [side*1.325,axleY+0.20*cos angle,0.525+0.20*sin angle] 0.026 0.026 #axle chrome meter busParts sides:10
            )
            boxPart ("Fender"+wheelName) [side*1.19,axleY,1.075] [0.18,1.26,0.13] paint meter busParts corner:0.05
        )
    )
    beamPart "WindMid" [0,-3.58,1.69] [0,-3.36,2.40] 0.022 roofPaint meter busParts
    boxPart "Header" [0,-3.555,2.54] [1.88,0.062,0.24] paint meter busParts corner:0.025
    local label = meshPart "SchoolTag" #([-0.85,-3.591,2.445],[0.85,-3.591,2.445],[0.85,-3.591,2.635],[-0.85,-3.591,2.635]) #([1,2,3],[1,3,4]) labelMat meter busParts
    setNumTVerts label 4
    setTVert label 1 [0,0,0];setTVert label 2 [1,0,0];setTVert label 3 [1,1,0];setTVert label 4 [0,1,0]
    setTVFace label 1 [1,2,3];setTVFace label 2 [1,3,4]
    update label
    for x in #(-1.02,-0.51,0,0.51,1.02) do cylinderPart ("RoofLamp"+(x as string)) [x,-3.57,2.64] 0.051 0.038 #front marker meter busParts sides:16
    -- Passenger door frame, two glazed leaves, entry step and rear emergency handle.
    boxPart "DoorFrame" [1.205,-3.10,1.40] [0.035,0.79,1.62] trim meter busParts corner:0.018
    for y in #(-3.30,-2.91) do
    (
        boxPart "DoorLeaf" [1.230,y,1.43] [0.025,0.35,1.46] paint meter busParts corner:0.018
        boxPart "DoorGlass" [1.249,y,1.74] [0.018,0.29,0.69] glass meter busParts corner:0.014
    )
    boxPart "Step" [1.24,-3.11,0.58] [0.28,0.74,0.09] trim meter busParts corner:0.025
    boxPart "RearHandle" [0,4.01,1.49] [0.23,0.035,0.036] chrome meter busParts corner:0.008

    for part in #(body,cabin,nose) do unwrapPaintedPart part hasInterface
    -- Preserve texture coordinates and explicit smoothing groups in the 3DS export.
    if not(saveMaxFile sceneFile quiet:true) do throw "Nao foi possivel salvar Bus.max."
    select busParts
    exportFile exportTarget #noPrompt selectedOnly:true
    if not(doesFileExist exportTarget) do throw "Bus.max salvo; exporte manualmente BUS.3DS preservando Texture Coordinates."
    if hasInterface do (select body;max tool zoomextents all)
    format "Onibus atualizado: %\nExportacao: %\nPartes: %\n" sceneFile exportTarget busParts.count
)
