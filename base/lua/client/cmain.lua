--[[
This should be a completely sandboxed environment.
No libraries have been loaded, and the only resources available to the script are whitelisted C functions.

This should do almost nothing. The server engine/script and client engine will do the majority of the work.
This script will draw the HUD, create and set 3D reference entities, and perform other menial tasks.
--]]

include "keys.lua"

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
	texturePath = "oolite-binary-resources/Textures/"
	modelPath = "oolite-binary-resources/Models/"
	HullTexture = "oolite_cobra3_diffuse.png"
	GunTexture = "oolite_cobra3_subents.png"
	
	info("startup", "Loading models")
	
	cobra3ModelName = "oolite_cobra3.dat"
	cobra3Model, error = loadOoliteModel(modelPath .. cobra3ModelName)
	if cobra3Model ~= -1 then
		info("startup", "Loaded " .. cobra3ModelName)
	end
	
	keys_createFullBind("k_1073741903", "key_left",     "key_left_d",   "key_left_u")
	keys_createFullBind("k_1073741904", "key_right",    "key_right_d",  "key_right_u")
	keys_createFullBind("k_1073741906", "key_up",       "key_up_d",     "key_up_u")
	keys_createFullBind("k_1073741905", "key_down",     "key_down_d",   "key_down_u")

	cfg2_setVariable("bind k_113 quit")
	-- cfg2_setVariable("specialBind")
	
	-- Create keys/buttons
	clientState.keys = {}
	clientState.keys.up = false
	clientState.keys.down = false
	clientState.keys.left = false
	clientState.keys.right = false
	
	Keys.up = false
	Keys.down = false
	Keys.left = false
	Keys.right = false
	
	info("startup", "Starting game")
end

function main()
end

function shutdown()
	info("shutdown", "Client quit")
end
