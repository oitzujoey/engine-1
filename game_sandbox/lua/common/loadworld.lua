
worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}

TurnRate = 0.01

g_boxes_length = 1
g_boxes_spacing = g_gridSpacing
g_boxes_scale = g_boxes_spacing/2


g_boxEntities = {}


function loadWorld()
	local e
	g_cameraEntity = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_worldEntity, g_cameraEntity)

	boxModel, e = mesh_load("blender/cube")
	if e ~= 0 then quit() end
	terrainModel, e = mesh_load("blender/terrain10")
	if e ~= 0 then quit() end
	local sandboxModel, e = mesh_load("blender/sandbox")
	if e ~= 0 then quit() end
	g_planeModel, e = mesh_load("blender/plane")
	if e ~= 0 then quit() end

	-- local sandboxEntity, e = entity_createEntity(g_entity_type_model)
	-- e = entity_linkChild(g_cameraEntity, sandboxEntity)
	-- e = entity_linkChild(sandboxEntity, sandboxModel)
	-- entity_setScale(sandboxEntity, g_boundingBoxRadius)
	-- entity_setOrientation(sandboxEntity, aaToQuat({w=G_PI/2, x=1, y=0, z=0}))

	-- terrainEntity, e = entity_createEntity(g_entity_type_model)
	-- e = entity_linkChild(g_cameraEntity, terrainEntity)
	-- e = entity_linkChild(terrainEntity, terrainModel)
	-- entity_setScale(terrainEntity, 1100.0)
	-- entity_setPosition(terrainEntity, {x=0, y=0, z=-(g_boundingBoxRadius/2+200)})
	-- entity_setOrientation(terrainEntity, {w=1, x=1, y=0, z=0})

	local planeEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, planeEntity)
	e = entity_linkChild(planeEntity, g_planeModel)
	entity_setScale(planeEntity, g_boundingBoxRadius)
	entity_setPosition(planeEntity, {x=0, y=0, z=-(g_boundingBoxRadius/2 + g_gridSpacing/2)})
	entity_setOrientation(planeEntity, {w=1, x=1, y=0, z=0})

	local planeEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, planeEntity)
	e = entity_linkChild(planeEntity, g_planeModel)
	entity_setScale(planeEntity, g_boundingBoxRadius)
	entity_setPosition(planeEntity, {x=0, y=0, z=-(g_boundingBoxRadius/2 + g_gridSpacing/2)})
	entity_setOrientation(planeEntity, {w=1, x=-1, y=0, z=0})

	for i = 1,g_boxes_length,1 do
		local boxEntity, e = entity_createEntity(g_entity_type_model)
		e = entity_linkChild(g_cameraEntity, boxEntity)
		e = entity_linkChild(boxEntity, boxModel)
		entity_setScale(boxEntity, g_boxes_scale)
		entity_setPosition(boxEntity,
						   {x=((((i-1)-(i-1)%10-(i-1)%100)/100) - 4.5)*g_boxes_spacing,
							y=((((i-1)-(i-1)%10)/10)%10 - 4.5)*g_boxes_spacing,
							z=1000 + (((i-1)%10) - 4.5)*g_boxes_spacing})
		entity_setOrientation(boxEntity, {w=1, x=0, y=0, z=0})
		g_boxEntities[i] = boxEntity
	end
end
