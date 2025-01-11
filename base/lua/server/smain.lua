
-- `include` is the engine's version of `require`.
include "../common/common.lua"
include "../common/loadworld.lua"

function clientConnect(clientNumber)
	connectedClients[clientNumber] = true
	numClients = numClients + 1

	info("clientConnect", "Client " .. clientNumber .. " connected.")
	if clientNumber == 1 and rant == nil then
		info("clientConnect", "You do not know how much I hate one-indexed arrays. Honestly, one-based indexing isn't that bad. The problem only appears when you make a language (Lua) designed to interface with another language (C) that has zero-based indexing. Then you get this annoying problem where C refers to the first connected client as \"Client 0\" (as it should be), whereas Lua refers to the first connected client as \"Client 1\". This is MADNESS. Client 0 != Client 1. This little nuisance alone could turn me off from a language such as this. It's almost as bad as MATLAB, and yet, here I am using it. If I ever do this again, maybe I'll use something decent like JavaScript.")
		rant = true
	end

	-- Inform the client how many total clients there may be.
	serverState[clientNumber].maxClients = maxClients

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
	numClients = numClients - 1

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
	numClients = 0

	-- file = l_vfs_getFileText("src/sgame/smain.lua")
	-- l_puts(file)

	-- Entity list constant indices

	-- Textures

	info("startup", "Loading world tree")
    loadWorld()

	--[[
	How it's setup right now, loading a model does nothing. The only useful
	action this does is create a model for the client to link to. This will be
	important in the future though when ray tracing (geometric, not graphical)
	is added.
	]]--
	-- if cobra3Model ~= -1 then
	-- 	-- l_entity_setOrientation(cobra3Entity, {w=1, x=0, y=0, z=0})

	-- 	-- l_renderEntity(worldEntity, 0.0, 0.0, 1.0, 1.0)
	-- end

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

	info("startup", "Starting game")
end

function main()
	-- Main program loop.

	for i = 1,maxClients,1 do
		if connectedClients[i] then
			-- Inform the client how many clients there are.
			serverState[i].numClients = numClients

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


			-- entity_setVisible(worldEntity, i)

			serverState[i].orientation, error = quatNormalize(serverState[i].orientation)

			rotatedVec3, error = vec3_rotate({x=0, y=0, z=1}, serverState[i].orientation)
			serverState[i].position.x = serverState[i].position.x + serverState[i].speed * rotatedVec3.x
			serverState[i].position.y = serverState[i].position.y + serverState[i].speed * rotatedVec3.y
			serverState[i].position.z = serverState[i].position.z + serverState[i].speed * rotatedVec3.z

			entity_setOrientation(cobra3Entity[i], serverState[i].orientation)
			entity_setPosition(cobra3Entity[i], serverState[i].position)

			serverState[i].position.z = serverState[i].position.z

            serverState[i].list = {0, 1, 2}
		end
	end

	for i = 1,maxClients,1 do
		if connectedClients[i] then
			-- Send other clients' state to the client.
            serverState[i].otherClients = {}
			for j = 1,maxClients,1 do
				if connectedClients[j] and j ~= i then
					serverState[i].otherClients[j] = {}
					serverState[i].otherClients[j].position = serverState[j].position
					serverState[i].otherClients[j].orientation = serverState[j].orientation
                else
                    serverState[i].otherClients[j] = nil
				end
			end
		end
	end
end

function shutdown()
	info("shutdown", "Server quit");
end
