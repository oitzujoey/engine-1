--[[
This should be a completely sandboxed environment.
No libraries have been loaded, and the only resources available to the script are whitelisted C functions.

This should do almost nothing. The server engine/script and client engine will do the majority of the work.
This script will draw the HUD, create and set 3D reference entities, and perform other menial tasks.
--]]

include "../client/keys.lua"

Keys = {}

function startup()
	info("startup", "Lua start");
	
	-- Entity types
	type_none = 0
	type_entity = 1
	type_model = 2
	
	-- Entity list constant indices
	worldEntity = 0
	
	-- File paths
	-- More info about ship resources can be found in https://github.com/OoliteProject/oolite/blob/master/Resources/Config/shipdata.plist
	texturePath = "oolite-binary-resources/Textures/"
	modelPath = "oolite-binary-resources/Models/"
	
	cobra3ModelFileName = "oolite_cobra3.dat"
	cobra3HullTextureFileName = "oolite_cobra3_diffuse.png"
	cobra3GunTextureFileName = "oolite_cobra3_subents.png"
	
	cobra1ModelFileName = "oolite_cobramk1.dat"
	cobra1TextureFileName = "oolite_cobramk1_diffuse.png"
	-- cobra1TextureFileName = "bottom_metal.png"
	
	info("startup", "Loading models")
	
	--[[
	How it's setup right now, creating an entity does nothing. The only useful
	action this does is load the model for the server to link to. All these
	entities will be overridden after the client receives the server's entities.
	]]--
	cobra3ModelFileName = cobra3ModelFileName
	cobra3Model, error = loadOoliteModel(modelPath .. cobra3ModelFileName)
	if cobra3Model ~= -1 then
		info("startup", "Loaded " .. cobra3ModelFileName)
	end
	cobra1Model, error = loadOoliteModel(modelPath .. cobra1ModelFileName)
	
	-- Prepare the textures.
	cobra1Material, error = material_create()
	cobra1Texture, error = material_loadTexture(texturePath .. cobra1TextureFileName)
	error = material_linkTexture(cobra1Material, cobra1Texture)
	
	-- Set the default materials for each model.
	error = model_linkDefaultMaterial(cobra1Model, cobra1Material)
	
	keys_createFullBind("k_1073741903", "key_left",         "key_left_d",       "key_left_u")
	keys_createFullBind("k_1073741904", "key_right",        "key_right_d",      "key_right_u")
	keys_createFullBind("k_1073741906", "key_up",           "key_up_d",         "key_up_u")
	keys_createFullBind("k_1073741905", "key_down",         "key_down_d",       "key_down_u")
	keys_createFullBind("k_119",        "key_accelerate",   "key_accelerate_d", "key_accelerate_u")
	keys_createFullBind("k_115",        "key_decelerate",   "key_decelerate_d", "key_decelerate_u")

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

function main()
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
end

function shutdown()
	info("shutdown", "Client quit")
end
