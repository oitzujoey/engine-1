g_entity_type_none = 0
g_entity_type_entity = 1
g_entity_type_model = 2

g_worldEntity = 0

g_models = {}
g_entities = {}


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
