G_CLIENT=true

include "../common/common.lua"
include "../client/keys.lua"
include "../common/loadworld.lua"

g_sensitivity = 1.0

g_cursorScale = 1.05
g_selectionScale = 1.1

Keys = {}
g_mouse = {}
g_eventQueue = {}

function client_sendEvent(command, data)
	local event = {command=command, data=data}
	push(g_eventQueue, event)
end

function client_sendQueuedEvents()
	clientState.events = g_eventQueue
	g_eventQueue = {}
end


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
		elseif c == "change box color" then
			changeBoxMaterial(g_boxes[getBoxEntry(d.position)], d.color)
		elseif c == "move box" then
			local box_index = getBoxEntry(d.start_position)
			g_boxes[box_index].position = d.end_position
			moveBox(box_index, d.end_position, d.start_position)
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

	g_cursorMaterial, error = loadTexture("cursor")
	g_cursorEntity = modelEntity_create({x=0, y=0, z=0}, {w=1, x=0, y=0, z=0}, g_boxes_scale * g_cursorScale)
	e = entity_linkMaterial(g_cursorEntity, g_cursorMaterial)

	g_selectionMaterial, error = loadTexture("selection")

	-- a
	keys_createFullBind("k_97", "key_strafeLeft", "key_strafeLeft_d", "key_strafeLeft_u")
	-- d
	keys_createFullBind("k_100", "key_strafeRight", "key_strafeRight_d", "key_strafeRight_u")
	-- w
	keys_createFullBind("k_119", "key_forward",	"key_forward_d", "key_forward_u")
	-- s
	keys_createFullBind("k_115", "key_backward", "key_backward_d", "key_backward_u")
	-- Box colors
	keys_createHalfBind("k_49", "key_color1", "key_color1_d")
	keys_createHalfBind("k_50", "key_color2", "key_color2_d")
	keys_createHalfBind("k_51", "key_color3", "key_color3_d")
	keys_createHalfBind("k_52", "key_color4", "key_color4_d")
	keys_createHalfBind("k_53", "key_color5", "key_color5_d")
	keys_createHalfBind("k_54", "key_color6", "key_color6_d")
	keys_createHalfBind("k_55", "key_color7", "key_color7_d")
	keys_createHalfBind("k_56", "key_color8", "key_color8_d")
	keys_createHalfBind("k_57", "key_color9", "key_color9_d")
	keys_createHalfBind("k_48", "key_color0", "key_color0_d")
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
	clientState.keys.color = nil

	Keys.up = false
	Keys.down = false
	Keys.left = false
	Keys.right = false
	Keys.strafeLeft = false
	Keys.strafeRight = false
	Keys.forward = false
	Keys.backward = false
	Keys.color = nil
	g_jump = false
	g_forward = false
	g_backward = false
	g_strafeLeft = false
	g_strafeRight = false
	g_selectCube = false

	g_mouse = {x=nil, y=nil, delta_x=0, delta_y=0}

	g_playerState = {}
	g_playerState.orientation = {w=1, x=0, y=0, z=0}
	g_playerState.position = {x=0, y=0, z=0}
	g_playerState.velocity = {x=0, y=0, z=0}
	g_playerState.grounded = false
	g_playerState.collided = false
	g_playerState.euler = {yaw=0, pitch=0}
	g_playerState.aabb = G_PLAYER_BB

	g_selectedBox = nil

	info("startup", "Starting game")
end

function main()
	local e

	processEvents()


	-- Box manipulation

	local cursorPosition = calculateCursorPosition(g_playerState.position, g_playerState.orientation)

	-- Change box color
	if Keys.color then
		local color = g_materialNames[Keys.color]
		if color then
			client_sendEvent("change box color", {position=cursorPosition, color=color})
		end
	end
	Keys.color = nil

	-- Move box
	if g_selectCube then
		g_selectCube = false
		local occupied, _ = isOccupied(cursorPosition)
		if g_selectedPosition and not occupied then
			e = modelEntity_delete(g_selectionEntity)
			if e ~= 0 then quit() end
			client_sendEvent("move box",
							 {start_position=g_selectedPosition,
							  end_position={x=cursorPosition.x, y=cursorPosition.y, z=cursorPosition.z + g_backOff}})
			g_selectedPosition = nil
		elseif not g_selectedPosition and occupied then
			g_selectedPosition = cursorPosition
			g_selectionEntity = modelEntity_create(g_selectedPosition,
												   {w=1, x=0, y=0, z=0},
												   g_boxes_scale * g_selectionScale)
			e = entity_linkMaterial(g_selectionEntity, g_selectionMaterial)
		end
	end

	-- Cursor
	if g_selectedPosition and vec3_equal(cursorPosition, g_selectedPosition) then
		if g_cursorEntity then
			modelEntity_delete(g_cursorEntity)
			g_cursorEntity = nil
		end
	else
		if g_cursorEntity then
			entity_setPosition(g_cursorEntity, cursorPosition)
		else
			g_cursorEntity = modelEntity_create(cursorPosition, {w=1, x=0, y=0, z=0}, g_boxes_scale * g_cursorScale)
			e = entity_linkMaterial(g_cursorEntity, g_cursorMaterial)
		end
	end
		

	-- Movement

	-- Friction
	local scale = 0.90
	g_playerState.velocity.x = g_playerState.velocity.x * scale
	g_playerState.velocity.y = g_playerState.velocity.y * scale

	-- Jumping
	if g_playerState.grounded then
		if g_jump then
			g_playerState.velocity.z = g_playerState.velocity.z + G_JUMPVELOCITY
		end
	end

	-- Horizontal movement
	local yaw_x, yaw_y = 0, 0
	if g_forward then
		yaw_x = yaw_x + sin(g_playerState.euler.yaw)
		yaw_y = yaw_y - cos(g_playerState.euler.yaw)
	end
	if g_backward then
		yaw_x = yaw_x - sin(g_playerState.euler.yaw)
		yaw_y = yaw_y + cos(g_playerState.euler.yaw)
	end
	if g_strafeLeft then
		yaw_x = yaw_x - cos(g_playerState.euler.yaw)
		yaw_y = yaw_y - sin(g_playerState.euler.yaw)
	end
	if g_strafeRight then
		yaw_x = yaw_x + cos(g_playerState.euler.yaw)
		yaw_y = yaw_y + sin(g_playerState.euler.yaw)
	end
	g_playerState.velocity = vec3_add(g_playerState.velocity, {x=yaw_x, y=yaw_y, z=0})

	if (g_mouse.delta_y and g_mouse.delta_y ~= 0) then
		g_playerState.euler.pitch = g_playerState.euler.pitch + g_mouse.delta_y/1000.0
	end
	if (g_mouse.delta_x and g_mouse.delta_x ~= 0) then
		g_playerState.euler.yaw = g_playerState.euler.yaw + g_mouse.delta_x/1000.0
	end

	-- Constrain up-down view to not go past vertical.
	if g_playerState.euler.pitch > G_PI/2 then g_playerState.euler.pitch = G_PI/2 end
	if g_playerState.euler.pitch < -G_PI/2 then g_playerState.euler.pitch = -G_PI/2 end

	g_playerState.velocity.z = g_playerState.velocity.z + G_GRAVITY

	g_playerState.orientation = hamiltonProduct(eulerToQuat(g_playerState.euler), aaToQuat({w=G_PI/2, x=1, y=0, z=0}))
	g_playerState = playerMoveAndCollide(g_playerState)

	-- Display self.
	entity_setOrientation(g_worldEntity, {w=g_playerState.orientation.w,
										  x=-g_playerState.orientation.x,
										  y=-g_playerState.orientation.y,
										  z=-g_playerState.orientation.z})
	entity_setPosition(g_cameraEntity,
					   {x=-g_playerState.position.x, y=-g_playerState.position.y, z=-g_playerState.position.z})


	processBoxes(g_boxes)

	client_sendQueuedEvents()

	g_frame = g_frame + 1
	g_mouse = {x=nil, y=nil, delta_x=0, delta_y=0}
end

function shutdown()
	info("shutdown", "Client quit")
end
