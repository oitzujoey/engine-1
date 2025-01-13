g_entity_type_none = 0
g_entity_type_entity = 1
g_entity_type_renderModel = 2
g_entity_type_collisionModel = 3

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
	common_loadModel("box")
	-- common_loadModel("map")
end
