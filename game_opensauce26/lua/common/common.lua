g_entity_type_none = 0
g_entity_type_entity = 1
g_entity_type_model = 2

g_worldEntity = 0

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
G_GRAVITY = -0.4
G_JUMPVELOCITY = 7.0


function quit()
	cfg2_setVariable("quit")
end

function abort(message)
	critical_error("abort", message)
    cfg2_setVariable("quit")
end

function createConsoleCommand(configVariable, callback)
	cfg2_setVariable("create command " .. configVariable)
	cfg2_setCallback(configVariable, callback)
end


function string_at(string, i)
	return string_sub(string, i, i)
end

function string_first(string)
	return string_sub(string, 1, 1)
end

function string_rest(string)
	return string_sub(string, 2, #string)
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
