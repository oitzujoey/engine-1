G_CLIENT=true

include "../common/common.lua"
include "../client/keys.lua"
include "../generated/initial.lua"
include "gcode.lua"

g_sensitivity = 1.0
g_fly = false

Keys = {}
g_mouse = {}

g_averageFramerate = 0


function draw_line(point1, point2, scale)
	local line_entity, e = entity_createEntity(g_entity_type_model)
	if e ~= 0 then quit() end
	e = entity_linkChild(g_cameraEntity, line_entity)
	if e ~= 0 then quit() end
	e = entity_linkChild(line_entity, g_cube_model)
	if e ~= 0 then quit() end
	entity_setScale(line_entity, scale)
	entity_setPosition(line_entity, {x=X, y=(1000 + Y), z=Z})
	entity_setOrientation(line_entity, {w=1, x=1, y=1, z=1})
	e = entity_linkMaterial(line_entity, g_line_material)
	if e ~= 0 then quit() end
	return entity
end


function startup()
	local e
	info("startup", "Lua start");

	info("startup", "Loading models")

	g_cameraEntity = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_worldEntity, g_cameraEntity)
	if e ~= 0 then quit() end

	g_airliner_model, e = mesh_load("models/airliner")
	if e ~= 0 then quit() end

	g_cube_model, e = mesh_load("models/cube")
	if e ~= 0 then quit() end

	g_map_model, e = mesh_load("models/map")
	if e ~= 0 then quit() end

	function loadShader(name)
		return shader_create("shaders/"..name)
	end
	-- Load a .png texture from "textures/" as a new material.
	function loadMaterial(shader, name)
		local material, e = material_create(shader, "textures/"..name)
		return material, e
	end

	local defaultShader, e = loadShader("default")
	if e ~= 0 then quit() end
	g_airliner_material, e = loadMaterial(defaultShader, "airliner.png")
	g_airliner_entity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, g_airliner_entity)
	if e ~= 0 then quit() end
	e = entity_linkChild(g_airliner_entity, g_cube_model)
	if e ~= 0 then quit() end
	g_airliner_scale = 0.25
	entity_setScale(g_airliner_entity, g_airliner_scale)
	entity_setPosition(g_airliner_entity, {x=0, y=0, z=0})
	entity_setOrientation(g_airliner_entity, {w=1, x=1, y=1, z=1})
	e = entity_linkMaterial(g_airliner_entity, g_airliner_material)
	if e ~= 0 then quit() end

	g_map_material, e = loadMaterial(defaultShader, "map-pin.png")
	g_map_entity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, g_map_entity)
	if e ~= 0 then quit() end
	e = entity_linkChild(g_map_entity, g_map_model)
	if e ~= 0 then quit() end
	g_map_scale = 100.0
	entity_setScale(g_map_entity, g_map_scale)
	entity_setPosition(g_map_entity, {x=0, y=0, z=-1})
	entity_setOrientation(g_map_entity, {w=1, x=1, y=1, z=1})
	e = entity_linkMaterial(g_map_entity, g_map_material)
	if e ~= 0 then quit() end


	-- e = material_setCull(clearMaterial, false)
	-- if e ~= 0 then quit() end


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
	-- Ctrl (crouch)
	keys_createFullBind("k_1073742048", "key_ctrl", "key_ctrl_d", "key_ctrl_u")
	keys_createFullBind("k_99", "key_c", "key_c_d", "key_c_u")
	-- Fly toggle
	keys_createFullBind("k_102", "key_f", "key_f_d", "key_f_u")
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

	local function read()
		local buffer, e = namedPipe_readAsString()
		if e ~= 0 then  return nil  end
		if buffer == "" then  return nil  end
		return buffer
	end
	gcode.register_reader(read)
	g_print = true
	g_head_position = {x=0, y=0, z=0}

	g_tokens = {}
	g_tokens_entity = {}
	for i=1,#g_tokens_position_initial do
		local position_tuple = g_tokens_position_initial[i]
		local token = {x=10*position_tuple[1], y=10*position_tuple[3]}
		g_tokens[i] = token

		local new_entity, e = entity_createEntity(g_entity_type_model)
		if e ~= 0 then quit() end
		e = entity_linkChild(g_cameraEntity, new_entity)
		if e ~= 0 then quit() end
		e = entity_linkChild(new_entity, g_airliner_model)
		if e ~= 0 then quit() end
		entity_setScale(new_entity, g_airliner_scale)
    	local aircraft_position = {x=token["x"], y=token["y"], z=2.0}
		entity_setPosition(new_entity, aircraft_position)
		entity_setOrientation(new_entity, {w=1, x=1, y=1, z=1})
		e = entity_linkMaterial(new_entity, g_airliner_material)
		if e ~= 0 then quit() end
		g_tokens_entity[i] = new_entity
	end

	info("startup", "Starting game")
end

function mainGame()
	local e

	-- Plane position

	if g_print then
		local command = gcode.getNextCommand_nonblocking()
		if command then
			if command.opcode == "PRINT_END" then
				g_print = false
				puts("PRINT_END")
				quit()
			end
			puts("opcode: "..command.opcode)
			local X = parse_double(command.X)
			local Y = parse_double(command.Y)
			local Z = parse_double(command.Z)
			local E = parse_double(command.E)
			local head_position_last = g_head_position
			g_head_position = {x=X, y=Y, z=Z}
			-- puts(parse_double(command.X)..", "..parse_double(command.Y)..", "..parse_double(command.Z))
			local l_X = head_position_last.x
			local l_Y = head_position_last.y
			local l_Z = head_position_last.z
			local aircraft_displacement = {x=X-l_X, y=Y-l_Y, z=Z-l_Z}
			vec3_print(aircraft_displacement)

			if E > 0.0 then
				puts("Processing...")
				puts("original position:")
				vec2_print({x=l_X, y=l_Y})
				local token_index = nil
				for index=1,#g_tokens do
					local token = g_tokens[index]
					puts("token "..toString(index)..":")
					vec2_print(token)
					local dist = vec2_dist(token, {x=l_X, y=l_Y})
					puts("dist: "..toString(dist))
					-- vec2_print(dist)
					if vec2_dist(token, {x=l_X, y=l_Y}) < 1.0 then
						token_index = index
						break
					end
				end
				if not token_index then
					abort("Lost a token!")
				end
				g_tokens[token_index] = {x=X, y=Y}
				if vec3_norm2(aircraft_displacement) ~= 0.0 then
					-- Resource leak:
					aircraft_entity = g_tokens_entity[token_index]
					-- local aircraft_entity, e = entity_createEntity(g_entity_type_model)
					-- if e ~= 0 then quit() end
					-- e = entity_linkChild(g_cameraEntity, aircraft_entity)
					-- if e ~= 0 then quit() end
					-- e = entity_linkChild(aircraft_entity, g_airliner_model)
					-- if e ~= 0 then quit() end
					-- entity_setScale(aircraft_entity, g_airliner_scale)
                    local aircraft_position = {x=g_head_position['x'], y=g_head_position['y'], z=g_head_position['z'] + 2.0}
					entity_setPosition(aircraft_entity, aircraft_position)

					local aircraft_heading_normal = vec3_normalize(aircraft_displacement)
					local aircraft_heading_yaw = -atan2(-aircraft_heading_normal.x,
					                                    -aircraft_heading_normal.y)
					local aircraft_heading_aa = {w=aircraft_heading_yaw, x=0, y=1, z=0}
					local aircraft_heading_quat = aaToQuat(aircraft_heading_aa)
					local base_orientation = {w=1, x=1, y=1, z=1}
					local orientation = hamiltonProduct(base_orientation, aircraft_heading_quat)
					entity_setOrientation(aircraft_entity, orientation)

					-- entity_setOrientation(aircraft_entity, {w=1, x=1, y=1, z=1})
					-- e = entity_linkMaterial(aircraft_entity, g_airliner_material)
					-- if e ~= 0 then quit() end
				end
			end
		end
	end


	-- Movement

	-- Friction
	local scale = 0.90
	g_playerState.velocity.x = g_playerState.velocity.x * scale
	g_playerState.velocity.y = g_playerState.velocity.y * scale
	if g_fly then
		g_playerState.velocity.z = g_playerState.velocity.z * scale
	end
	g_playerState.position.x = g_playerState.position.x + g_playerState.velocity.x
	g_playerState.position.y = g_playerState.position.y + g_playerState.velocity.y
	g_playerState.position.z = g_playerState.position.z + g_playerState.velocity.z
	if g_playerState.position.z < 0 then
		g_playerState.grounded = true
		g_playerState.velocity.z = 0
		g_playerState.position.z = 0
	else
		g_playerState.grounded = false
	end

	if g_fly then
		-- Move up.
		if g_jump then
			g_playerState.velocity.z = g_playerState.velocity.z + 1.0
		end
		-- Move down.
		if g_crouch then
			g_playerState.velocity.z = g_playerState.velocity.z - 1.0
		end
	else
		-- Jumping
		if g_playerState.grounded then
			if g_jump then
				g_playerState.velocity.z = g_playerState.velocity.z + G_JUMPVELOCITY
			end
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
	g_playerState.velocity = vec3_add(g_playerState.velocity, {x=yaw_x, y=yaw_y, z=0})

	if (g_mouse.delta_y and g_mouse.delta_y ~= 0) then
		g_playerState.euler.pitch = g_playerState.euler.pitch - g_mouse.delta_y/1000.0
	end
	if (g_mouse.delta_x and g_mouse.delta_x ~= 0) then
		g_playerState.euler.yaw = g_playerState.euler.yaw - g_mouse.delta_x/1000.0
	end

	-- Constrain up-down view to not go past vertical.
	if g_playerState.euler.pitch > G_PI/2 then g_playerState.euler.pitch = G_PI/2 end
	if g_playerState.euler.pitch < -G_PI/2 then g_playerState.euler.pitch = -G_PI/2 end

	if not g_fly then
		g_playerState.velocity.z = g_playerState.velocity.z + G_GRAVITY
	end

	g_playerState.orientation = hamiltonProduct(eulerToQuat(g_playerState.euler), aaToQuat({w=G_PI/2, x=1, y=0, z=0}))

	-- Display self.
	entity_setOrientation(g_worldEntity, {w=g_playerState.orientation.w,
	                                      x=-g_playerState.orientation.x,
	                                      y=-g_playerState.orientation.y,
	                                      z=-g_playerState.orientation.z})
	entity_setPosition(g_cameraEntity,
	                   {x=-g_playerState.position.x, y=-g_playerState.position.y, z=-(g_playerState.position.z + 25)})
end

function main()
	local e

	mainGame()

	g_averageFramerate = g_averageFramerate + (1/deltaT - g_averageFramerate)/1000
	g_frame = g_frame + 1
	g_mouse = {x=nil, y=nil, delta_x=0, delta_y=0}
end

function shutdown()
	info("shutdown", "Client quit")
	puts("FPS: "..toString(g_averageFramerate))
end
