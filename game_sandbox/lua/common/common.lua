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
G_PLAYER_BB = {mins={x=-g_gridSpacing/2, y=-g_gridSpacing/2, z=-1.8*g_gridSpacing},
			   maxs={x=g_gridSpacing/2, y=g_gridSpacing/2, z=0.2*g_gridSpacing}}


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
	local bt_xyz = g_boxTable
	local bt_yz = bt_xyz[point.x]
	if bt_yz == nil then return negative() end
	local bt_z = bt_yz[point.y]
	if bt_z == nil then return negative() end
	local spot = bt_z[point.z]
	if spot == nil then return negative() end
	-- Success.
	local boxNumber = spot
	return affirmative(boxNumber)
end

-- traceComponent(4.3, "x", {x=23.3, y=4.0, z=-12.3}, -20, 20)
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

function playerCollide(state)
	state.collided = false
	state.grounded = false
	local oldPosition = state.position
	local oldVelocity = state.velocity
	local newPosition = vec3_add(oldPosition, oldVelocity)
	local newVelocity = {x=oldVelocity.x, y=oldVelocity.y, z=oldVelocity.z}
	-- Do collision per-axis.
	local traceCollided, endComponent = traceComponent(newPosition.x,
													   "x",
													   {x=oldPosition.x, y=oldPosition.y, z=oldPosition.z},
													   G_PLAYER_BB.mins.x,
													   G_PLAYER_BB.maxs.x)
	if traceCollided then
		state.collided = true
		newVelocity.x = 0.0
	end
	newPosition.x = endComponent
	local traceCollided, endComponent = traceComponent(newPosition.y,
													   "y",
													   {x=newPosition.x, y=oldPosition.y, z=oldPosition.z},
													   G_PLAYER_BB.mins.y,
													   G_PLAYER_BB.maxs.y)
	if traceCollided then
		state.collided = true
		newVelocity.y = 0.0
	end
	newPosition.y = endComponent
	local traceCollided, endComponent = traceComponent(newPosition.z,
													   "z",
													   {x=newPosition.x, y=newPosition.y, z=oldPosition.z},
													   G_PLAYER_BB.mins.z,
													   G_PLAYER_BB.maxs.z)
	if traceCollided then
		state.collided = true
		if oldVelocity.z < 0 then state.grounded = true end
		newVelocity.z = 0.0
	end
	newPosition.z = endComponent
	local delta = vec3_subtract(newPosition, oldPosition)
	state.position = newPosition
	state.velocity = newVelocity
	return state
end
