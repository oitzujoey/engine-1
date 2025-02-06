G_CLIENT=true

include "../common/common.lua"
include "../client/keys.lua"
include "../common/loadworld.lua"

Keys = {}
g_mouse = {}

g_sensitivity = 1.0

g_cursorOffset = {x=0, y=0, z=100}


g_initialBoxesCreated = false
function processEvents()
	local createdBoxes = false
	if not serverState.events then return end
	local events_length = #serverState.events
	for i = 1,events_length,1 do
		local event = serverState.events[i]
		local c = event.command
		local d = event.data
		if c == "create box" then
			createBox(d.position, d.materialName)
		elseif c == "create initial box" then
			if not g_initialBoxesCreated then
				createBox(d.position, d.materialName)
				createdBoxes = true
			end
		elseif c == "no initial boxes" then
			if not g_initialBoxesCreated then
				createdBoxes = true
			end
		else
			warning("processEvents", "Unrecognized event \""..c.."\"")
		end
	end
	if createdBoxes then
		clientState.boxesCreated = true
		g_initialBoxesCreated = true
	end
	serverState.events = {}
end


function dumpBoxes()
	for i = 1,g_boxes_length,1 do
		local box = g_boxes[i]
		puts("box {")
		local p = box.position
		puts(toString(p.x).." "..toString(p.y).." "..toString(p.z))
		puts("}")
	end
end


function startup()
	local e
	info("startup", "Lua start");

	info("startup", "Loading models")

	loadWorld()

	-- Load a .png texture from "textures/".
	function loadTexture(name)
		local e = false
		material, e = material_create()
		if e ~= 0 then return nil, e end
		texture, e = material_loadTexture("textures/"..name..".png")
		if e ~= 0 then return nil, e end
		e = material_linkTexture(material, texture)
		if e ~= 0 then return nil, e end
		return material, nil
	end

	function loadMaterial(name)
		local material, e = loadTexture(name)
		g_materials[name] = material
		return material, e
	end

	redMaterial, error = loadMaterial("red")
	greenMaterial, error = loadMaterial("green")
	blueMaterial, error = loadMaterial("blue")
	whiteMaterial, error = loadMaterial("white")
	blackMaterial, error = loadMaterial("black")
	cyanMaterial, error = loadMaterial("cyan")
	magentaMaterial, error = loadMaterial("magenta")
	yellowMaterial, error = loadMaterial("yellow")

	cardboardBoxMaterial, error = loadTexture("box")

	-- World box
	local sandboxModel, e = mesh_load("blender/sandbox")
	if e ~= 0 then quit() end
	local sandboxEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, sandboxEntity)
	if e ~= 0 then quit() end
	e = entity_linkChild(sandboxEntity, sandboxModel)
	if e ~= 0 then quit() end
	entity_setScale(sandboxEntity, 10*g_boundingBoxRadius)
	entity_setOrientation(sandboxEntity, aaToQuat({w=G_PI/2, x=1, y=0, z=0}))
	local sandboxMaterial, e = loadTexture("lava")
	e = model_linkDefaultMaterial(sandboxModel, sandboxMaterial)

	-- Ground
	local planeModel, e = mesh_load("blender/plane")
	if e ~= 0 then quit() end
	local planeEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, planeEntity)
	if e ~= 0 then quit() end
	e = entity_linkChild(planeEntity, planeModel)
	if e ~= 0 then quit() end
	entity_setScale(planeEntity, g_boundingBoxRadius + g_gridSpacing/2)
	entity_setPosition(planeEntity, {x=0, y=0, z=-(g_boundingBoxRadius/2 + g_gridSpacing/2)})
	entity_setOrientation(planeEntity, {w=1, x=1, y=0, z=0})
	local groundMaterial, e = loadTexture("floor")
	e = model_linkDefaultMaterial(planeModel, groundMaterial)
	if e ~= 0 then quit() end

	cursorMaterial, error = loadTexture("cursor")
	g_cursorEntity = modelEntity_create({x=0, y=0, z=0}, {w=1, x=0, y=0, z=0}, g_boxes_scale * 1.1)
	e = entity_linkMaterial(g_cursorEntity, cursorMaterial)

	do
		local e = nil
		for i = 1,g_boxes_length,1 do
			local material = cardboardBoxMaterial
			e = entity_linkMaterial(g_boxes[i].entity, material)
		end
	end

	-- Left arrow
	keys_createFullBind("k_1073741903", "key_left",			"key_left_d",		"key_left_u")
	-- Right arrow
	keys_createFullBind("k_1073741904", "key_right",		"key_right_d",		"key_right_u")
	-- Up arrow
	keys_createFullBind("k_1073741906", "key_up",			"key_up_d",			"key_up_u")
	-- Down arrow
	keys_createFullBind("k_1073741905", "key_down",			"key_down_d",		"key_down_u")
	-- a
	keys_createFullBind("k_97", "key_strafeLeft", "key_strafeLeft_d", "key_strafeLeft_u")
	-- d
	keys_createFullBind("k_100", "key_strafeRight", "key_strafeRight_d", "key_strafeRight_u")
	-- w
	keys_createFullBind("k_119", "key_forward",	"key_forward_d", "key_forward_u")
	-- s
	keys_createFullBind("k_115", "key_backward", "key_backward_d", "key_backward_u")
	-- Left mouse
	keys_createFullBind("m_1", "mouse_leftButton", "mouse_leftPress", "mouse_leftRelease")
	-- Right mouse
	keys_createFullBind("m_3", "mouse_rightButton", "mouse_rightPress", "mouse_rightRelease")
	-- Mouse x-y
	keys_createMouseBind("mouse_motion")
	-- q
	cfg2_setVariable("bind k_113 quit")

	createConsoleCommand("dump_boxes", "dumpBoxes")

	-- Create keys/buttons
	clientState.keys = {}
	clientState.keys.up = false
	clientState.keys.down = false
	clientState.keys.left = false
	clientState.keys.right = false
	clientState.keys.strafeLeft = false
	clientState.keys.strafeRight = false
	clientState.keys.forward = false
	clientState.keys.backward = false

	Keys.up = false
	Keys.down = false
	Keys.left = false
	Keys.right = false
	Keys.strafeLeft = false
	Keys.strafeRight = false
	Keys.forward = false
	Keys.backward = false

	g_mouse = {x=nil, y=nil, delta_x=0, delta_y=0}
	clientState.mouse = g_mouse

	orientation = {w=1, x=0, y=0, z=0}
	position = {x=0, y=0, z=0}

	info("startup", "Starting game")
end


initialized = false
function init()
	if serverState.maxClients ~= nil then
		clientEntities = {}
		for j = 1,serverState.maxClients,1 do
			clientEntities[j] = nil
		end

		initialized = true
	end
end


function main()
	local error
	if not initialized then
		init()
		return
	end

	processEvents()

	clientState.mouse = {x=nil, y=nil, delta_x=0, delta_y=0}

	-- Display self.
	if (serverState.position ~= nil) then
		position.x = -serverState.position.x
		position.y = -serverState.position.y
		position.z = -serverState.position.z
		orientation.w = serverState.orientation.w
		orientation.x = -serverState.orientation.x
		orientation.y = -serverState.orientation.y
		orientation.z = -serverState.orientation.z
		entity_setOrientation(g_worldEntity, orientation)
		entity_setPosition(g_cameraEntity, position)
	end

	local cursorPosition = snapToGrid(vec3_add(serverState.position,
											   vec3_rotate(g_cursorOffset, serverState.orientation)))
	entity_setPosition(g_cursorEntity, cursorPosition)

	processBoxes(g_boxes)

	g_frame = g_frame + 1
end

function shutdown()
	info("shutdown", "Client quit")
end
