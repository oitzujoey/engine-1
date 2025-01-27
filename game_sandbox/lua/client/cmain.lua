include "../common/common.lua"
include "../client/keys.lua"
include "../common/loadworld.lua"

Keys = {}

function startup()
	info("startup", "Lua start");

	info("startup", "Loading models")

	loadWorld()

	-- Prepare the textures.
	defaultTerrainMaterial, error = material_create()
	defaultTerrainTexture, error = material_loadTexture("blender/terrain_material Base Color.png")
	error = material_linkTexture(defaultTerrainMaterial, defaultTerrainTexture)
	error = model_linkDefaultMaterial(terrainModel, defaultTerrainMaterial)

	defaultBoxMaterial, error = material_create()
	defaultBoxTexture, error = material_loadTexture("textures/blue.png")
	error = material_linkTexture(defaultBoxMaterial, defaultBoxTexture)
	error = model_linkDefaultMaterial(boxModel, defaultBoxMaterial)

	keys_createFullBind("k_1073741903", "key_left",			"key_left_d",		"key_left_u")
	keys_createFullBind("k_1073741904", "key_right",		"key_right_d",		"key_right_u")
	keys_createFullBind("k_1073741906", "key_up",			"key_up_d",			"key_up_u")
	keys_createFullBind("k_1073741905", "key_down",			"key_down_d",		"key_down_u")
	keys_createFullBind("k_100", "key_yawLeft", "key_yawLeft_d", "key_yawLeft_u")
	keys_createFullBind("k_97", "key_yawRight", "key_yawRight_d", "key_yawRight_u")
	keys_createFullBind("k_119",		"key_accelerate",	"key_accelerate_d", "key_accelerate_u")
	keys_createFullBind("k_115",		"key_decelerate",	"key_decelerate_d", "key_decelerate_u")

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
		entity_setPosition(g_worldEntity, position)
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
end

function shutdown()
	info("shutdown", "Client quit")
end
