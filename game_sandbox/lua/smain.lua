G_SERVER=true

-- `include` is the engine's version of `require`.
include "common.lua"

g_eventQueues = {}
g_clients = {}

function sendEvent(command, data)
	for i = 1,maxClients,1 do
		if g_clients[i].connected then
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
		if g_clients[i].connected then
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
			local box_index = getBoxEntry(d.position)
			if box_index then
				local c = d.color
				local p = d.position
				local x = p.x
				local y = p.y
				local z = p.z
				changeBoxMaterial(g_boxes[box_index], c)
				sendEvent("change box color", {position=p, color=c})
				local id = g_boxes[getBoxEntry(p)].id
				sqlite_exec(g_db, "UPDATE boxes SET color = '"..c.."' WHERE id == "..id..";")
			end
		elseif c == "move box" then
			local start_occupied, box_index = isOccupied(d.start_position)
			local end_occupied, _ = isOccupied(d.end_position)
			-- Must be a box within the bounds of the map.
			if start_occupied and box_index and not end_occupied then
				local box_index = getBoxEntry(d.start_position)
				g_boxes[box_index].position = d.end_position
				moveBox(box_index, d.end_position, d.start_position)
				g_boxes[box_index].needsUpdate = true
				sendEvent("move box", {start_position=d.start_position, end_position=d.end_position})

				-- Enable physics for box above. We could do all boxes above, but I noticed that the boxes don't move until
				-- the box below them moves entirely out of the way, meaning that the box above doesn't move immediately,
				-- `needsUpdate` is set to false, and the box ends up *never* moving.
				updateNeighborBoxes(d.start_position)

				do -- Update database
					local id = g_boxes[box_index].id
					local n = snapToGrid(d.end_position)
					n.z = n.z + g_backOff
					sqlite_exec(g_db, "UPDATE boxes SET x = "..n.x..", y = "..n.y..", z = "..n.z.." WHERE id == "..id..";")
				end
			end
		else
			warning("processEvents", "Unrecognized event \""..c.."\"")
		end
	end
end

-- Keeping the code below because it could be used to create the boxes on first run.

function initializeBoxes()
	for i = 1,1000,1 do
		local p = snapToGrid({x=((i-1)%10 - 4.5)*g_gridSpacing,
							  y=((((i-1)-(i-1)%10)/10)%10 - 4.5)*g_gridSpacing,
							  z=(((i-1)-(i-1)%10-(i-1)%100)/100 - 4.5)*g_gridSpacing})
		sqlite_exec(g_db, "INSERT INTO boxes(color, x, y, z) VALUES ('red', "..p.x..", "..p.y..", "..p.z..");")
	end
end

function loadBoxes()
	local boxDescriptors, e = sqlite_exec(g_db, 'SELECT * FROM boxes;')
	local boxDescriptors_length = #boxDescriptors
	for i = 1,boxDescriptors_length,1 do
		local boxDescriptor = boxDescriptors[i]
		local x = boxDescriptor.x
		local y = boxDescriptor.y
		local z = boxDescriptor.z
		local position = {x=x, y=y, z=z}
		createBox(boxDescriptor.id, position, boxDescriptor.color)
	end
	for i = 1,boxDescriptors_length,1 do
		if checkIfBoxNeedsUpdate(g_boxes[i].position) then
			local p = g_boxes[i].position
			puts("Update: "..toString(p.x).." "..toString(p.y).." "..toString(p.z))
			g_boxes[i].needsUpdate = true
		end
	end
end


function consoleCommandCreateBox()
	local position = {x=0, y=0, z=0}
	local box_index = getBoxEntry(position)
	if box_index then
		info("createBox", "Cannot spawn box. Another box is currently at the origin.")
	else
		local materialName = g_materialNames[random()%#g_materialNames + 1]
		sqlite_exec(g_db, "INSERT INTO boxes(color, x, y, z) VALUES ('red', 0, 0, 0);")
		local result = sqlite_exec(g_db, "SELECT id FROM boxes WHERE x == 0 AND y == 0 AND z == 0;")
		local id = result[1].id
		createBox(id, position, materialName)
		sendEvent("create box", {position=position, materialName=materialName})
		info("createBox", "Created box at origin.")
	end
end

function setupConsoleCommands()
	createConsoleCommand("create_box", "consoleCommandCreateBox")
	createConsoleCommand("initialize_boxes", "initializeBoxes")
end


function clientConnect(clientNumber)
	g_clients[clientNumber].connected = true
	g_clients[clientNumber].authenticated = false
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
	g_clients[clientNumber].connected = false
	g_clients[clientNumber].authenticated = false
	serverState[clientNumber] = {}
	numClients = numClients - 1
	info("clientDisconnect", "Client " .. clientNumber .. " disconnected.")
end

function startup()
	info("startup", "Server started")

	-- We start with no connected clients.
	g_clients = {}
	for i = 1,maxClients,1 do
		g_clients[i] = {connected = false}
	end
	numClients = 0

	setupConsoleCommands()

	g_db = sqlite_open("server")
	g_secretsDb = sqlite_open("secrets")

	info("startup", "Loading world tree")

	g_cameraEntity = entity_createEntity(g_entity_type_entity)
	e = entity_linkChild(g_worldEntity, g_cameraEntity)
	if e ~= 0 then quit() end

	g_boxModel, e = mesh_load("blender/cube")
	if e ~= 0 then quit() end

	loadBoxes()

	info("startup", "Starting game")
end

function clientMain(clientIndex)
	-- Send boxes to clients when they connect.
	if not serverState[clientIndex].boxesCreated then
		for box_index = 1,g_boxes_length,1 do
			sendEventToClient(clientIndex,
							  "create initial box",
							  {position=g_boxes[box_index].position,
							   materialName=g_boxes[box_index].materialName})
		end
		if g_boxes_length == 0 then
			sendEventToClient(clientIndex, "no initial boxes", nil)
		end
	end

	-- Now here's the trick. We may receive lots of messages, or we may receive none.
	local messages = clientState[clientIndex]
	local messages_length = #messages
	if not serverState[clientIndex].boxesCreated then
		for messages_index = 1,messages_length,1 do
			if messages[messages_index].boxesCreated then
				serverState[clientIndex].boxesCreated = true
			end
		end
	end

	local events
	for messages_index = 1,messages_length,1 do
		server_processEvents(messages[messages_index].events, clientIndex)
	end
end

function main()
	local e
	-- Process clients
	for i = 1,maxClients,1 do
		if g_clients[i].connected then
			if g_clients[i].authenticated then
				clientMain(i)
			else
				if #clientState[i] > 0 then
					-- OK to hard code index because the client will be constantly streaming the identity to us.
					local username = clientState[i][1].username
					local password = clientState[i][1].password
					local query = "SELECT password FROM users WHERE name == '"..username.."'"
					local identities, e = sqlite_exec(g_secretsDb, query)
					if e ~= 0 then warning("main", "QUERY FAILED") end
					local identities_length = #identities
					if identities_length > 1 then
						error("main", "Multiple copies of user in database!")
					elseif identities_length < 1 then
						warning("main", "Login attempt from unrecognized user: \""..username..'".')
					else
						identity = identities[1]
						if password == identity.password then
							info("main", username.." logged in.")
							g_clients[i].authenticated = true
						else
							warning("main", username.." submitted the wrong password.")
							-- Haha look at all these trailing parentheses
						end
					end
					if not g_clients[i].authenticated then
						network_disconnect(i)
					end
				end
			end
		end
	end

	local movementScale = G_STANDARD_FRAMERATE * deltaT
	processBoxes(g_boxes, movementScale)
	sendQueuedEvents()

	-- puts("FPS: "..toString(1/deltaT))
end

function shutdown()
	info("shutdown", "Server quit");
	sqlite_close(g_db)
	sqlite_close(g_secretsDb)
end
