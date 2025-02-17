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


-- This function can be run multiple times a frame.
function server_processEvents(events, client_index)
	local createdBoxes = false
	if not events then return end
	local events_length = #events
	for i = 1,events_length,1 do
		local event = events[i]
		local c = event.command
		local d = event.data
		if c == "change box color" then
			local occupied, box_index = isOccupied(d.position)
			if occupied then
				changeBoxMaterial(g_boxes[box_index], d.color)
				sendEvent("change box color", {position=d.position, color=d.color})
				-- TODO: Write to database.
			end
		elseif c == "move box" then
			local start_occupied, box_index = isOccupied(d.start_position)
			local end_occupied, _ = isOccupied(d.end_position)
			-- Must be a box within the bounds of the map.
			if start_occupied and box_index and not end_occupied then
				local box_index = getBoxEntry(d.start_position)
				g_boxes[box_index].position = d.end_position
				moveBox(box_index, d.end_position, d.start_position)
				sendEvent("move box", {start_position=d.start_position, end_position=d.end_position})
				-- TODO: Write to database.
			end
		else
			warning("processEvents", "Unrecognized event \""..c.."\"")
		end
	end
end

-- Keeping the code below because it could be used to create the boxes on first run.

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
					sendEventToClient(i,
									  "create initial box",
									  {position=g_boxes[box_index].position,
									   materialName=g_boxes[box_index].materialName})
				end
				if g_boxes_length == 0 then
					sendEventToClient(i, "no initial boxes", nil)
				end
			end

			-- Now here's the trick. We may receive lots of messages, or we may receive none.
			local messages = clientState[i]
			local messages_length = #messages
			if not serverState[i].boxesCreated then
				for messages_index = 1,messages_length,1 do
					if messages[messages_index].boxesCreated then
						serverState[i].boxesCreated = true
					end
				end
			end

			local events
			for messages_index = 1,messages_length,1 do
				server_processEvents(messages[messages_index].events, i)
			end
		end
	end

	processBoxes(g_boxes)
	sendQueuedEvents()
end

function shutdown()
	info("shutdown", "Server quit");
end
