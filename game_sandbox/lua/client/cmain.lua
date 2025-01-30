include "../common/common.lua"
include "../client/keys.lua"
include "../common/loadworld.lua"

Keys = {}
g_mouse = {}

g_sensitivity = 1.0

function startup()
	local e
	info("startup", "Lua start");

	info("startup", "Loading models")

	loadWorld()

	-- Prepare the textures.
	defaultTerrainMaterial, error = material_create()
	defaultTerrainTexture, error = material_loadTexture("blender/terrain_material Base Color.png")
	error = material_linkTexture(defaultTerrainMaterial, defaultTerrainTexture)
	error = model_linkDefaultMaterial(terrainModel, defaultTerrainMaterial)

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
	magentaAlphaMaterial, error = loadTexture("magenta-alpha")

	-- error = model_linkDefaultMaterial(boxModel, whiteMaterial)

	g_cursorEntity = modelEntity_create({x=0, y=0, z=1000}, {w=1, x=-1, y=0, z=0}, 100.0)
	e = entity_linkMaterial(g_cursorEntity, magentaAlphaMaterial)

	e = entity_linkMaterial(modelEntity_create({x=0, y=0, z=2000}, {w=1, x=-1, y=0, z=0}, 400.0), cyanMaterial)

	g_solarSystem, e = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_cameraEntity, g_solarSystem)

	g_binarySystem, e = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_solarSystem, g_binarySystem)

	g_bigPlanet, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_bigPlanet, boxModel)
	entity_setScale(g_bigPlanet, 20.0)
	entity_setPosition(g_bigPlanet, {x=-25, y=-25, z=0})
	entity_setOrientation(g_bigPlanet, {w=1, x=-1, y=0, z=0})
	e = entity_linkMaterial(g_bigPlanet, redMaterial)
	e = entity_linkChild(g_binarySystem, g_bigPlanet)

	g_smallPlanet, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_smallPlanet, boxModel)
	entity_setScale(g_smallPlanet, 15.0)
	entity_setPosition(g_smallPlanet, {x=25, y=25, z=-5})
	entity_setOrientation(g_smallPlanet, {w=1, x=-1, y=0, z=0})
	e = entity_linkMaterial(g_smallPlanet, greenMaterial)
	e = entity_linkChild(g_binarySystem, g_smallPlanet)

	g_entity = modelEntity_create({x=500, y=0, z=1000}, {w=1, x=-1, y=0, z=0}, 100.0)

	_ = (function()
			local e = nil
			for i = 1,g_boxes_length,1 do
				local material = cardboardBoxMaterial
				e = entity_linkMaterial(g_boxEntities[i], material)
			end
			return nil
	end)()

	keys_createFullBind("k_1073741903", "key_left",			"key_left_d",		"key_left_u")
	keys_createFullBind("k_1073741904", "key_right",		"key_right_d",		"key_right_u")
	keys_createFullBind("k_1073741906", "key_up",			"key_up_d",			"key_up_u")
	keys_createFullBind("k_1073741905", "key_down",			"key_down_d",		"key_down_u")
	keys_createFullBind("k_100", "key_yawLeft", "key_yawLeft_d", "key_yawLeft_u")
	keys_createFullBind("k_97", "key_yawRight", "key_yawRight_d", "key_yawRight_u")
	keys_createFullBind("k_119",		"key_accelerate",	"key_accelerate_d", "key_accelerate_u")
	keys_createFullBind("k_115",		"key_decelerate",	"key_decelerate_d", "key_decelerate_u")
	keys_createFullBind("m_1", "mouse_leftButton", "mouse_leftPress", "mouse_leftRelease")
	keys_createMouseBind("mouse_motion")

	cfg2_setVariable("bind k_113 quit")

	-- Create keys/buttons
	clientState.keys = {}
	clientState.keys.up = false
	clientState.keys.down = false
	clientState.keys.left = false
	clientState.keys.right = false
	clientState.keys.yawLeft = false
	clientState.keys.yawRight = false
	clientState.keys.accelerate = false
	clientState.keys.decelerate = false

	Keys.up = false
	Keys.down = false
	Keys.left = false
	Keys.right = false
	Keys.yawLeft = false
	Keys.yawRight = false

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


function spawnClientShip()
	-- local cobra3Entity, error = entity_createEntity(type_model)
	-- -- if error then return cobra3Entity, error end
	-- error = entity_linkChild(worldEntity, cobra3Entity)
	-- -- if error then return cobra3Entity, error end
	-- error = entity_linkChild(cobra3Entity, cobra3Model)
	-- -- if error then return cobra3Entity, error end
	-- return cobra3Entity, nil
	return nil, nil
end

function despawnClientShip(cobra3Entity)
	-- local error = entity_unlinkChild(worldEntity, cobra3Entity)
	-- -- if error then return error end
	-- entity_deleteEntity(cobra3Entity)
	return nil
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
		-- entity_setOrientation(g_cameraEntity, serverState.orientation)
		orientation.w = serverState.orientation.w
		orientation.x = -serverState.orientation.x
		orientation.y = -serverState.orientation.y
		orientation.z = -serverState.orientation.z
		entity_setOrientation(g_worldEntity, orientation)
		entity_setPosition(g_cameraEntity, position)
	end

	-- -- Display other client ships.
	-- puts(serverState.numClients .. " " .. serverState.maxClients)
	-- for i = 1,serverState.maxClients,1 do
	-- 	local otherClientState = serverState.otherClients[i]
	-- 	-- Check for client connect.
	-- 	if not clientEntities[i] and otherClientState then
	-- 		clientEntities[i], error = spawnClientShip(clientEntities[i])
	-- 	-- Check for client disconnect.
	-- 	elseif clientEntities[i] and not otherClientState then
	-- 		error = despawnClientShip(clientEntities[i])
	-- 		clientEntities[i] = nil
	-- 	end
	-- 	local clientEntity = clientEntities[i]
	-- 	if clientEntity and otherClientState then
	-- 		entity_setPosition(clientEntity, otherClientState.position)
	-- 		entity_setOrientation(clientEntity, otherClientState.orientation)
	-- 	end
	-- end


	_ = (function()
			entity_deleteEntity(g_entity)
			g_entity = modelEntity_create({x=500, y=0, z=1000}, {w=1, x=-1, y=0, z=0}, 100.0)
			local materials = {redMaterial, greenMaterial, blueMaterial}
			local material = materials[((g_frame//60) % 3) + 1]
			puts(material)
			e = entity_linkMaterial(g_entity, material)
	end)()


	entity_setPosition(g_solarSystem, {x=100*sin(g_frame/100.0), y=100*cos(g_frame/100.0), z=500})
	entity_setOrientation(g_solarSystem, aaToQuat({w=g_frame/60, x=0, y=0, z=1}))
	entity_setOrientation(g_smallPlanet, aaToQuat({w=g_frame/60*7, x=1, y=1, z=0}))
	entity_setOrientation(g_bigPlanet, aaToQuat({w=g_frame/60*3, x=0, y=1, z=1}))

	g_frame = g_frame + 1
end

function shutdown()
	info("shutdown", "Client quit")
end
