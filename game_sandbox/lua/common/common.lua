g_entity_type_none = 0
g_entity_type_entity = 1
g_entity_type_model = 2

g_worldEntity = 0

g_models = {}
g_entities = {}

g_materialNames = {
	"red",
	"blue",
	"green",
	"yellow",
	"cyan",
	"magenta",
	"black",
	"white",
	"clear",
	"frog"
}
g_materials = {}

g_modelForMaterial = {}

g_frame = 1

g_cursorOffset = {x=0, y=0, z=-100}
g_backOff = 0.001

Vec3_xn = {x=-1, y=0, z=0}
Vec3_xp = {x=1, y=0, z=0}
Vec3_yn = {x=0, y=-1, z=0}
Vec3_yp = {x=0, y=1, z=0}
Vec3_zn = {x=0, y=0, z=-1}
Vec3_zp = {x=0, y=0, z=1}

G_PI = 3.14159265358979323


function quit()
	cfg2_setVariable("quit")
end

function abort(message)
	critical_error("abort", message)
end

function createConsoleCommand(configVariable, callback)
	cfg2_setVariable("create command " .. configVariable)
	cfg2_setCallback(configVariable, callback)
end


function push(stack, item)
	stack[#stack+1] = item
end

function pop(stack)
	if #stack == 0 then
		return nil
	end
	local item = stack[#stack]
	stack[#stack] = nil
	return item
end


function vec3_add(a, b)
	return {x=a.x+b.x, y=a.y+b.y, z=a.z+b.z}
end

function vec3_subtract(a, b)
	return {x=a.x-b.x, y=a.y-b.y, z=a.z-b.z}
end

function vec3_scale(a, s)
	return {x=a.x*s, y=a.y*s, z=a.z*s}
end

function vec3_norm2(a)
	return a.x*a.x + a.y*a.y + a.z*a.z
end

function vec3_norm(a)
	return sqrt(a.x*a.x + a.y*a.y + a.z*a.z)
end

function vec3_normalize(a)
	return vec3_scale(a, 1.0/vec3_norm(a))
end

function vec3_equal(a, b)
	return a.x==b.x and a.y==b.y and a.z==b.z
end

function vec3_copy(v)
	return {x=v.x, y=v.y, z=v.z}
end

function aaToQuat(axisAngle)
	local angle = axisAngle.w
	local w_part = cos(angle/2)
	local v_part = sin(angle/2)
	return {w=w_part, x=axisAngle.x*v_part, y=axisAngle.y*v_part, z=axisAngle.z*v_part}
end

function eulerToQuat(euler)
	local pitch = Vec3_xp
	pitch.w = euler.pitch
	local yaw = Vec3_zp
	yaw.w = euler.yaw
	return hamiltonProduct(aaToQuat(yaw), aaToQuat(pitch))
end


function modelEntity_create(position, orientation, scale)
	local entity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, entity)
	e = entity_linkChild(entity, g_boxModel)
	entity_setScale(entity, scale)
	entity_setPosition(entity, position)
	entity_setOrientation(entity, orientation)
	return entity
end

function modelEntity_delete(entity)
	local e = entity_deleteEntity(entity)
	if e ~= 0 then return e end
	return entity_unlinkChild(g_cameraEntity, entity)
end


--------------------------
-- Game specific functions
--------------------------


g_worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}

g_gridSpacing = 40

g_boxes_length = 0
g_boxes_scale = g_gridSpacing/2

g_boxes = {}

g_boundingBoxRadius = g_gridSpacing * 20
g_boxTable = {}
G_GRAVITY = -0.4
G_JUMPVELOCITY = 7.0
G_PLAYER_BB = {mins={x=-g_gridSpacing*0.4, y=-g_gridSpacing*0.4, z=-0.7*g_gridSpacing},
			   maxs={x=g_gridSpacing*0.4, y=g_gridSpacing*0.4, z=0.2*g_gridSpacing}}
G_BOX_BB = {mins={x=-g_gridSpacing/2, y=-g_gridSpacing/2, z=-g_gridSpacing/2},
			maxs={x=g_gridSpacing/2, y=g_gridSpacing/2, z=g_gridSpacing/2}}


function snapComponentToGrid(component)
	return g_gridSpacing*round(component/g_gridSpacing)
end

function snapToGrid(point)
	return {x=snapComponentToGrid(point.x),
			y=snapComponentToGrid(point.y),
			z=snapComponentToGrid(point.z)}
end

-- Returns whether the spot on the grid is occupied by something (meaning can we move into it) and what box is in that
-- spot, if a box is in that spot. If the point is outside of the bounds of the map, then the result will be `true,
-- nil`.
function isOccupied(point)
	function negative() return false, nil end
	function affirmative(box_index) return true, box_index end
	function outOfBounds() return true, nil end
	local boundingBoxRadius = g_boundingBoxRadius
	-- Only work with grid coordinates.
	local point = snapToGrid(point)
	-- Check if outside bounds.
	if point.x > boundingBoxRadius then return outOfBounds() end
	if point.x < -boundingBoxRadius then return outOfBounds() end
	if point.y > boundingBoxRadius then return outOfBounds() end
	if point.y < -boundingBoxRadius then return outOfBounds() end
	if point.z > boundingBoxRadius/2 then return outOfBounds() end
	if point.z < -boundingBoxRadius/2 then return outOfBounds() end
	-- Check if spot contains a box.
	local box_index = getBoxEntry(point)
	if box_index then
		return affirmative(box_index)
	else
		return negative()
	end
end

-- traceComponent(4.3, "x", {x=23.3, y=4.0, z=-12.3}, -20, 20)
--
-- This function traces an axial line of grid points from `startPosition` to an end point determined by the axis name
-- and the axis offset (`endComponent`). It stops at the first box or out-of-bounds point.
function traceComponent(endComponent, componentName, startPosition, entityMin, entityMax)
	local extreme
	if endComponent > startPosition[componentName] then
		extreme = entityMax
	else
		extreme = entityMin
	end
	endComponent = endComponent + extreme
	local startPosition = {x=startPosition.x, y=startPosition.y, z=startPosition.z}
	startPosition[componentName] = startPosition[componentName] + extreme
	startPosition = snapToGrid(startPosition)

	local endPosition = {x=startPosition.x, y=startPosition.y, z=startPosition.z}
	endPosition[componentName] = snapComponentToGrid(endComponent)

	local displacement = endPosition[componentName] - startPosition[componentName]

	local iterations = round(abs(displacement / g_gridSpacing))
	local sign
	if displacement < 0 then
		sign = -1
	else
		sign = 1
	end

	local hit = false
	local gridOffset = 0
	local position = {x=startPosition.x, y=startPosition.y, z=startPosition.z}
	for gridOffset = 1,iterations,1 do
		position[componentName] = startPosition[componentName] + g_gridSpacing * gridOffset * sign
		local occupied, box_index = isOccupied(position)
		if occupied then
			return true, startPosition[componentName] + g_gridSpacing * (gridOffset - 0.5 - g_backOff) * sign - extreme
		end
	end
	return false, endComponent - extreme

end

-- `state` must have `position`, `velocity`, and `aabb` fields.
function boxMoveAndCollide(state)
	state.collided = false
	state.grounded = false
	local oldPosition = state.position
	local oldVelocity = state.velocity
	local newPosition = vec3_add(oldPosition, oldVelocity)
	local newVelocity = {x=oldVelocity.x, y=oldVelocity.y, z=oldVelocity.z}
	-- Do collision per-axis.
	components = {'x', 'y', 'z'}
	local position = {x=oldPosition.x, y=oldPosition.y, z=oldPosition.z}
	for componentIndex = 1,3,1 do
		component = components[componentIndex]
		local traceCollided, endComponent = traceComponent(newPosition[component],
														   component,
														   position,
														   state.aabb.mins[component],
														   state.aabb.maxs[component])
		if traceCollided then
			state.collided = true
			if component == 'z' and oldVelocity.z < 0 then
				state.grounded = true
			end
			newVelocity[component] = 0.0
		end
		position[component] = endComponent
	end
	state.position = position
	state.velocity = newVelocity
	return state
end

-- `state` must have `position`, `velocity`, and `aabb` fields.
function playerMoveAndCollide(state)
	state.collided = false
	state.grounded = false
	local oldPosition = state.position
	local oldVelocity = state.velocity
	local newPosition = vec3_add(oldPosition, oldVelocity)
	local newVelocity = {x=oldVelocity.x, y=oldVelocity.y, z=oldVelocity.z}
	local position = {x=oldPosition.x, y=oldPosition.y, z=oldPosition.z}
	-- Construct bounding volume vertices.
	local startingPoints = {}
	local aabb = state.aabb
	local extremeNames = {"mins", "maxs"}
	for z = 1,2,1 do
		extreme_z = aabb[extremeNames[z]].z
		for y = 1,2,1 do
			extreme_y = aabb[extremeNames[y]].y
			for x = 1,2,1 do
				extreme_x = aabb[extremeNames[x]].x
				startingPoints[#startingPoints+1] = {x=extreme_x, y=extreme_y, z=extreme_z}
			end
		end
	end
	-- Do collision per-axis.
	components = {'x', 'y', 'z'}
	for componentIndex = 1,3,1 do
		component = components[componentIndex]
		local minEndComponent = newPosition[component]
		local tracesCollided = false
		-- Trace from each corner of the cube (so 8 points), but we only need to check the points on the leading side (so 4 points).
		for j = 1,8,1 do
			-- But cull the points on the trailing face of the cube.
			if startingPoints[j][component] * (newPosition[component] - position[component]) >= 0 then
				-- Shift position to cube corner.
				local startingPoint = startingPoints[j]
				-- Trace.
				local traceCollided, endComponent = traceComponent(newPosition[component] + startingPoint[component],
																   component,
																   vec3_add(position, startingPoints[j]),
																   0,
																   0)
				endComponent = endComponent - startingPoint[component]
				if traceCollided then
					if abs(endComponent - position[component]) < abs(minEndComponent - position[component]) then
						minEndComponent = endComponent
					end
					tracesCollided = true
				end
			end
		end
		if tracesCollided then
			state.collided = true
			if component == 'z' and oldVelocity.z < 0 then
				state.grounded = true
			end
			-- newVelocity[component] = -0.001 * newVelocity[component]
			newVelocity[component] = 0.0
		end
		position[component] = minEndComponent
	end
	local delta = vec3_subtract(position, oldPosition)
	state.position = position
	state.velocity = newVelocity
	return state
end


function createBoxEntry(box_index, point)
	point = snapToGrid(point)
	local bt_xyz = g_boxTable
	local bt_yz = bt_xyz[point.x]
	if bt_yz == nil then
		bt_xyz[point.x] = {}
		bt_yz = bt_xyz[point.x]
		bt_yz.entries = 0
	end
	local bt_z = bt_yz[point.y]
	if bt_z == nil then
		bt_yz[point.y] = {}
		bt_z = bt_yz[point.y]
		bt_z.entries = 0
		bt_yz.entries = bt_yz.entries + 1
	end
	local spot = bt_z[point.z]
	if spot ~= nil then
		error("moveBox", "Box is already occupied.")
		return
	end
	bt_z[point.z] = box_index
	bt_z.entries = bt_z.entries + 1
end

-- Returns the index of the box at that point, or nil if a box doesn't exist there.
function getBoxEntry(point)
	point = snapToGrid(point)
	-- Check if spot contains a box.
	local bt_xyz = g_boxTable
	local bt_yz = bt_xyz[point.x]
	if bt_yz == nil then return nil end
	local bt_z = bt_yz[point.y]
	if bt_z == nil then return nil end
	local spot = bt_z[point.z]
	if spot == nil then return nil end
	-- Success.
	local box_index = spot
	return box_index
end

function moveBox(box_index, newPosition, oldPosition)
	oldPosition = snapToGrid(oldPosition)
	newPosition = snapToGrid(newPosition)
	if oldPosition.x == newPosition.x and oldPosition.y == newPosition.y and oldPosition.z == newPosition.z then
		return false
	end

	-- Find the current box entry.
	local bt_xyz = g_boxTable
	local bt_yz = bt_xyz[oldPosition.x]
	if bt_yz == nil then
		warning("moveBox", "Box does not have x-axis entry.")
		return false
	end
	local bt_z = bt_yz[oldPosition.y]
	if bt_z == nil then
		warning("moveBox", "Box does not have y-axis entry.")
		return false
	end
	local spot = bt_z[oldPosition.z]
	if spot == nil then
		warning("moveBox", "Box does not have z-axis entry.")
		return false
	end
	local box_index = spot

	-- Double check that we are moving the right box.
	if box_index ~= box_index then
		warning("moveBox", "The box being moved is not the box that is at this point.")
		return false
	end

	-- Delete existing entry.
	bt_z[oldPosition.z] = nil
	bt_z.entries = bt_z.entries - 1
	if bt_z.entries == 0 then
		bt_z.entries = nil

		bt_yz[oldPosition.y] = nil
		bt_yz.entries = bt_yz.entries - 1
		if bt_yz.entries == 0 then
			bt_yz.entries = nil

			bt_xyz[oldPosition.x] = nil
		end
	end

	-- Create new entry.
	createBoxEntry(box_index, newPosition)

	-- if G_SERVER then
	-- 	sendEvent('move box', {oldPosition=oldPosition, newPosition=newPosition})
	-- end

	return true
end

function checkIfBoxNeedsUpdate(position)
	local centerPosition = snapToGrid(position)
	-- Doubt these are the correct directions. What is important is that they are all different cardinal directions.
	local d = vec3_copy(centerPosition)
	d.z = d.z - g_gridSpacing
	local n = vec3_copy(centerPosition)
	n.x = n.x + g_gridSpacing
	local s = vec3_copy(centerPosition)
	s.x = s.x - g_gridSpacing
	local e = vec3_copy(centerPosition)
	e.y = e.y + g_gridSpacing
	local w = vec3_copy(centerPosition)
	w.y = w.y - g_gridSpacing
	local directions = {n, s, e, w}
	local directions_length = #directions
	local occupied, _ = isOccupied(d)
	if occupied then return false end
	local box_count = 0
	for i = 1,directions_length,1 do
		local direction = directions[i]
		local box_index = getBoxEntry(direction)
		if box_index then
			box_count = box_count + 1
		end
	end
	return box_count < 3
end

function updateNeighborBoxes(position)
	local centerPosition = snapToGrid(position)
	-- Doubt these are the correct directions. What is important is that they are all different cardinal directions.
	local u = vec3_copy(centerPosition)
	u.z = u.z + g_gridSpacing
	local n = vec3_copy(centerPosition)
	n.x = n.x + g_gridSpacing
	local s = vec3_copy(centerPosition)
	s.x = s.x - g_gridSpacing
	local e = vec3_copy(centerPosition)
	e.y = e.y + g_gridSpacing
	local w = vec3_copy(centerPosition)
	w.y = w.y - g_gridSpacing
	local directions = {u, n, s, e, w}
	local directions_length = #directions
	local box_count = 0
	for i = 1,directions_length,1 do
		local direction = directions[i]
		if checkIfBoxNeedsUpdate(direction) then
			local box_index = getBoxEntry(direction)
			if box_index then
				g_boxes[box_index].needsUpdate = true
			end
		end
	end
end

function processBoxes(boxes)
	for i = 1,g_boxes_length,1 do
		if boxes[i].needsUpdate then
			puts("Update: "..toString(boxes[i].position.x).." "..toString(boxes[i].position.y).." "..toString(boxes[i].position.z))
			boxes[i].velocity.z = boxes[i].velocity.z + G_GRAVITY
			local oldPosition = boxes[i].position
			boxes[i] = boxMoveAndCollide(boxes[i])
			local newPosition = boxes[i].position
			if vec3_equal(oldPosition, newPosition) then
				boxes[i].needsUpdate = false
			end
			local moved = moveBox(i, newPosition, oldPosition)
			if moved then
				updateNeighborBoxes(oldPosition)
			end
			if G_SERVER and moved then
				local id = boxes[i].id
				local n = snapToGrid(newPosition)
				n.z = n.z + g_backOff
				sqlite_exec(g_db, "UPDATE boxes SET x = "..n.x..", y = "..n.y..", z = "..n.z.." WHERE id == "..id..";")
			end
			entity_setPosition(boxes[i].entity, boxes[i].position)
		end
	end
end


function createBox(id, position, materialName)
	local box = {}
	local boxEntity, e = entity_createEntity(g_entity_type_model)
	e = entity_linkChild(g_cameraEntity, boxEntity)
	if G_CLIENT then
		model = g_modelForMaterial[materialName]
	else
		model = g_boxModel
	end
	e = entity_linkChild(boxEntity, model)
	box.id = id
	box.entity = boxEntity
	-- Enable gravity.
	box.needsUpdate = false

	entity_setScale(boxEntity, g_boxes_scale)
	box.position = position
	entity_setPosition(boxEntity, box.position)
	entity_setOrientation(boxEntity, {w=1, x=0, y=0, z=0})

	box.velocity = {x=0, y=0, z=0}
	box.aabb = G_BOX_BB

	box.materialName = materialName

	push(g_boxes, box)
	g_boxes_length = #g_boxes
	createBoxEntry(g_boxes_length, box.position)

	if G_CLIENT then
		e = entity_linkMaterial(boxEntity, g_materials[materialName])
	end
	return e
end

function changeBoxMaterial(box, materialName)
	-- Delete.
	local e = 0
	local box_entity = box.entity
	if G_CLIENT then
		if not g_materials[materialName] then return 0 end
		e = entity_deleteEntity(box_entity)
		if e ~= 0 then return e end
		e = entity_unlinkChild(g_cameraEntity, box_entity)
		if e ~= 0 then return e end

		-- Create.
		box_entity, e = entity_createEntity(g_entity_type_model)
		if e ~= 0 then return e end
		box.entity = box_entity
		e = entity_linkChild(g_cameraEntity, box_entity)
		if e ~= 0 then return e end
		e = entity_linkChild(box_entity, g_modelForMaterial[materialName])
		if e ~= 0 then return e end
		entity_setScale(box_entity, g_boxes_scale)
		entity_setPosition(box_entity, box.position)
		entity_setOrientation(box_entity, {w=1, x=0, y=0, z=0})
	end

	-- Set material.
	box.materialName = materialName
	if G_CLIENT then
		e = entity_linkMaterial(box_entity, g_materials[materialName])
	end
	return e
end


function calculateCursorPosition(position, orientation)
	return snapToGrid(vec3_add(position, vec3_rotate(g_cursorOffset, orientation)))
end
