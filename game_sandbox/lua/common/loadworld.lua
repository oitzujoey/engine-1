
worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}

TurnRate = 0.01

g_boxes_length = 1000
g_boxes_scale = g_gridSpacing/2


g_boxes = {}


function loadWorld()
	local e
	g_cameraEntity = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_worldEntity, g_cameraEntity)

	boxModel, e = mesh_load("blender/cube")
	if e ~= 0 then quit() end
	terrainModel, e = mesh_load("blender/terrain10")
	if e ~= 0 then quit() end
	g_sandboxModel, e = mesh_load("blender/sandbox")
	if e ~= 0 then quit() end
	g_planeModel, e = mesh_load("blender/plane")
	if e ~= 0 then quit() end

	-- World box
	local sandboxEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, sandboxEntity)
	e = entity_linkChild(sandboxEntity, g_sandboxModel)
	entity_setScale(sandboxEntity, 10*g_boundingBoxRadius)
	entity_setOrientation(sandboxEntity, aaToQuat({w=G_PI/2, x=1, y=0, z=0}))

	-- Ground
	local planeEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, planeEntity)
	e = entity_linkChild(planeEntity, g_planeModel)
	entity_setScale(planeEntity, g_boundingBoxRadius + g_gridSpacing/2)
	entity_setPosition(planeEntity, {x=0, y=0, z=-(g_boundingBoxRadius/2 + g_gridSpacing/2)})
	entity_setOrientation(planeEntity, {w=1, x=1, y=0, z=0})

	for i = 1,g_boxes_length,1 do
		local box = {}
		local boxEntity, e = entity_createEntity(g_entity_type_model)
		e = entity_linkChild(g_cameraEntity, boxEntity)
		e = entity_linkChild(boxEntity, boxModel)
		box.entity = boxEntity

		entity_setScale(boxEntity, g_boxes_scale)
		box.position = snapToGrid({x=((i-1)%10 - 4.5)*g_gridSpacing,
								   y=((((i-1)-(i-1)%10)/10)%10 - 4.5)*g_gridSpacing,
								   z=(((i-1)-(i-1)%10-(i-1)%100)/100 - 4.5)*g_gridSpacing})
		entity_setPosition(boxEntity, box.position)
		entity_setOrientation(boxEntity, {w=1, x=0, y=0, z=0})

		createBoxEntry(boxEntity, box.position)

		box.velocity = {x=0, y=0, z=0}
		box.aabb = G_BOX_BB

		g_boxes[i] = box
	end
end
