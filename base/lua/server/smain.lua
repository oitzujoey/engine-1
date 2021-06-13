
-- `include` is the engine's version of `require`.
include "../common/extra.lua"

function clientConnect(clientNumber)
	connectedClients[clientNumber] = true
	
	info("clientConnect", "Client " .. clientNumber .. " connected.")
	if clientNumber == 1 and rant == nil then
		info("clientConnect", "You do not know how much I hate one-indexed arrays. Honestly, one-based indexing isn't that bad. The problem only appears when you make a language (Lua) designed to interface with another language (C) that has zero-based indexing. Then you get this annoying problem where C refers to the first connected client as \"Client 0\" (as it should be), whereas Lua refers to the first connected client as \"Client 1\". This is MADNESS. Client 0 != Client 1. This little nuisance alone could turn me off from a language such as this. It's almost as bad as MATLAB, and yet, here I am using it. If I ever do this again, maybe I'll use something decent like JavaScript.")
		rant = true
	end
	
	-- Spawn a spaceship.
	serverState[clientNumber].position = {x=0, y=0, z=0}
	serverState[clientNumber].orientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	serverState[clientNumber].speed = 1
	cobra3Entity[clientNumber], error = entity_createEntity(type_model)
	error = entity_linkChild(worldEntity, cobra3Entity[clientNumber])
	error = entity_linkChild(cobra3Entity[clientNumber], cobra3Model)
	entity_setPosition(cobra3Entity[clientNumber], serverState[clientNumber].position)
	entity_setOrientation(cobra3Entity[clientNumber], serverState[clientNumber].orientation)
end

function clientDisconnect(clientNumber)
	connectedClients[clientNumber] = false
	error = entity_unlinkChild(worldEntity, cobra3Entity[clientNumber])
	info("clientDisconnect", "Client " .. clientNumber .. " disconnected.")
	
	entity_deleteEntity(cobra3Entity[clientNumber])
end

function startup()
	
	info("startup", "Server started")
	
	-- We start with no connected clients.
	connectedClients = {}
	
	for i = 1,maxClients,1 do
		connectedClients[i] = false
	end
	
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
	cobra1Model, error = loadOoliteModel(modelPath .. "oolite_cobramk1.dat")
	-- if cobra3Model ~= -1 then
	-- 	-- l_entity_setOrientation(cobra3Entity, {w=1, x=0, y=0, z=0})
		
	-- 	-- l_renderEntity(worldEntity, 0.0, 0.0, 1.0, 1.0)
	-- end
	
	cobra3Entity = {}

	-- // Load the Oolite model specified by the string. Return the newly created entity index and the name of the entity.
	-- integer entityIndex, string name = l_loadOoliteModel(string fileName)
	
	-- // Create an entity (if a freed one doesn't already exist) with name "name" and type "type" and add it to the entity list. Return the index.
	-- integer index = l_createEntity(string name, integer type)
	
	-- // Link the two entities if possible. Non-entity structures are allowed to be linked as long as the type of the entity is set properly.
	-- l_entity_linkChild(integer parentIndex, integer childIndex)
	
	-- Rotate the world around the camera a bit.
	
	-- rotation = {w=0.99985, x=-0.005, y=-0.01, z=0.005}
	-- worldRotation = {w=0.99985, x=0.005, y=0.01, z=-0.005}
	-- worldCounterRotation = {w=0.99985, x=-0.005, y=-0.01, z=0.005}
	worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	
	TurnRate = 100
	
	Vec3_xn = {x=-1, y=0, z=0}
	Vec3_xp = {x=1, y=0, z=0}
	Vec3_yn = {x=0, y=-1, z=0}
	Vec3_yp = {x=0, y=1, z=0}
	Vec3_zn = {x=0, y=0, z=-1}
	Vec3_zp = {x=0, y=0, z=1}
	
	cobra1Entity = {}
	cobra1Scale = 1000.0
	cobra1Num = 500
	
	for i = 1,cobra1Num,1 do
		cobra1Entity[i], error = entity_createEntity(type_model)
		error = entity_linkChild(worldEntity, cobra1Entity[i])
		error = entity_linkChild(cobra1Entity[i], cobra1Model)
		entity_setPosition(cobra1Entity[i], {x=cobra1Scale*((i%8)/4 - 1), y=cobra1Scale*((i%64 - i%8)/32 - 1), z=cobra1Scale*((i%512 - i%64)/256 - 1)})
		entity_setOrientation(cobra1Entity[i], {w=1, x=0, y=0, z=0})
	end
	
	info("startup", "Starting game")
end

function main()
	-- Main program loop.
	
	for i = 1,maxClients,1 do
		if connectedClients[i] then
			tempVec3 = {x=0, y=0, z=0}
			if clientState[i].keys.left then
				tempVec3 = vec3_crossProduct(Vec3_yp, Vec3_xp)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				serverState[i].orientation, error = hamiltonProduct(serverState[i].orientation, tempQuat)
			end
			if clientState[i].keys.right then
				tempVec3 = vec3_crossProduct(Vec3_yp, Vec3_xn)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				serverState[i].orientation, error = hamiltonProduct(serverState[i].orientation, tempQuat)
			end
			if clientState[i].keys.up then
				tempVec3 = vec3_crossProduct(Vec3_zp, Vec3_yn)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				serverState[i].orientation, error = hamiltonProduct(serverState[i].orientation, tempQuat)
			end
			if clientState[i].keys.down then
				tempVec3 = vec3_crossProduct(Vec3_zp, Vec3_yp)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				serverState[i].orientation, error = hamiltonProduct(serverState[i].orientation, tempQuat)
			end
			if clientState[i].keys.accelerate then
				serverState[i].speed = serverState[i].speed + 0.1
			end
			if clientState[i].keys.decelerate then
				serverState[i].speed = serverState[i].speed - 0.1
				if serverState[i].speed < 0 then
					serverState[i].speed = 0
				end
			end
			
			
			entity_setVisible(worldEntity, i)
			
			-- Make all ships visible except for the player's own ship.
			for j = 1,maxClients,1 do
				if j ~= i then
					entity_setVisible(cobra3Entity[i], j)
				end
			end
			
			for j = 1,cobra1Num,1 do
				entity_setVisible(cobra1Entity[j], i)
			end
			
			serverState[i].orientation, error = quatNormalize(serverState[i].orientation)
			
			rotatedVec3, error = vec3_rotate({x=0, y=0, z=1}, serverState[i].orientation)
			serverState[i].position.x = serverState[i].position.x + serverState[i].speed * rotatedVec3.x
			serverState[i].position.y = serverState[i].position.y + serverState[i].speed * rotatedVec3.y
			serverState[i].position.z = serverState[i].position.z + serverState[i].speed * rotatedVec3.z
			
			entity_setOrientation(cobra3Entity[i], serverState[i].orientation)
			entity_setPosition(cobra3Entity[i], serverState[i].position)
			
			serverState[i].position.z = serverState[i].position.z
		end
	end
end

function shutdown()
	info("shutdown", "Server quit");
end
