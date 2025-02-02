include "../common/common.lua"
include "../client/keys.lua"
include "../common/loadworld.lua"

Keys = {}
g_mouse = {}

g_sensitivity = 1.0

g_cursorOffset = {x=0, y=0, z=100}

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
	redMaterial, error = loadTexture("red")
	greenMaterial, error = loadTexture("green")
	blueMaterial, error = loadTexture("blue")
	whiteMaterial, error = loadTexture("white")
	blackMaterial, error = loadTexture("black")
	cyanMaterial, error = loadTexture("cyan")
	magentaMaterial, error = loadTexture("magenta")
	yellowMaterial, error = loadTexture("yellow")
	cardboardBoxMaterial, error = loadTexture("box")

	sandboxMaterial, error = loadTexture("lava")
	e = model_linkDefaultMaterial(g_sandboxModel, sandboxMaterial)

	groundMaterial, error = loadTexture("floor")
	e = model_linkDefaultMaterial(g_planeModel, groundMaterial)

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
