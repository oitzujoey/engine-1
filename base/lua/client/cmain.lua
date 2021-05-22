--[[
This should be a completely sandboxed environment.
No libraries have been loaded, and the only resources available to the script are whitelisted C functions.

This should do almost nothing. The server engine/script and client engine will do the majority of the work.
This script will draw the HUD, create and set 3D reference entities, and perform other menial tasks.
--]]

specialKey = false

--[[
Key bind functions are run before networking happens.
`main` is run after networking happens.
--]]
function key_rollClockwise()
	clientState.right = true
end

function key_rollCounterClockwise()
	clientState.left = true
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
		-- cobra3Entity, error = l_createEntity(type_model)
		-- error = l_entity_linkChild(worldEntity, cobra3Entity)
		-- error = l_entity_linkChild(cobra3Entity, cobra3Model)
		-- -- l_cnetwork_receiveEntityTree will set the 
		-- l_entity_setPosition(cobra3Entity, {x=20, y=0, z=0})
		-- l_entity_setOrientation(cobra3Entity, {w=1, x=0, y=0, z=0})
		
		-- l_snetwork_sendEntityTree()
		-- l_renderEntity(worldEntity, 0.0, 0.0, 1.0, 1.0)
	end
	
	cfg2_setVariable("create none key_rollClockwise")
	cfg2_setCallback("key_rollClockwise", "key_rollClockwise")
	cfg2_setVariable("bind k_1073741903 key_rollClockwise")

	cfg2_setVariable("create none key_rollCounterClockwise")
	cfg2_setCallback("key_rollCounterClockwise", "key_rollCounterClockwise")
	cfg2_setVariable("bind k_1073741904 key_rollCounterClockwise")

	-- cfg2_setVariable("create none key_rollClockwise")
	-- cfg2_setCallback("key_rollClockwise", "key_rollClockwise")
	-- cfg2_setVariable("bind k_1073741903 key_rollClockwise")

	-- cfg2_setVariable("create none key_rollClockwise")
	-- cfg2_setCallback("key_rollClockwise", "key_rollClockwise")
	-- cfg2_setVariable("bind k_1073741903 key_rollClockwise")

	cfg2_setVariable("bind k_113 quit")
	-- cfg2_setVariable("specialBind")
	
	-- Create keys/buttons
	clientState.up = false
	clientState.down = false
	clientState.left = false
	clientState.right = false
	
	info("main", "Starting game")
end

function main()
	-- while l_checkQuit() == 0 do
	
		
	
		-- puts("Lua render")
	-- render()
	
		-- -- puts("Lua getInput")
		-- input = (getInput())
		-- if input then
		-- 	-- puts("Lua got input");
		-- 	if input == 256 then
		-- 		break
		-- 	end
		-- end
		
		-- puts("Lua looping")
		
		-- l_main_housekeeping()
	
	-- if clientState.up then
	-- 	clientState["Hello"] = "world!"
	-- end
	
	clientState.up = false
	clientState.down = false
	clientState.left = false
	clientState.right = false
end

function shutdown()
	info("main", "Lua end")
end
