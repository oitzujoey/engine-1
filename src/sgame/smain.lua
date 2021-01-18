
l_log_info("main", "Server started")
-- box = l_loadObj("resources/microcode-v1.1.2.obj")
-- box = l_loadObj("resources/untitled.obj")
-- l_puts(box)

-- file = l_vfs_getFileText("src/sgame/smain.lua")
-- l_puts(file)

-- Entity list constant indices
type_none = 0
type_entity = 1
type_model = 2
worldEntity = 0

-- Textures
texturePath = "oolite-binary-resources/Textures/"
modelPath = "oolite-binary-resources/Models/"
HullTexture = "oolite_cobra3_diffuse.png"
GunTexture = "oolite_cobra3_subents.png"

l_log_info("main", "Loading world tree")

cobra3Model, error = l_loadOoliteModel(modelPath .. "oolite_cobra3.dat")
if cobra3Model ~= -1 then
	cobra3Entity, error = l_createEntity(type_model)
	error = l_entity_linkChild(worldEntity, cobra3Entity)
	error = l_entity_linkChild(cobra3Entity, cobra3Model)
	l_entity_setPosition(cobra3Entity, {x=0, y=0, z=-20})
	l_entity_setOrientation(cobra3Entity, {w=0, x=-0.5, y=-1, z=0.5})
	-- l_entity_setOrientation(cobra3Entity, {w=1, x=0, y=0, z=0})
	
	-- l_snetwork_sendEntityTree()
	-- l_renderEntity(worldEntity, 0.0, 0.0, 1.0, 1.0)
end

-- // Load the Oolite model specified by the string. Return the newly created entity index and the name of the entity.
-- integer entityIndex, string name = l_loadOoliteModel(string fileName)

-- // Create an entity (if a freed one doesn't already exist) with name "name" and type "type" and add it to the entity list. Return the index.
-- integer index = l_createEntity(string name, integer type)

-- // Link the two entities if possible. Non-entity structures are allowed to be linked as long as the type of the entity is set properly.
-- l_entity_linkChild(integer parentIndex, integer childIndex)

l_log_info("main", "Starting game")

while l_checkQuit() == 0 do
	l_main_housekeeping()
end

l_log_info("main", "Server quit");
