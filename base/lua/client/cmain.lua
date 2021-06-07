--[[
This should be a completely sandboxed environment.
No libraries have been loaded, and the only resources available to the script are whitelisted C functions.

This should do almost nothing. The server engine/script and client engine will do the majority of the work.
This script will draw the HUD, create and set 3D reference entities, and perform other menial tasks.
--]]

Keys = {}

--[[
Key bind functions are run before networking happens.
`main` is run after networking happens.
--]]
function key_rollClockwise_d()
	Keys.right = true
	if not clientState.keys.right then
		clientState.keys.right = true
		clientState.keys.left = false
	end
end

function key_rollClockwise_u()
	Keys.right = false
	clientState.keys.right = false
	if Keys.left then
		clientState.keys.left = true
	end
end

function key_rollCounterClockwise_d()
	Keys.left = true
	if not clientState.keys.left then
		clientState.keys.left = true
		clientState.keys.right = false
	end
end

function key_rollCounterClockwise_u()
	Keys.left = false
	clientState.keys.left = false
	if Keys.right then
		clientState.keys.right = true
	end
end

function startup()
	info("main", "Lua start");
	
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
	
	info("main", "Loading models")
	
	cobra3ModelName = "oolite_cobra3.dat"
	cobra3Model, error = loadOoliteModel(modelPath .. cobra3ModelName)
	if cobra3Model ~= -1 then
		info("main", "Loaded " .. cobra3ModelName)
	end
	
	cfg2_setVariable("create none +key_rollClockwise")
	cfg2_setVariable("create none -key_rollClockwise")
	cfg2_setCallback("+key_rollClockwise", "key_rollClockwise_d")
	cfg2_setCallback("-key_rollClockwise", "key_rollClockwise_u")
	cfg2_setVariable("bind k_1073741903 +key_rollClockwise -key_rollClockwise")

	cfg2_setVariable("create none +key_rollCounterClockwise")
	cfg2_setVariable("create none -key_rollCounterClockwise")
	cfg2_setCallback("+key_rollCounterClockwise", "key_rollCounterClockwise_d")
	cfg2_setCallback("-key_rollCounterClockwise", "key_rollCounterClockwise_u")
	cfg2_setVariable("bind k_1073741904 +key_rollCounterClockwise -key_rollCounterClockwise")

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
	
	info("main", "Starting game")
end

function main()
end

function shutdown()
	info("main", "Lua end")
end
