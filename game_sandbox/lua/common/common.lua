g_entity_type_none = 0
g_entity_type_entity = 1
g_entity_type_model = 2

g_worldEntity = 0

g_models = {}
g_entities = {}

g_frame = 1

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
	e = entity_linkChild(entity, boxModel)
	entity_setScale(entity, scale)
	entity_setPosition(entity, position)
	entity_setOrientation(entity, orientation)
	return entity
end


--------------------------
-- Game specific functions
--------------------------


g_gridSpacing = 40
g_boundingBoxRadius = g_gridSpacing * 20
g_boxTable = {}
G_GRAVITY = -1.0e0
G_JUMPVELOCITY = 3.0e1
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
	function affirmative(boxNumber) return true, boxNumber end
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
	local boxNumber = getBoxEntry(point)
	if boxNumber then
		return affirmative(boxNumber)
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
		local occupied, boxEntity = isOccupied(position)
		if occupied then
			local backOff = 0.001
			return true, startPosition[componentName] + g_gridSpacing * (gridOffset - 0.5 - backOff) * sign - extreme
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
	local delta = vec3_subtract(position, oldPosition)
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


function createBoxEntry(boxEntity, point)
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
	bt_z[point.z] = boxEntity
	bt_z.entries = bt_z.entries + 1
end

function getBoxEntry(point)
	-- Check if spot contains a box.
	local bt_xyz = g_boxTable
	local bt_yz = bt_xyz[point.x]
	if bt_yz == nil then return nil end
	local bt_z = bt_yz[point.y]
	if bt_z == nil then return nil end
	local spot = bt_z[point.z]
	if spot == nil then return nil end
	-- Success.
	local boxNumber = spot
	return boxNumber
end

function moveBox(boxEntity, newPosition, oldPosition)
	oldPosition = snapToGrid(oldPosition)
	newPosition = snapToGrid(newPosition)

	-- Find the current box entry.
	local bt_xyz = g_boxTable
	local bt_yz = bt_xyz[oldPosition.x]
	if bt_yz == nil then
		error("moveBox", "Box does not have x-axis entry.")
		return
	end
	local bt_z = bt_yz[oldPosition.y]
	if bt_z == nil then
		error("moveBox", "Box does not have y-axis entry.")
		return
	end
	local spot = bt_z[oldPosition.z]
	if spot == nil then
		error("moveBox", "Box does not have z-axis entry.")
		return
	end
	local boxNumber = spot

	-- Double check that we are moving the right box.
	if boxNumber ~= boxEntity then
		error("moveBox", "The box being moved is not the box that is at this point.")
		return
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

			bt_xyz[oldPosition.z] = nil
		end
	end

	-- Create new entry.
	createBoxEntry(boxEntity, newPosition)
end

function processBoxes(boxes)
	for i = 1,g_boxes_length,1 do
		boxes[i].velocity.z = boxes[i].velocity.z + G_GRAVITY
		oldPosition = boxes[i].position
		boxes[i] = boxMoveAndCollide(boxes[i])
		moveBox(boxes[i].entity, boxes[i].position, oldPosition)
		entity_setPosition(boxes[i].entity, boxes[i].position)
	end
end
