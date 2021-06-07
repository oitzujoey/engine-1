
-- `include` is the engine's version of `require`.
include "../common/extra.lua"

function startup()
	
	info("startup", "Server started")
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
	
	info("startup", "Loading world tree")
	
	cobra3Model, error = loadOoliteModel(modelPath .. "oolite_cobra3.dat")
	if cobra3Model ~= -1 then
		cobra3Entity, error = createEntity(type_model)
		info("startup", "Linking entity to world.")
		error = entity_linkChild(worldEntity, cobra3Entity)
		info("startup", "Linking model to entity.")
		error = entity_linkChild(cobra3Entity, cobra3Model)
		entity_setPosition(cobra3Entity, {x=0, y=0, z=50})
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
	worldCounterRotation = {w=0.99985, x=-0.005, y=-0.01, z=0.005}
	worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	
	info("startup", "Starting game")
end

function main()
	-- Main program loop. l_checkQuit() checks if the program is supposed to end and ends it if needed.
	-- while l_checkQuit() == 0 do
	-- orientation, error = l_hamiltonProduct(orientation, rotation)
	-- orientation, error = l_quatNormalize(orientation)
	-- -- l_log_info("main", orientation.w .. " " .. orientation.x .. " " .. orientation.y .. " " .. orientation.z)
	-- l_entity_setOrientation(cobra3Entity, orientation)
	
	-- This runs once per loop. It does tasks such as read from the terminal and send and receive packets.
	-- l_main_housekeeping()
	-- end
	
	for i = 0,1,1 do
		if clientState[i] ~= nil then
	-- 	if clientState[0].up ~= nil then
			-- if clientState[0].up then
			-- 	info("main", "right")
			-- else
			-- 	info("main", "not right")
			-- end
			-- if clientState[0].down then
			-- 	info("main", "right")
			-- else
			-- 	info("main", "not right")
			-- end
			-- info("main", "running")
			if clientState[i].keys.left then
				info("main", "left")
				worldOrientation, error = hamiltonProduct(worldOrientation, worldCounterRotation)
			end
			if clientState[i].keys.right then
				info("main", "right")
				worldOrientation, error = hamiltonProduct(worldOrientation, worldRotation)
			end
			
			-- entity_setVisible(worldEntity, i)
			-- entity_setVisible(cobra3Entity, i)
		end
	end
	worldOrientation, error = quatNormalize(worldOrientation)
	entity_setOrientation(worldEntity, worldOrientation)
	
end

function shutdown()
	info("shutdown", "Server quit");
end
