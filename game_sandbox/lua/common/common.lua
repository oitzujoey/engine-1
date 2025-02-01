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
G_GRAVITY = -1.0e0
G_JUMPVELOCITY = 3.0e1


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
g_boundingBoxRadius = g_gridSpacing * 100
g_boxTable = {}

function snapComponentToGrid(component)
	return g_gridSpacing*round(component/g_gridSpacing)
end

function snapToGrid(point)
	return {x=snapComponentToGrid(point.x),
			y=snapComponentToGrid(point.y),
			z=snapComponentToGrid(point.z)}
end

-- Returns whether the point is in the map boundary.
function playerIsInBounds(point)
	function inBounds() return true end
	function outOfBounds() return false end
	local boundingBoxRadius = g_boundingBoxRadius
	-- Check if outside bounds.
	if point.x > boundingBoxRadius then return outOfBounds() end
	if point.x < -boundingBoxRadius then return outOfBounds() end
	if point.y > boundingBoxRadius then return outOfBounds() end
	if point.y < -boundingBoxRadius then return outOfBounds() end
	if point.z > boundingBoxRadius/2 then return outOfBounds() end
	if point.z < -boundingBoxRadius/2 then return outOfBounds() end
	return inBounds()
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

function playerCollide(state)
	-- TODO: This is wrong. We can't just average the snapped old and new positions because it's possible we warped
	--       through a block. We need to run a trace function to stop on the first block we encounter. Only after we
	--       have the right block can we average the positions to get the intersection.
	state.collided = false
	state.grounded = false
	local oldPosition = state.position
	local oldVelocity = state.velocity
	local newPosition = vec3_add(oldPosition, oldVelocity)
	local newVelocity = {x=oldVelocity.x, y=oldVelocity.y, z=oldVelocity.z}
	-- Do collision per-axis.
	local x = {x=newPosition.x, y=oldPosition.y, z=oldPosition.z}
	if not playerIsInBounds(x) then
		state.collided = true
		x.x = (snapComponentToGrid(x.x) + snapComponentToGrid(oldPosition.x))/2
		newVelocity.x = 0.0
	end
	newPosition.x = x.x
	local y = {x=x.x, y=newPosition.y, z=x.z}
	if not playerIsInBounds(y) then
		state.collided = true
		y.y = (snapComponentToGrid(y.y) + snapComponentToGrid(x.y))/2
		newVelocity.y = 0.0
	end
	newPosition.y = y.y
	local z = {x=y.x, y=y.y, z=newPosition.z}
	if not playerIsInBounds(z) then
		state.collided = true
		if oldVelocity.z < 0 then state.grounded = true end
		z.z = (snapComponentToGrid(z.z) + snapComponentToGrid(y.z))/2
		newVelocity.z = 0.0
	end
	newPosition.z = z.z
	local delta = vec3_subtract(newPosition, oldPosition)
	-- puts("delta {")
	-- puts(delta.x)
	-- puts(delta.y)
	-- puts(delta.z)
	-- puts("}")
	state.position = newPosition
	state.velocity = newVelocity
	return state
end
