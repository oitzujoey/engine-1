
Vec3_xn = {x=-1, y=0, z=0}
Vec3_xp = {x=1, y=0, z=0}
Vec3_yn = {x=0, y=-1, z=0}
Vec3_yp = {x=0, y=1, z=0}
Vec3_zn = {x=0, y=0, z=-1}
Vec3_zp = {x=0, y=0, z=1}

worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}

TurnRate = 100

function loadWorld()
	-- terrainModel, error = rmsh_load("untitled.rmsh")
	terrainModel, error = rmsh_load("blender/terrain10.rmsh")
	terrainEntity, error = entity_createEntity(g_entity_type_model)
	error = entity_linkChild(g_worldEntity, terrainEntity)
	error = entity_linkChild(terrainEntity, terrainModel)
	entity_setScale(terrainEntity, 100.0)
	entity_setPosition(terrainEntity, {x=0, y=0, z=200})
	entity_setOrientation(terrainEntity, {w=1, x=-1, y=0, z=0})
end
