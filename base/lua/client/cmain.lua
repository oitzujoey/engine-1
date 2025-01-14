--[[
This should be a completely sandboxed environment.
No libraries have been loaded, and the only resources available to the script are whitelisted C functions.

This should do almost nothing. The server engine/script and client engine will do the majority of the work.
This script will draw the HUD, create and set 3D reference entities, and perform other menial tasks.
--]]

include "../common/common.lua"
include "../client/keys.lua"
include "../common/loadworld.lua"

Keys = {}

function startup()
	info("startup", "Lua start");

	-- File paths
	-- More info about ship resources can be found in https://github.com/OoliteProject/oolite/blob/master/Resources/Config/shipdata.plist

	info("startup", "Loading models")

	loadWorld()

	cobra3ModelFileName = cobra3ModelFileName
	cobra3Model, error = loadOoliteModel(modelPath .. cobra3ModelFileName)
	if cobra3Model ~= -1 then
		info("startup", "Loaded " .. cobra3ModelFileName)
	end

	-- Prepare the textures.
	cobra1Material, error = material_create()
	cobra1Texture, error = material_loadTexture(texturePath .. cobra1TextureFileName)
	error = material_linkTexture(cobra1Material, cobra1Texture)

	cobra3Material, error = material_create()
	cobra3Texture, error = material_loadTexture(texturePath .. cobra3HullTextureFileName)
	error = material_linkTexture(cobra3Material, cobra3Texture)

	-- Set the default materials for each model.
	error = model_linkDefaultMaterial(cobra1Model, cobra1Material)
	error = model_linkDefaultMaterial(cobra3Model, cobra3Material)

	keys_createFullBind("k_1073741903", "key_left",			"key_left_d",		"key_left_u")
	keys_createFullBind("k_1073741904", "key_right",		"key_right_d",		"key_right_u")
	keys_createFullBind("k_1073741906", "key_up",			"key_up_d",			"key_up_u")
	keys_createFullBind("k_1073741905", "key_down",			"key_down_d",		"key_down_u")
	keys_createFullBind("k_119",		"key_accelerate",	"key_accelerate_d", "key_accelerate_u")
	keys_createFullBind("k_115",		"key_decelerate",	"key_decelerate_d", "key_decelerate_u")

	cfg2_setVariable("bind k_113 quit")
	-- cfg2_setVariable("specialBind")

	-- Create keys/buttons
	clientState.keys = {}
	clientState.keys.up = false
	clientState.keys.down = false
	clientState.keys.left = false
	clientState.keys.right = false
	clientState.keys.accelerate = false
	clientState.keys.decelerate = false

	Keys.up = false
	Keys.down = false
	Keys.left = false
	Keys.right = false

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
	local cobra3Entity, error = entity_createEntity(type_model)
	-- if error then return cobra3Entity, error end
	error = entity_linkChild(worldEntity, cobra3Entity)
	-- if error then return cobra3Entity, error end
	error = entity_linkChild(cobra3Entity, cobra3Model)
	-- if error then return cobra3Entity, error end
	return cobra3Entity, nil
end

function despawnClientShip(cobra3Entity)
	local error = entity_unlinkChild(worldEntity, cobra3Entity)
	-- if error then return error end
	entity_deleteEntity(cobra3Entity)
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
		entity_setOrientation(worldEntity, orientation)
		entity_setPosition(worldEntity, position)
	end

	-- Display other client ships.
	puts(serverState.numClients .. " " .. serverState.maxClients)
	for i = 1,serverState.maxClients,1 do
		local otherClientState = serverState.otherClients[i]
		-- Check for client connect.
		if not clientEntities[i] and otherClientState then
			clientEntities[i], error = spawnClientShip(clientEntities[i])
		-- Check for client disconnect.
		elseif clientEntities[i] and not otherClientState then
			error = despawnClientShip(clientEntities[i])
			clientEntities[i] = nil
		end
		local clientEntity = clientEntities[i]
		if clientEntity and otherClientState then
			entity_setPosition(clientEntity, otherClientState.position)
			entity_setOrientation(clientEntity, otherClientState.orientation)
		end
	end
end

function shutdown()
	info("shutdown", "Client quit")
end
