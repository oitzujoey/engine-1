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
G_GRAVITY = -1.0


function quit()
	cfg2_setVariable("quit")
end

function abort(message)
	critical_error("abort", message)
end


function vec3_add(a, b)
	return {x=a.x+b.x, y=a.y+b.y, z=a.z+b.z}
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

g_gridSpacing = 20
g_boundingBoxRadius = g_gridSpacing * 100
g_boxTable = {}

function snapToGrid(point)
	function snapScalarToPeriod(scalar)
		return g_gridSpacing*round(scalar/g_gridSpacing)
	end
	return {x=snapScalarToPeriod(point.x),
			y=snapScalarToPeriod(point.y),
			z=snapScalarToPeriod(point.z)}
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
