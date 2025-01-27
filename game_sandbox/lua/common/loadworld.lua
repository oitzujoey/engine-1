
Vec3_xn = {x=-1, y=0, z=0}
Vec3_xp = {x=1, y=0, z=0}
Vec3_yn = {x=0, y=-1, z=0}
Vec3_yp = {x=0, y=1, z=0}
Vec3_zn = {x=0, y=0, z=-1}
Vec3_zp = {x=0, y=0, z=1}

worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}

TurnRate = 100

boxes_length = 100
boxes_spacing = 10


function loadWorld()
	boxModel, error = mesh_load("blender/cube")
	terrainModel, error = mesh_load("blender/terrain10")


	terrainEntity, error = entity_createEntity(g_entity_type_model)
	error = entity_linkChild(g_worldEntity, terrainEntity)
	error = entity_linkChild(terrainEntity, terrainModel)
	entity_setScale(terrainEntity, 100.0)
	entity_setPosition(terrainEntity, {x=0, y=0, z=200})
	entity_setOrientation(terrainEntity, {w=1, x=-1, y=0, z=0})

	for i = 1,boxes_length,1 do
		boxEntity, error = entity_createEntity(g_entity_type_model)
		error = entity_linkChild(g_worldEntity, boxEntity)
		error = entity_linkChild(boxEntity, boxModel)
		entity_setScale(boxEntity, 1.0)
		entity_setPosition(boxEntity, {x=(((i-1)%10) - 4.5)*boxes_spacing, y=((((i-1)-(i-1)%10)/10) - 4.5)*boxes_spacing, z=100})
		entity_setOrientation(boxEntity, {w=1, x=0, y=0, z=0})
	end
end
