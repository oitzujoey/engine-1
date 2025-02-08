
function keys_createFullBind(key, configVariable, callbackDown, callbackUp)
	cfg2_setVariable("create command +" .. configVariable)
	cfg2_setVariable("create command -" .. configVariable)
	cfg2_setCallback("+" .. configVariable, callbackDown)
	cfg2_setCallback("-" .. configVariable, callbackUp)
	cfg2_setVariable("bind " .. key .. " +" .. configVariable .. " -" .. configVariable)
end

function keys_createHalfBind(key, configVariable, callback)
	cfg2_setVariable("create command " .. configVariable)
	cfg2_setCallback(configVariable, callback)
	cfg2_setVariable("bind " .. key .. " " .. configVariable)
end

function keys_createMouseBind(callback)
	cfg2_setVariable("bindMouse "..callback)
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


function key_forward_d()
	Keys.forward = true
	if not clientState.keys.forward then
		clientState.keys.forward = true
		clientState.keys.backward = false
	end
end

function key_forward_u()
	Keys.forward = false
	clientState.keys.forward = false
	if Keys.backward then
		clientState.keys.backward = true
	end
end

function key_backward_d()
	Keys.backward = true
	if not clientState.keys.backward then
		clientState.keys.backward = true
		clientState.keys.forward = false
	end
end

function key_backward_u()
	Keys.backward = false
	clientState.keys.backward = false
	if Keys.forward then
		clientState.keys.forward = true
	end
end


function key_strafeLeft_d()
	Keys.strafeLeft = true
	if not clientState.keys.strafeLeft then
		clientState.keys.strafeLeft = true
		clientState.keys.strafeRight = false
	end
end

function key_strafeLeft_u()
	Keys.strafeLeft = false
	clientState.keys.strafeLeft = false
	if Keys.strafeRight then
		clientState.keys.strafeRight = true
	end
end

function key_strafeRight_d()
	Keys.strafeRight = true
	if not clientState.keys.strafeRight then
		clientState.keys.strafeRight = true
		clientState.keys.strafeLeft = false
	end
end


function key_color1_d() Keys.color = 1 end
function key_color2_d() Keys.color = 2 end
function key_color3_d() Keys.color = 3 end
function key_color4_d() Keys.color = 4 end
function key_color5_d() Keys.color = 5 end
function key_color6_d() Keys.color = 6 end
function key_color7_d() Keys.color = 7 end
function key_color8_d() Keys.color = 8 end
function key_color9_d() Keys.color = 9 end
function key_color0_d() Keys.color = 10 end


function key_strafeRight_u()
	Keys.strafeRight = false
	clientState.keys.strafeRight = false
	if Keys.strafeLeft then
		clientState.keys.strafeLeft = true
	end
end


function mouse_leftPress()
	info("mouse_leftPress", "Mouse press!")
end

function mouse_leftRelease()
	info("mouse_leftRelease", "Mouse release!")
end

function mouse_rightPress()
	clientState.keys.jump = true
end

function mouse_rightRelease()
	clientState.keys.jump = false
end

function mouse_motion(motionEvent)
	g_mouse = motionEvent
	clientState.mouse = {x=motionEvent.x,
						 y=motionEvent.y,
						 delta_x=motionEvent.delta_x*g_sensitivity,
						 delta_y=motionEvent.delta_y*g_sensitivity}
end
