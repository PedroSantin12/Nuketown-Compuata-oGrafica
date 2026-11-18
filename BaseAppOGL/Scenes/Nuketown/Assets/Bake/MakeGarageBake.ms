-- Real 3ds Max Render To Texture / Scanline shadow bake.
-- Run in a new empty scene; generated files stay next to this script unless
-- 3dsmaxbatch supplies -mxsString outputFolder:<folder>.
-- Local OpenGL metres: (x,y,z). Max coordinates: (x,-z,y).
(
    local sourceFile = getSourceFileName()
    if sourceFile == undefined or sourceFile == "" do throw "Run the saved MakeGarageBake.ms file with Scripting > Run Script."
    local folder = getFilenamePath sourceFile
    local commandArguments = maxOps.mxsCmdLineArgs
    local folderArgument = if commandArguments != undefined then commandArguments[#outputFolder] else undefined
    if folderArgument != undefined do folder = folderArgument
    if substring folder folder.count 1 != "\\" and substring folder folder.count 1 != "/" do folder += "/"
    local size = 1024
    local sizeArgument = if commandArguments != undefined then commandArguments[#size] else undefined
    if sizeArgument != undefined do size = sizeArgument as integer
    if objects.count != 0 or maxFileName != "" or getSaveRequired() do throw "Use a new empty scene for the garage bake."
    local sceneFile = folder + "GarageBake.max"
    local shadowFile = folder + "GarageShadow.bmp"
    local lightingFile = folder + "GarageLighting.bmp"
    local receiverFile = folder + "GARFLOOR.3ds"
    if doesFileExist sceneFile or doesFileExist shadowFile or doesFileExist lightingFile or doesFileExist receiverFile do throw "Output already exists: use a new output folder."
    makeDir folder all:true
    units.systemType = #Meters
    units.systemScale = 1.0
    renderers.current = default_scanline_renderer()
    renderers.current.shadows = true
    ambientColor = color 0 0 0
    backgroundColor = color 0 0 0
    ColorPipelineMgr.Mode = #Unmanaged
    local neutral = standard name:"BakeNeutral" diffuse:(color 255 255 255) specularLevel:0 glossiness:0
    local blockers = #()
    fn toMax p = [p.x,-p.z,p.y]
    fn boxCG label center size materialValue blockers =
    (
        local part = box name:label width:size.x length:size.z height:size.y
        part.pos = toMax (center - [0, size.y*0.5, 0])
        part.material = materialValue
        append blockers part
        part
    )
    -- A single two-triangle receiver has a non-overlapping full-square UV layout.
    -- u=(localX+2.33)/4.35; v=(localZ+3.24)/6.48.
    local floor = mesh name:"GarageFloor_BakeReceiver" vertices:#([-2.33,3.24,0.022],[2.02,3.24,0.022],[2.02,-3.24,0.022],[-2.33,-3.24,0.022]) faces:#([1,3,2],[1,4,3])
    setNumTVerts floor 4
    for i=1 to 4 do setTVert floor i (#([0,0,0],[1,0,0],[1,1,0],[0,1,0])[i])
    buildTVFaces floor
    setTVFace floor 1 [1,3,2]
    setTVFace floor 2 [1,4,3]
    floor.material = neutral
    update floor
    -- Same static shell, raised gate, crates and bench as BuildHouse().
    -- Left wall has the new transparent side-window opening; glass does not cast shadows.
    boxCG "WallLeftLower" [-2.13,0.575,0] [0.20,1.15,6.48] neutral blockers
    boxCG "WallLeftUpper" [-2.13,2.465,0] [0.20,0.63,6.48] neutral blockers
    boxCG "WallLeftRear" [-2.13,1.65,-2.295] [0.20,1.00,1.89] neutral blockers
    boxCG "WallLeftFront" [-2.13,1.65,1.995] [0.20,1.00,2.49] neutral blockers
    boxCG "WallRight" [1.82,1.39,0] [0.20,2.78,6.48] neutral blockers
    boxCG "Ceiling" [-0.155,2.72,0] [4.35,0.12,6.48] neutral blockers
    for side in #(-1,1) do
    (
        boxCG ("FrontJamb"+side as string) [side*2.04,1.16,3.24] [0.38,2.32,0.20] neutral blockers
        boxCG ("RearWall"+side as string) [side*1.525,1.10,-3.24] [1.41,2.20,0.20] neutral blockers
    )
    boxCG "FrontLintel" [0,2.55,3.24] [4.46,0.46,0.20] neutral blockers
    boxCG "RearLintel" [0,2.49,-3.24] [4.46,0.58,0.20] neutral blockers
    boxCG "RaisedGarageGate" [0,2.52,1.90] [3.70,0.10,2.55] neutral blockers
    boxCG "CrateLower" [-1.48,0.37,-1.96] [0.84,0.70,0.88] neutral blockers
    boxCG "CrateUpper" [-1.48,0.96,-1.96] [0.64,0.48,0.66] neutral blockers
    boxCG "CrateRight" [1.18,0.40,-1.99] [0.78,0.76,0.86] neutral blockers
    boxCG "CrateFront" [-1.46,0.29,0.39] [0.76,0.54,0.72] neutral blockers
    boxCG "Workbench" [1.30,0.64,0.37] [0.58,1.20,1.45] neutral blockers
    boxCG "BenchTop" [1.30,1.27,0.37] [0.67,0.075,1.54] neutral blockers
    boxCG "BenchBox" [1.26,1.48,0.43] [0.38,0.34,0.47] neutral blockers
    -- Interior point source matches garagePoint(-.16,2.48,-.15) in the application.
    local lamp = omniLight name:"GaragePoint_MatchingOpenGL" pos:(toMax [-0.16,2.48,-0.15]) multiplier:1.0 rgb:(color 255 255 255)
    -- Node.castShadows is a different property; the light's base object enables
    -- actual lighting shadows in Scanline (see Autodesk light common properties).
    lamp.baseObject.castShadows = true
    lamp.useGlobalShadowSettings = false
    -- Classic scanline ShadowMap is used deliberately: in 2027.2 the legacy
    -- raytraceShadow path produced a constant white RTT output even for the
    -- official sphere/plane example. ShadowMap renders real caster silhouettes.
    lamp.baseObject.shadowGenerator = shadowMap mapsize:2048 samplerange:1.0 mapbias:0.001 absoluteMapBias:true twoSidedShadows:true
    lamp.shadowColor = color 0 0 0
    lamp.shadowMultiplier = 1.0
    format "LIGHT_SHADOW_CONFIG cast:% generator:% bias:% global:% rendererShadows:%\n" lamp.baseObject.castShadows (classof lamp.shadowGenerator) lamp.baseObject.shadowGenerator.mapbias lamp.useGlobalShadowSettings renderers.current.shadows
    for part in blockers do format "BLOCKER % center:% bounds:% castShadows:%\n" part.name part.pos (nodeGetBoundingBox part (matrix3 1)) part.castShadows
    local shadow = ShadowsMap()
    shadow.elementName = "GaragePoint_StaticShadow"
    shadow.outputSzX = size
    shadow.outputSzY = size
    shadow.fileType = shadowFile
    shadow.fileName = filenameFromPath shadowFile
    shadow.filterOn = true
    shadow.enabled = true
    local light = LightingMap()
    light.elementName = "GaragePoint_LightingEvidence"
    light.outputSzX = size
    light.outputSzY = size
    light.fileType = lightingFile
    light.fileName = filenameFromPath lightingFile
    light.filterOn = true
    light.shadowsOn = true
    light.enabled = true
    floor.INodeBakeProperties.removeAllBakeElements()
    floor.INodeBakeProperties.addBakeElement shadow
    floor.INodeBakeProperties.addBakeElement light
    floor.INodeBakeProperties.bakeEnabled = true
    floor.INodeBakeProperties.bakeChannel = 1
    floor.INodeBakeProperties.nDilations = 2
    select (join #(floor) blockers)
    saveMaxFile sceneFile quiet:true
    format "PRE_BAKE floorBounds:% lamp:%\n" (nodeGetBoundingBox floor (matrix3 1)) lamp.pos
    local testPoint = toMax [-1.7,0.022,-2.9]
    local testRay = ray testPoint (normalize(lamp.pos-testPoint))
    for part in blockers do
    (
        local hit = intersectRay part testRay
        if hit != undefined do format "SHADOW_RAY_HIT % at %\n" part.name hit.pos
    )
    format "BAKE_START renderer:% receiver:% resolution:% shadow:%\n" (classof renderers.current) floor.name size shadowFile
    render rendertype:#bakeSelected vfb:off progressBar:false outputSize:[size,size] shadows:true
    if not(doesFileExist shadowFile) or not(doesFileExist lightingFile) do throw "Render To Texture did not create both bake outputs."
    select floor
    exportFile receiverFile #noPrompt selectedOnly:true
    local bakedMaterial = standard name:"GarageFloor_BakedShadowPreview" diffuseMap:(bitmapTexture filename:shadowFile) selfIllumAmount:100
    -- Store companion filenames rather than machine-specific temporary paths.
    bakedMaterial.diffuseMap.filename = filenameFromPath shadowFile
    shadow.fileType = filenameFromPath shadowFile
    light.fileType = filenameFromPath lightingFile
    floor.material = Shell_Material originalMaterial:neutral bakedMaterial:bakedMaterial viewportMtlIndex:1 renderMtlIndex:0
    showTextureMap floor.material floor.material.bakedMaterial true
    saveMaxFile sceneFile quiet:true
    format "BAKE_SUCCESS ShadowsMap:% LightingMap:% source:% receiver:%\n" shadowFile lightingFile sceneFile receiverFile
)
