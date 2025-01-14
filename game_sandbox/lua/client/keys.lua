
function keys_createFullBind(key, configVariable, callbackDown, callbackUp)
	cfg2_setVariable("create none +" .. configVariable)
	cfg2_setVariable("create none -" .. configVariable)
	cfg2_setCallback("+" .. configVariable, callbackDown)
	cfg2_setCallback("-" .. configVariable, callbackUp)
	cfg2_setVariable("bind " .. key .. " +" .. configVariable .. " -" .. configVariable)
end

function keys_createHalfBind(key, configVariable, callback)
	cfg2_setVariable("create none " .. configVariable)
	cfg2_setCallback(configVariable, callback)
	cfg2_setVariable("bind " .. key .. " " .. configVariable)
end


--[[
Key bind functions are run before networking happens.
`main` is run after networking happens.
--]]

function key_right_d()
	Keys.right = true
	if not clientState.keys.right then
		clientState.keys.right = true
		clientState.keys.left = false
	end
end

function key_right_u()
	Keys.right = false
	clientState.keys.right = false
	if Keys.left then
		clientState.keys.left = true
	end
end

function key_left_d()
	Keys.left = true
	if not clientState.keys.left then
		clientState.keys.left = true
		clientState.keys.right = false
	end
end

function key_left_u()
	Keys.left = false
	clientState.keys.left = false
	if Keys.right then
		clientState.keys.right = true
	end
end

function key_up_d()
	Keys.up = true
	if not clientState.keys.up then
		clientState.keys.up = true
		clientState.keys.down = false
	end
end

function key_up_u()
	Keys.up = false
	clientState.keys.up = false
	if Keys.down then
		clientState.keys.down = true
	end
end

function key_down_d()
	Keys.down = true
	if not clientState.keys.down then
		clientState.keys.down = true
		clientState.keys.up = false
	end
end

function key_down_u()
	Keys.down = false
	clientState.keys.down = false
	if Keys.up then
		clientState.keys.up = true
	end
end

function key_yawLeft_d()
	Keys.yawLeft = true
	if not clientState.keys.yawLeft then
		clientState.keys.yawLeft = true
		clientState.keys.yawRight = false
	end
end

function key_yawLeft_u()
	Keys.yawLeft = false
	clientState.keys.yawLeft = false
	if Keys.yawRight then
		clientState.keys.yawRight = true
	end
end

function key_yawRight_d()
	Keys.yawRight = true
	if not clientState.keys.yawRight then
		clientState.keys.yawRight = true
		clientState.keys.yawLeft = false
	end
end

function key_yawRight_u()
	Keys.yawRight = false
	clientState.keys.yawRight = false
	if Keys.yawLeft then
		clientState.keys.yawLeft = true
	end
end

function key_accelerate_d()
	Keys.accelerate = true
	if not clientState.keys.accelerate then
		clientState.keys.accelerate = true
		clientState.keys.decelerate = false
	end
end

function key_accelerate_u()
	Keys.accelerate = false
	clientState.keys.accelerate = false
	if Keys.decelerate then
		clientState.keys.decelerate = true
	end
end

function key_decelerate_d()
	Keys.decelerate = true
	if not clientState.keys.decelerate then
		clientState.keys.decelerate = true
		clientState.keys.accelerate = false
	end
end

function key_decelerate_u()
	Keys.decelerate = false
	clientState.keys.decelerate = false
	if Keys.accelerate then
		clientState.keys.accelerate = true
	end
end
