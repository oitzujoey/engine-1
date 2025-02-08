G_SERVER=true

-- `include` is the engine's version of `require`.
include "../common/common.lua"
include "../common/loadworld.lua"

g_eventQueues = {}

function sendEvent(command, data)
	for i = 1,maxClients,1 do
		if connectedClients[i] then
			sendEventToClient(i, command, data)
		end
	end
end

function sendEventToClient(clientNumber, command, data)
	local event = {command=command, data=data}
	if not g_eventQueues[clientNumber] then
		g_eventQueues[clientNumber] = {}
	end
	push(g_eventQueues[clientNumber], event)
end

function sendQueuedEvents()
	for i = 1,maxClients,1 do
		if connectedClients[i] then
			if not g_eventQueues[i] then
				g_eventQueues[i] = {}
			end
			serverState[i].events = g_eventQueues[i]
			g_eventQueues[i] = {}
		end
	end
end

-- function loadBoxes()
-- 	for i = 1,g_boxes_length,1 do
-- 		local box = {}
-- 		local boxEntity, e = entity_createEntity(g_entity_type_model)
-- 		e = entity_linkChild(g_cameraEntity, boxEntity)
-- 		e = entity_linkChild(boxEntity, boxModel)
-- 		box.entity = boxEntity

-- 		entity_setScale(boxEntity, g_boxes_scale)
-- 		box.position = snapToGrid({x=((i-1)%10 - 4.5)*g_gridSpacing,
-- 								   y=((((i-1)-(i-1)%10)/10)%10 - 4.5)*g_gridSpacing,
-- 								   z=(((i-1)-(i-1)%10-(i-1)%100)/100 - 4.5)*g_gridSpacing})
-- 		entity_setPosition(boxEntity, box.position)
-- 		entity_setOrientation(boxEntity, {w=1, x=0, y=0, z=0})

-- 		createBoxEntry(boxEntity, box.position)

-- 		box.velocity = {x=0, y=0, z=0}
-- 		box.aabb = G_BOX_BB

-- 		g_boxes[i] = box
-- 	end
-- end


function consoleCommandCreateBox()
	local position = {x=0, y=0, z=0}
	local box_index = getBoxEntry(position)
	if box_index then
		info("createBox", "Cannot spawn box. Another box is currently at the origin.")
	else
		local materialName = g_materialNames[random()%#g_materialNames + 1]
		createBox(position, materialName)
		sendEvent("create box", {position=position, materialName=materialName})
		info("createBox", "Created box at origin.")
	end
end

function setupConsoleCommands()
	createConsoleCommand("create_box", "consoleCommandCreateBox")
end


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
	serverState[clientNumber].events = {}

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

	serverState[clientNumber].boxesCreated = false
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

	setupConsoleCommands()

	-- file = l_vfs_getFileText("src/sgame/smain.lua")
	-- l_puts(file)

	info("startup", "Loading world tree")
    loadWorld()
	-- loadBoxes()

	info("startup", "Starting game")
end

function main()
	-- Process clients
	for i = 1,maxClients,1 do
		if connectedClients[i] then
			-- Send boxes to clients when they connect.
			if not serverState[i].boxesCreated then
				for box_index = 1,g_boxes_length,1 do
					puts(toString(g_boxes[box_index].position.x).." "..toString(g_boxes[box_index].position.y).." "..toString(g_boxes[box_index].position.z))
					puts(toString(box_index))
					puts(toString(g_boxes[box_index].materialName))
					sendEventToClient(i,
									  "create initial box",
									  {position=g_boxes[box_index].position,
									   materialName=g_boxes[box_index].materialName})
				end
				if g_boxes_length == 0 then
					sendEventToClient(i, "no initial boxes", nil)
				end
				puts("")
			end
			if clientState[i].boxesCreated then
				serverState[i].boxesCreated = true
			end

			-- Inform the client how many clients there are.
			serverState[i].numClients = numClients

			local keys = clientState[i].keys

			if keys.color then
				local cursorPosition = calculateCursorPosition(serverState[i].position, serverState[i].orientation)
				local occupied, box_index = isOccupied(cursorPosition)
				if occupied then
					local color = g_materialNames[keys.color]
					changeBoxMaterial(g_boxes[box_index], color)
					sendEvent("change box color", {position=cursorPosition, color=color})
				end
			end

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


	sendQueuedEvents()
end

function shutdown()
	info("shutdown", "Server quit");
end
