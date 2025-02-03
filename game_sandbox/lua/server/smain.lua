
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

	-- Set the physics state.
	serverState[clientNumber].position = {x=0, y=0, z=g_boundingBoxRadius/2-3}
	serverState[clientNumber].orientation = aaToQuat({w=G_PI/2, x=1, y=0, z=0})
	serverState[clientNumber].euler = {}
	serverState[clientNumber].euler.yaw = 0.0
	serverState[clientNumber].euler.pitch = 0.0
	serverState[clientNumber].velocity = {x=0, y=0, z=0}
	serverState[clientNumber].collided = false
	serverState[clientNumber].grounded = false
	serverState[clientNumber].aabb = G_PLAYER_BB
end

function clientDisconnect(clientNumber)
	connectedClients[clientNumber] = false
	numClients = numClients - 1
	info("clientDisconnect", "Client " .. clientNumber .. " disconnected.")
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

	info("startup", "Starting game")
end

function main()
	-- Main program loop.

	-- Process clients
	for i = 1,maxClients,1 do
		if connectedClients[i] then
			-- Inform the client how many clients there are.
			serverState[i].numClients = numClients

			local keys = clientState[i].keys

			local scale = 0.90
			serverState[i].velocity.x = serverState[i].velocity.x * scale
			serverState[i].velocity.y = serverState[i].velocity.y * scale

			if serverState[i].grounded then
				if keys.jump then
					serverState[i].velocity.z = serverState[i].velocity.z + G_JUMPVELOCITY
				end
			end
			
			local yaw_x, yaw_y = 0, 0
			if keys.forward then
				yaw_x = yaw_x + sin(serverState[i].euler.yaw)
				yaw_y = yaw_y - cos(serverState[i].euler.yaw)
			end
			if keys.backward then
				yaw_x = yaw_x - sin(serverState[i].euler.yaw)
				yaw_y = yaw_y + cos(serverState[i].euler.yaw)
			end
			if keys.strafeLeft then
				yaw_x = yaw_x - cos(serverState[i].euler.yaw)
				yaw_y = yaw_y - sin(serverState[i].euler.yaw)
			end
			if keys.strafeRight then
				yaw_x = yaw_x + cos(serverState[i].euler.yaw)
				yaw_y = yaw_y + sin(serverState[i].euler.yaw)
			end
			serverState[i].velocity = vec3_add(serverState[i].velocity, {x=yaw_x, y=yaw_y, z=0})

			if (clientState[i].mouse.delta_y and clientState[i].mouse.delta_y ~= 0) then
				serverState[i].euler.pitch = serverState[i].euler.pitch + clientState[i].mouse.delta_y/1000.0
			end
			if (clientState[i].mouse.delta_x and clientState[i].mouse.delta_x ~= 0) then
				serverState[i].euler.yaw = serverState[i].euler.yaw + clientState[i].mouse.delta_x/1000.0
			end

			-- Constrain up-down view to not go past vertical.
			if serverState[i].euler.pitch > G_PI/2 then serverState[i].euler.pitch = G_PI/2 end
			if serverState[i].euler.pitch < -G_PI/2 then serverState[i].euler.pitch = -G_PI/2 end

			serverState[i].velocity.z = serverState[i].velocity.z + G_GRAVITY

			serverState[i].orientation = hamiltonProduct(eulerToQuat(serverState[i].euler),
														 aaToQuat({w=G_PI/2, x=1, y=0, z=0}))
			serverState[i] = playerMoveAndCollide(serverState[i])
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

	-- Process boxes
	processBoxes(g_boxes)
end

function shutdown()
	info("shutdown", "Server quit");
end
