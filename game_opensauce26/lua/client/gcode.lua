gcode = {}


-- Assumptions:
-- Orca emits 1 command per line when emitting for Klipper. Therefore assume there is only ever 1 command per line.
-- The file I'm testing on doesn't have any parameters I care about, so don't parse parameters (or even the rest of the line) for commands I want to ignore.
-- We might not read full lines from the named pipe, which means it's not guaranteed that we will be able to read complete commands.


-- ;
-- EXCLUDE_OBJECT_DEFINE NAME=aircraft.stl_id_0_copy_0 CENTER=125,125 POLYGON=[[73.0457,127.255],[73.0579,125.124],[73.0598,124.838],[135.69,70.9827],[136.171,70.9457],[137.561,70.9399],[141.59,70.9898],[175.74,110.084],[176.954,123.598],[175.709,140.09],[141.48,179.06],[135.58,179.053],[73.0457,127.255]] ; Ignore.
-- M73 P0 R40 ; Ignore.
-- M190 S105 ; Ignore.
-- M109 S260 ; Ignore.
-- PRINT_START EXTRUDER=260 BED=105 ; Ignore.
-- G90 ; Use absolute coordinates. Ignore for now?
-- G21 ; Use millimeter coordinates. Ignore for now?
-- M83 ; Use relative distances for extrusion. Ignore for now?
-- M106 S0 ; Ignore.
-- G92 E0 ; Set position. Important.
-- G1 E-.8 F1800 ; Move. Important.
-- SET_VELOCITY_LIMIT ACCEL=500 ACCEL_TO_DECEL=250 ; Ignore?
-- EXCLUDE_OBJECT_START NAME=aircraft.stl_id_0_copy_0 ; Ignore.
-- G1 Z.6 F21000
-- G1 X84.374 Y122.676
-- G1 Z.6
-- G1 F3000
-- G1 X84.309 Y122.758 E.00275
-- PRINT_END

-- Cast E to a bool.
-- X, Y, Z are important.
-- Ignore F for now (in the test code), and go at max speed.


function gcode.resetState()
	gcode.buffer = ""
	gcode.commands_ignored = {"EXCLUDE_OBJECT_DEFINE", "M73", "M190", "M109", "PRINT_START", "G90", "G21", "M83", "M106", "SET_VELOCITY_LIMIT", "EXCLUDE_OBJECT_START", "EXCLUDE_OBJECT_END"}
end
gcode.resetState()

-- (Void -> (string::String | nil)) -> Void
function gcode.register_reader(reader)
	gcode.read = reader
end


function gcode.elementInList(element, list)
	local index = 1
	while index <= #list do
		if element == list[index] then  return true  end
		index = index + 1
	end
	return false
end


-- String -> String, Signal
function gcode.parse_comment(string)
    return signal
end

function gcode.getLine()
	-- Fetch any new text from the pipe.
	local string = gcode.read()
	if string then
		gcode.buffer = gcode.buffer .. string
	end
	-- Find newline and extract the line, if it exists.
	local newline_index = 1
	while true do
		if newline_index > #gcode.buffer then
			return nil
		end
		if "\n" == string_at(gcode.buffer, newline_index) then
			break
		end
		newline_index = newline_index + 1
	end
	local line = string_sub(gcode.buffer, 1, newline_index-1)
	-- Trim line we read from the front of the buffer to avoid a memory leak.
	gcode.buffer = string_sub(gcode.buffer, newline_index + 1, #gcode.buffer)
	return line
end

function gcode.getWord(line)
	-- Real gcode seems to allow mashing words together, but we'll assume words are separated by
	-- spaces.
	local word_index = 1
	while (word_index <= #line) and (' ' ~= string_at(line, word_index)) do
		word_index = word_index + 1
	end
	word = string_sub(line, 1, word_index-1)
	if word_index > #line then  word_index = #line  end
	line = string_sub(line, word_index+1, #line)
	if word == "" then  return line, nil  end
	return line, word
end

-- Requires:
-- string_sub(string::String, index::Integer) -> character::Integer
function gcode.getNextCommand_nonblocking()
	local command = {}
	local line = gcode.getLine()
	-- Check if we have a complete line.
	if not line then  return nil  end

	-- Strip comments.
	local comment_index = 1
	while comment_index <= #line do
		if ';' == string_at(line, comment_index) then
			line = string_sub(line, 1, comment_index-1)
			break
		end
		comment_index = comment_index + 1
	end
	if #line == 0 then  return nil  end

	-- Parse line.
	command.line = line
	line, command.opcode = gcode.getWord(line)
	if gcode.elementInList(command.opcode, gcode.commands_ignored) then
		return nil
	end
	local defaultWords = "EFXYZ"
	if gcode.command_previous then
	--  and (command.opcode == gcode.command_previous.opcode)
		for i = 1,#defaultWords do
			command[string_at(defaultWords, i)] = gcode.command_previous[string_at(defaultWords, i)]
		end
	else
		for i = 1,#defaultWords do
			command[string_at(defaultWords, i)] = "0"
		end
	end
	while true do
		line, word = gcode.getWord(line)
		if not word then  break  end
		command[string_first(word)] = string_sub(word, 2, #word)
	end

	gcode.command_previous = command
	return command
end

function gcode.getNextCommand()
	local command = nil
	while not command do
		command = gcode.getNextCommand_nonblocking()
	end
	return command
end


-- return gcode
