
Vec3_xn = {x=-1, y=0, z=0}
Vec3_xp = {x=1, y=0, z=0}
Vec3_yn = {x=0, y=-1, z=0}
Vec3_yp = {x=0, y=1, z=0}
Vec3_zn = {x=0, y=0, z=-1}
Vec3_zp = {x=0, y=0, z=1}

worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	
cobra1Entity = {}	
cobra3Entity = {}

texturePath = "oolite-binary-resources/Textures/"
modelPath = "oolite-binary-resources/Models/"

cobra3ModelFileName = "oolite_cobra3.dat"
cobra3HullTextureFileName = "oolite_cobra3_diffuse.png"
cobra3GunTextureFileName = "oolite_cobra3_subents.png"

cobra1ModelFileName = "oolite_cobramk1.dat"
cobra1TextureFileName = "oolite_cobramk1_diffuse.png"
-- cobra1TextureFileName = "bottom_metal.png"

cobra1Num = 500
cobra1Spacing = 1000.0

TurnRate = 100

function loadWorld()
	cobra3Model, error = loadOoliteModel(modelPath .. cobra3ModelFileName)
	cobra1Model, error = loadOoliteModel(modelPath .. cobra1ModelFileName)

	for i = 1,cobra1Num,1 do
		cobra1Entity[i], error = entity_createEntity(type_model)
		error = entity_linkChild(worldEntity, cobra1Entity[i])
		error = entity_linkChild(cobra1Entity[i], cobra1Model)
		entity_setPosition(cobra1Entity[i], {x=cobra1Spacing*((i%8)/4 - 1), y=cobra1Spacing*((i%64 - i%8)/32 - 1), z=cobra1Spacing*((i - i%64)/256 - 1)})
		entity_setOrientation(cobra1Entity[i], {w=1, x=0, y=0, z=0})
	end
end
