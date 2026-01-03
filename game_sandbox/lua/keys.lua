
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


function key_forward_d()
	Keys.forward = true
	if not g_forward then
		g_forward = true
		g_backward = false
	end
end

function key_forward_u()
	Keys.forward = false
	g_forward = false
	if Keys.backward then
		g_backward = true
	end
end

function key_backward_d()
	Keys.backward = true
	if not g_backward then
		g_backward = true
		g_forward = false
	end
end

function key_backward_u()
	Keys.backward = false
	g_backward = false
	if Keys.forward then
		g_forward = true
	end
end


function key_strafeLeft_d()
	Keys.strafeLeft = true
	if not g_strafeLeft then
		g_strafeLeft = true
		g_strafeRight = false
	end
end

function key_strafeLeft_u()
	Keys.strafeLeft = false
	g_strafeLeft = false
	if Keys.strafeRight then
		g_strafeRight = true
	end
end

function key_strafeRight_d()
	Keys.strafeRight = true
	if not g_strafeRight then
		g_strafeRight = true
		g_strafeLeft = false
	end
end

function key_strafeRight_u()
	Keys.strafeRight = false
	g_strafeRight = false
	if Keys.strafeLeft then
		g_strafeLeft = true
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

function key_space_d()
	g_jump = true
end

function key_space_u()
	g_jump = false
end


function mouse_leftPress()
	g_selectCube = true
end

function mouse_leftRelease()
	info("mouse_leftRelease", "Mouse release!")
end

function mouse_rightPress()
	g_jump = true
end

function mouse_rightRelease()
	g_jump = false
end

function mouse_motion(motionEvent)
	g_mouse = {x=motionEvent.x,
			   y=motionEvent.y,
			   delta_x=motionEvent.delta_x*g_sensitivity,
			   delta_y=motionEvent.delta_y*g_sensitivity}
end
