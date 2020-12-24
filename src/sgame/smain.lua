
l_log_info("main", "Server started")
-- box = l_loadObj("resources/microcode-v1.1.2.obj")
box = l_loadObj("resources/untitled.obj")
-- l_puts(box)

-- file = l_vfs_getFileText("src/sgame/smain.lua")
-- l_puts(file)

type_none = 0
type_model = 1
worldEntity = 0
asteroidModel, name = l_loadOoliteModel("Models/asteroid.dat")
if asteroidModel ~= -1 then
	asteroidEntity = l_createEntity("asteroid", type_model)
	l_entity_addChild(worldEntity, asteroidEntity)
	l_entity_addChild(asteroidEntity, asteroidModel)
	
	-- l_server_sendState
	-- l_renderEntity(worldEntity, 0.0, 0.0, 1.0, 1.0)
end

l_log_info("main", "Server quit");
