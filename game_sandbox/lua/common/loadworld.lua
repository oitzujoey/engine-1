
Vec3_xn = {x=-1, y=0, z=0}
Vec3_xp = {x=1, y=0, z=0}
Vec3_yn = {x=0, y=-1, z=0}
Vec3_yp = {x=0, y=1, z=0}
Vec3_zn = {x=0, y=0, z=-1}
Vec3_zp = {x=0, y=0, z=1}

worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}

TurnRate = 100

boxes_length = 1000
boxes_spacing = 4


g_boxEntities = {}


function loadWorld()
	local e
	boxModel, e = mesh_load("blender/cube")
	if e ~= 0 then quit() end
	terrainModel, e = mesh_load("blender/terrain10")
	if e ~= 0 then quit() end


	terrainEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_worldEntity, terrainEntity)
	e = entity_linkChild(terrainEntity, terrainModel)
	entity_setScale(terrainEntity, 100.0)
	entity_setPosition(terrainEntity, {x=0, y=0, z=200})
	entity_setOrientation(terrainEntity, {w=1, x=-1, y=0, z=0})

	for i = 1,boxes_length,1 do
		local boxEntity, e = entity_createEntity(g_entity_type_model)
		e = entity_linkChild(g_worldEntity, boxEntity)
		e = entity_linkChild(boxEntity, boxModel)
		entity_setScale(boxEntity, 1.0)
		entity_setPosition(boxEntity,
						   {x=((((i-1)-(i-1)%10-(i-1)%100)/100) - 4.5)*boxes_spacing,
							y=((((i-1)-(i-1)%10)/10)%10 - 4.5)*boxes_spacing,
							z=100 + (((i-1)%10) - 4.5)*boxes_spacing})
		entity_setOrientation(boxEntity, {w=1, x=0, y=0, z=0})
		g_boxEntities[i] = boxEntity
	end
end
