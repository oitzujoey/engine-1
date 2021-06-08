
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
