G_CLIENT=true

include "../common/common.lua"
include "../client/keys.lua"

g_sensitivity = 1.0

g_cursorScale = 1.05
g_selectionScale = 1.1

G_STANDARD_FRAMERATE = 60

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

g_frame = 0
g_averageFramerate = 0.0
g_time = 0.0

g_initialBoxesToCreate = {}

g_initialBoxesCreated = false
function processEvents(events)
	local createdBoxes = false
	if not events then return end
	local events_length = #events
	for i = 1,events_length,1 do
		local event = events[i]
		local c = event.command
		local d = event.data
		if c == "create box" then
			createBox(nil, d.position, d.materialName)
		elseif c == "create initial box" then
			if not g_initialBoxesCreated then
				push(g_initialBoxesToCreate, d)
				createdBoxes = true
			end
		elseif c == "no initial boxes" then
			if not g_initialBoxesCreated then
				createdBoxes = true
			end
		elseif c == "change box color" then
			changeBoxMaterial(g_boxes[getBoxEntry(d.position)], d.color)
		elseif c == "move box" then
			-- Move box.
			local box_index = getBoxEntry(d.start_position)
			g_boxes[box_index].position = d.end_position
			moveBox(box_index, d.end_position, d.start_position)
			-- Enable physics.
			g_boxes[box_index].needsUpdate = true
			entity_setPosition(g_boxes[box_index].entity, g_boxes[box_index].position)

			updateNeighborBoxes(d.start_position)
		else
			warning("processEvents", "Unrecognized event \""..c.."\"")
		end
	end
	if createdBoxes then
		clientState.boxesCreated = true
		g_initialBoxesCreated = true
		local boxes_length = #g_boxes
		for i = 1,boxes_length,1 do
			local p = g_boxes[i].position
			puts("Check: "..toString(p.x).." "..toString(p.y).." "..toString(p.z))
			if checkIfBoxNeedsUpdate(p) then
				puts("Update: "..toString(p.x).." "..toString(p.y).." "..toString(p.z))
				g_boxes[i].needsUpdate = true
			end
		end
	end
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

	g_cameraEntity = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_worldEntity, g_cameraEntity)
	if e ~= 0 then quit() end

	g_boxModel, e = mesh_load("blender/cube")
	if e ~= 0 then quit() end

	g_frogModel, e = mesh_load("blender/frogcube")
	if e ~= 0 then quit() end

	function loadShader(name)
		return shader_create("shaders/"..name)
	end
	-- Load a .png texture from "textures/" as a new material.
	function loadMaterial(shader, name)
		return material_create(shader, "textures/"..name)
	end

	local cubeShader, e = loadShader("cube")
	if e ~= 0 then quit() end
	local defaultShader, e = loadShader("default")
	if e ~= 0 then quit() end
	-- cubeShader = defaultShader

	function loadCubeMaterial(name, file, model, transparent)
		local shader = nil
		if transparent then
			shader = defaultShader
		else
			shader = cubeShader
		end
		local material, e = loadMaterial(shader, file)
		g_materials[name] = material
		g_modelForMaterial[name] = model
		return material, e
	end

	redMaterial, e = loadCubeMaterial("red", "red.png", g_boxModel, false)
	if e ~= 0 then quit() end
	greenMaterial, e = loadCubeMaterial("green", "green.png", g_boxModel, false)
	if e ~= 0 then quit() end
	blueMaterial, e = loadCubeMaterial("blue", "blue.png", g_boxModel, false)
	if e ~= 0 then quit() end
	whiteMaterial, e = loadCubeMaterial("white", "white.png", g_boxModel, false)
	if e ~= 0 then quit() end
	blackMaterial, e = loadCubeMaterial("black", "black.png", g_boxModel, false)
	if e ~= 0 then quit() end
	cyanMaterial, e = loadCubeMaterial("cyan", "cyan.png", g_boxModel, false)
	if e ~= 0 then quit() end
	magentaMaterial, e = loadCubeMaterial("magenta", "magenta.png", g_boxModel, false)
	if e ~= 0 then quit() end
	yellowMaterial, e = loadCubeMaterial("yellow", "yellow.png", g_boxModel, false)
	if e ~= 0 then quit() end
	clearMaterial, e = loadCubeMaterial("clear", "clear.png", g_boxModel, true)
	if e ~= 0 then quit() end
	frogMaterial, e = loadCubeMaterial("frog", "frogcube.png", g_frogModel, false)
	if e ~= 0 then quit() end
	-- e = material_setCull(clearMaterial, false)
	-- if e ~= 0 then quit() end


	-- World box
	local sandboxModel, e = mesh_load("blender/sandbox")
	if e ~= 0 then quit() end
	local sandboxEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, sandboxEntity)
	if e ~= 0 then quit() end
	e = entity_linkChild(sandboxEntity, sandboxModel)
	if e ~= 0 then quit() end
	entity_setScale(sandboxEntity, 5*g_boundingBoxRadius)
	entity_setOrientation(sandboxEntity, aaToQuat({w=G_PI/2, x=1, y=0, z=0}))
	local sandboxMaterial, e = loadMaterial(defaultShader, "lava.png")
	if e ~= 0 then quit() end
	material_setDepthSort(sandboxMaterial, false)
	e = model_linkDefaultMaterial(sandboxModel, sandboxMaterial)
	if e ~= 0 then quit() end

	-- Ground
	g_planeModel, e = mesh_load("blender/plane")
	if e ~= 0 then quit() end
	g_groundMaterial, e = loadMaterial(defaultShader, "floor.png")
	if e ~= 0 then quit() end
	material_setDepthSort(g_groundMaterial, false)

	g_cursorMaterial, e = loadMaterial(defaultShader, "cursor.png")
	if e ~= 0 then quit() end

	g_selectionMaterial, e = loadMaterial(defaultShader, "selection.png")
	if e ~= 0 then quit() end

	g_loadingMaterial, e = loadMaterial(defaultShader, "loading.png")
	if e ~= 0 then quit() end

	-- a, left array
	keys_createFullBind("k_100", "key_strafeLeft", "key_strafeLeft_d", "key_strafeLeft_u")
	keys_createFullBind("k_1073741903", "key_leftarrow", "key_strafeLeft_d", "key_strafeLeft_u")
	-- d, right arrow
	keys_createFullBind("k_97", "key_strafeRight", "key_strafeRight_d", "key_strafeRight_u")
	keys_createFullBind("k_1073741904", "key_rightarrow", "key_strafeRight_d", "key_strafeRight_u")
	-- w, up arrow
	keys_createFullBind("k_119", "key_backward", "key_backward_d", "key_backward_u")
	keys_createFullBind("k_1073741906", "key_downarrow", "key_backward_d", "key_backward_u")
	-- s, down arrow
	keys_createFullBind("k_115", "key_forward", "key_forward_d", "key_forward_u")
	keys_createFullBind("k_1073741905", "key_uparrow", "key_forward_d", "key_forward_u")
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
	-- Space (jump)
	keys_createFullBind("k_32", "key_space", "key_space_d", "key_space_u")
	-- Left mouse (select)
	keys_createFullBind("m_1", "mouse_leftButton", "mouse_leftPress", "mouse_leftRelease")
	-- Right mouse (jump)
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

	g_sensitivity, e = cfg2_getVariable("sensitivity")
	if e ~= 0 then quit() end


	g_username, e = cfg2_getVariable("username")
	if e ~= 0 then
		critical_error("startup", "`username` is not set!")
		quit()
	end
	g_password, e = cfg2_getVariable("password")
	if e ~= 0 then
		critical_error("startup", "`password` is not set!")
		quit()
	end
	info("startup", "username: "..g_username)

	g_authenticated = false

	info("startup", "Starting game")

	g_loadingScreen = aaToQuat({w=5*G_PI/4, x=0, y=0, z=1})
	g_loadingEntity = modelEntity_create({x=0, y=0, z=-100}, g_loadingScreen, g_boxes_scale)
	e = entity_linkMaterial(g_loadingEntity, g_loadingMaterial)
end

function mainGame()
	local e

	local movementScale = G_STANDARD_FRAMERATE * deltaT

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
		local occupied = getBoxEntry(cursorPosition)
		if g_selectedPosition and not occupied then
			e = modelEntity_delete(g_selectionEntity)
			if e ~= 0 then quit() end
			client_sendEvent("move box",
							 {start_position=g_selectedPosition,
							  end_position={x=cursorPosition.x, y=cursorPosition.y, z=cursorPosition.z + g_backOff}})
			g_selectedPosition = nil

			e = modelEntity_delete(g_cursorEntity)
			if e ~= 0 then quit() end
			g_cursorEntity = modelEntity_create(cursorPosition, {w=1, x=0, y=0, z=0}, g_boxes_scale * g_cursorScale)
			e = entity_linkMaterial(g_cursorEntity, g_cursorMaterial)
			if e ~= 0 then quit() end
		elseif not g_selectedPosition and occupied then
			g_selectedPosition = cursorPosition
			g_selectionEntity = modelEntity_create(g_selectedPosition,
												   {w=1, x=0, y=0, z=0},
												   g_boxes_scale * g_selectionScale)
			e = entity_linkMaterial(g_selectionEntity, g_selectionMaterial)
		elseif g_selectedPosition and occupied and vec3_equal(cursorPosition, g_selectedPosition) then
			e = modelEntity_delete(g_selectionEntity)
			if e ~= 0 then quit() end
			g_selectedPosition = nil
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
			if g_selectedPosition then
				e = entity_linkMaterial(g_cursorEntity, g_selectionMaterial)
			else
				e = entity_linkMaterial(g_cursorEntity, g_cursorMaterial)
			end
			if e ~= 0 then quit() end
		end
	end
		

	-- Movement

	-- Friction
	local scale = 1.0 - (1.0 - 0.90) * movementScale
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
		yaw_x = yaw_x + cos(g_playerState.euler.yaw)
		yaw_y = yaw_y + sin(g_playerState.euler.yaw)
	end
	if g_strafeRight then
		yaw_x = yaw_x - cos(g_playerState.euler.yaw)
		yaw_y = yaw_y - sin(g_playerState.euler.yaw)
	end
	g_playerState.velocity = vec3_add(g_playerState.velocity, vec3_scale({x=yaw_x, y=yaw_y, z=0}, movementScale))

	if (g_mouse.delta_y and g_mouse.delta_y ~= 0) then
		g_playerState.euler.pitch = g_playerState.euler.pitch - g_mouse.delta_y/1000.0
	end
	if (g_mouse.delta_x and g_mouse.delta_x ~= 0) then
		g_playerState.euler.yaw = g_playerState.euler.yaw - g_mouse.delta_x/1000.0
	end

	-- Constrain up-down view to not go past vertical.
	if g_playerState.euler.pitch > G_PI/2 then g_playerState.euler.pitch = G_PI/2 end
	if g_playerState.euler.pitch < -G_PI/2 then g_playerState.euler.pitch = -G_PI/2 end

	g_playerState.velocity.z = g_playerState.velocity.z + G_GRAVITY * movementScale

	g_playerState.orientation = hamiltonProduct(eulerToQuat(g_playerState.euler), aaToQuat({w=G_PI/2, x=1, y=0, z=0}))
	g_playerState = playerMoveAndCollide(g_playerState, movementScale)

	-- Display self.
	entity_setOrientation(g_worldEntity, {w=g_playerState.orientation.w,
	                                      x=-g_playerState.orientation.x,
	                                      y=-g_playerState.orientation.y,
	                                      z=-g_playerState.orientation.z})
	entity_setPosition(g_cameraEntity,
	                   {x=-g_playerState.position.x, y=-g_playerState.position.y, z=-g_playerState.position.z})


	processBoxes(g_boxes)
end

function loadingScreen()
	local scale = 3.0
	local angleVector = {x=0.0, y=0.0, z=0.0}
	if g_mouse.delta_x then
		angleVector.x = scale*g_mouse.delta_x/1000.0
	end
	if g_mouse.delta_y then
		angleVector.y = scale*g_mouse.delta_y/1000.0
	end
	local angle = vec3_norm(angleVector)
	local axisAngle = vec3_scale(angleVector, 1.0/angle)
	axisAngle.w = angle
	if angle ~= 0 then
		g_loadingScreen = hamiltonProduct(g_loadingScreen, aaToQuat(axisAngle))
		entity_setOrientation(g_loadingEntity, g_loadingScreen)
	end
end

g_wasLoading = true
function main()
	local e

	if not g_authenticated then
		clientState.username = g_username
		clientState.password = g_password
	end

	local messages = serverState
	local messages_length = #messages
	for messages_index = 1,messages_length,1 do
		processEvents(messages[messages_index].events)
	end

	if g_initialBoxesCreated then
		if g_wasLoading then
			g_wasLoading = false
			
			-- Do final game setup:

			-- The order things are created in is important because my transparency is broken because I don't sort by z value.

			-- Create ground.
			local planeEntity, e = entity_createEntity(g_entity_type_model)
			e = entity_linkChild(g_cameraEntity, planeEntity)
			if e ~= 0 then quit() end
			e = entity_linkChild(planeEntity, g_planeModel)
			if e ~= 0 then quit() end
			entity_setScale(planeEntity, g_boundingBoxRadius + g_gridSpacing/2)
			entity_setPosition(planeEntity, {x=0, y=0, z=-(g_boundingBoxRadius/2 + g_gridSpacing/2)})
			entity_setOrientation(planeEntity, {w=1, x=1, y=0, z=0})
			e = model_linkDefaultMaterial(g_planeModel, g_groundMaterial)
			if e ~= 0 then quit() end

			-- Create cursor.
			g_cursorEntity = modelEntity_create({x=0, y=0, z=0}, {w=1, x=0, y=0, z=0}, g_boxes_scale * g_cursorScale)
			e = entity_linkMaterial(g_cursorEntity, g_cursorMaterial)
			if e ~= 0 then quit() end

			-- Create boxes.
			do
				local boxes_length = #g_initialBoxesToCreate
				for box_index = 1,boxes_length,1 do
					local boxDescriptor = g_initialBoxesToCreate[box_index]
					createBox(nil, boxDescriptor.position, boxDescriptor.materialName)
				end
				g_initialBoxesToCreate = nil
			end

			-- Delete loading cube.
			e = modelEntity_delete(g_loadingEntity)
			if e ~= 0 then quit() end

			info("main", "Loaded cubes.")
		end

		mainGame()
	else -- Loading screen.
		loadingScreen()
	end

	client_sendQueuedEvents()

	g_averageFramerate = g_averageFramerate + (1/deltaT - g_averageFramerate)/1000
	g_frame = g_frame + 1
	g_time = g_time + deltaT
	g_mouse = {x=nil, y=nil, delta_x=0, delta_y=0}
end

function shutdown()
	info("shutdown", "Client quit")
	puts("FPS: "..toString(g_averageFramerate))
end
