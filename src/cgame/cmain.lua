--[[
This should be a completely sandboxed environment.
No libraries have been loaded, and the only resources available to the script are whitelisted C functions.
--]]

puts("Lua start");

while true do

	puts("Lua render")
	render()

	puts("Lua getInput")
	input = (getInput())
	if input then
		puts("Lua got input");
		if input == 256 then
			break
		end
	end
	
	puts("Lua looping")
end

puts("Lua end")
