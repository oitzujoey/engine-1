g_entity_type_none = 0
g_entity_type_entity = 1
g_entity_type_model = 2

g_worldEntity = 0

g_models = {}
g_entities = {}


function common_loadModel(modelName)
	local modelPath = modelName
	local model = model_load(modelPath)
	g_models[#g_models+1] = model
end


-- Load player, box, and map collision models.
function common_loadCollisionModels()
	-- common_loadModel("player")
	boxModel, error = cmsh_load("untitled.cmsh")
	-- Load untitled.cmsh and untitled.rmsh.
	-- boxModel, error = model_load("untitled")
	puts("error: "..error)
	puts("model: "..boxModel)
	terrainModel, error = cmsh_load("blender/terrain10.cmsh")
	puts("error: "..error)
	puts("model: "..terrainModel)
end
