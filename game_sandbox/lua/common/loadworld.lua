
worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}

TurnRate = 0.01

g_boxes_length = 0
g_boxes_scale = g_gridSpacing/2


g_boxes = {}


function loadWorld()
	local e
	g_cameraEntity = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_worldEntity, g_cameraEntity)
	if e ~= 0 then quit() end

	boxModel, e = mesh_load("blender/cube")
	if e ~= 0 then quit() end
	-- Enable GPU instancing for this model.
	e = model_setInstanced(boxModel, true)
	if e ~= 0 then quit() end
end
