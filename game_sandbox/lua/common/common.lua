g_entity_type_none = 0
g_entity_type_entity = 1
g_entity_type_model = 2

g_worldEntity = 0

g_models = {}
g_entities = {}

g_frame = 1


function quit()
	cfg2_setVariable("quit")
end

function abort(message)
	critical_error("abort", message)
end


function aaToQuat(axisAngle)
	angle = axisAngle.w
	w_part = cos(angle)/2
	v_part = sin(angle)/2
	return {w=w_part, x=axisAngle.x*v_part, y=axisAngle.y*v_part, z=axisAngle.z*v_part}
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

g_gridSpacing = 100

function snapToGrid(point)
	function snapScalarToPeriod(scalar)
		return g_gridSpacing*round(scalar/g_gridSpacing)
	end
	return {x=snapScalarToPeriod(point.x),
			y=snapScalarToPeriod(point.y),
			z=snapScalarToPeriod(point.z)}
end

