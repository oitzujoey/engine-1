
-- `include` is the engine's version of `require`.
include "../common/extra.lua"

function startup()
	
	info("startup", "Server started")
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
	
	info("startup", "Loading world tree")
	
	speed = 1
	position = {x=0, y=0, z=200}
	
	cobra3Model, error = loadOoliteModel(modelPath .. "oolite_cobra3.dat")
	if cobra3Model ~= -1 then
		cobra3Entity, error = createEntity(type_model)
		info("startup", "Linking entity to world.")
		error = entity_linkChild(worldEntity, cobra3Entity)
		info("startup", "Linking model to entity.")
		error = entity_linkChild(cobra3Entity, cobra3Model)
		entity_setPosition(cobra3Entity, position)
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
	
	-- Rotate the world around the camera a bit.
	
	rotation = {w=0.99985, x=-0.005, y=-0.01, z=0.005}
	orientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	worldRotation = {w=0.99985, x=0.005, y=0.01, z=-0.005}
	worldCounterRotation = {w=0.99985, x=-0.005, y=-0.01, z=0.005}
	worldOrientation = {w=1.0, x=0.0, y=0.0, z=0.0}
	
	TurnRate = 100
	
	Vec3_xn = {x=-1, y=0, z=0}
	Vec3_xp = {x=1, y=0, z=0}
	Vec3_yn = {x=0, y=-1, z=0}
	Vec3_yp = {x=0, y=1, z=0}
	Vec3_zn = {x=0, y=0, z=-1}
	Vec3_zp = {x=0, y=0, z=1}
	
	info("startup", "Starting game")
end

function main()
	-- Main program loop.
	
	for i = 0,1,1 do
		if clientState[i] ~= nil then
			tempVec3 = {x=0, y=0, z=0}
			if clientState[i].keys.left then
				tempVec3 = vec3_crossProduct(Vec3_yp, Vec3_xp)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				orientation, error = hamiltonProduct(orientation, tempQuat)
			end
			if clientState[i].keys.right then
				tempVec3 = vec3_crossProduct(Vec3_yp, Vec3_xn)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				orientation, error = hamiltonProduct(orientation, tempQuat)
			end
			if clientState[i].keys.up then
				tempVec3 = vec3_crossProduct(Vec3_zp, Vec3_yn)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				orientation, error = hamiltonProduct(orientation, tempQuat)
			end
			if clientState[i].keys.down then
				tempVec3 = vec3_crossProduct(Vec3_zp, Vec3_yp)
				tempQuat = {w=0, x=tempVec3.x, y=tempVec3.y, z=tempVec3.z}
				tempQuat = quatNormalize(tempQuat)
				tempQuat.w = TurnRate
				tempQuat = quatNormalize(tempQuat)
				orientation, error = hamiltonProduct(orientation, tempQuat)
			end
			
		end
	end
	
	entity_setVisible(worldEntity, 1)
	entity_setVisible(cobra3Entity, 1)
	
	orientation, error = quatNormalize(orientation)
	
	rotatedVec3, error = vec3_rotate({x=0, y=0, z=1}, orientation)
	position.x = position.x + speed * rotatedVec3.x
	position.y = position.y + speed * rotatedVec3.y
	position.z = position.z + speed * rotatedVec3.z
	
	entity_setOrientation(cobra3Entity, orientation)
	entity_setPosition(cobra3Entity, position)
end

function shutdown()
	info("shutdown", "Server quit");
end
