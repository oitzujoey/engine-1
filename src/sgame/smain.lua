
-- `include` is the engine's version of `require`.
-- include "extra.lua"

function startup()
	
	l_log_info("main", "Server started")
	-- box = l_loadObj("resources/microcode-v1.1.2.obj")
	-- box = l_loadObj("resources/untitled.obj")
	-- l_puts(box)
	
	-- file = l_vfs_getFileText("src/sgame/smain.lua")
	-- l_puts(file)
	
	-- Entity list constant indices
	type_none = 0
	type_entity = 1
	type_model = 2
	worldEntity = 0
	
	-- Textures
	texturePath = "oolite-binary-resources/Textures/"
	modelPath = "oolite-binary-resources/Models/"
	HullTexture = "oolite_cobra3_diffuse.png"
	GunTexture = "oolite_cobra3_subents.png"
	
	l_log_info("main", "Loading world tree")
	
	cobra3Model, error = l_loadOoliteModel(modelPath .. "oolite_cobra3.dat")
	if cobra3Model ~= -1 then
		cobra3Entity, error = l_createEntity(type_model)
		error = l_entity_linkChild(worldEntity, cobra3Entity)
		error = l_entity_linkChild(cobra3Entity, cobra3Model)
		l_entity_setPosition(cobra3Entity, {x=0, y=0, z=50})
		-- l_entity_setOrientation(cobra3Entity, {w=1, x=0, y=0, z=0})
		
		-- l_snetwork_sendEntityTree()
		-- l_renderEntity(worldEntity, 0.0, 0.0, 1.0, 1.0)
	end

	-- // Load the Oolite model specified by the string. Return the newly created entity index and the name of the entity.
	-- integer entityIndex, string name = l_loadOoliteModel(string fileName)
	
	-- // Create an entity (if a freed one doesn't already exist) with name "name" and type "type" and add it to the entity list. Return the index.
	-- integer index = l_createEntity(string name, integer type)
	
	-- // Link the two entities if possible. Non-entity structures are allowed to be linked as long as the type of the entity is set properly.
	-- l_entity_linkChild(integer parentIndex, integer childIndex)
	
	-- Rotate the world around the camera a bit.
	
	rotation = {w=0.99985, x=-0.005, y=-0.01, z=0.005}
	orientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	worldRotation = {w=0.99985, x=0.005, y=0.01, z=-0.005}
	worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	
	-- extra()
	
	l_log_info("main", "Starting game")
end

function main()
	-- Main program loop. l_checkQuit() checks if the program is supposed to end and ends it if needed.
	-- while l_checkQuit() == 0 do
	-- orientation, error = l_hamiltonProduct(orientation, rotation)
	-- orientation, error = l_quatNormalize(orientation)
	-- -- l_log_info("main", orientation.w .. " " .. orientation.x .. " " .. orientation.y .. " " .. orientation.z)
	-- l_entity_setOrientation(cobra3Entity, orientation)
	worldOrientation, error = l_hamiltonProduct(worldOrientation, worldRotation)
	worldOrientation, error = l_quatNormalize(worldOrientation)
	l_entity_setOrientation(worldEntity, worldOrientation)
	
	-- This runs once per loop. It does tasks such as read from the terminal and send and receive packets.
	-- l_main_housekeeping()
	-- end
end

function shutdown()
	l_log_info("main", "Server quit");
end
