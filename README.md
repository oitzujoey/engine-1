# engine-1

Honestly, I have no idea what this is going to be yet.
Here is what I do know:
    Written in C
    Cross-platform (Linux + Windows 10) (Currently only runs on Linux)
    Client-server
    Moddable through Lua

## Installation

### Dependencies

### Building

Make sure you have these dependencies. GL and GLEW are required for the client only.

    SDL2
    Lua
    ENet
    GL
    GLEW

```bash
$ mkdir build && cd build
$ cmake ..
$ make
```

## Project status

### Currently implemented

Turing complete scripting (for configuration)

### Partially implemented

Lua sandboxes  
Model loader  
Model list  
Entity tree  
Networking  
Rendering  

## Engine documentation

### Terminology

    model   A standard 3D model. May include other types of models in the future.
    entity  A node in the entity tree. It stores information used for manipulating models during rendering.
    object  A general term for a model or an entity.

### 3D graphics model

The rendering system is a bit complicated, but hopefully not too difficult to understand.
To render a model, it must first be loaded by the engine. This is done manually to save memory and loading time. The model is then bound to an entity. I expect that in most cases, only one model will be bound to a given entity for reasons that should become obvious shortly. Before binding can occur, the entity must first be created. Since we are going to bind a model to it, the entity must be created as a model entity. The model can now be bound to the newly created entity. After this is done, the model entity must be linked to the world entity. The world entity is created by the engine and anything linked to it will be rendered.

More concisely, with results shown after each step:

0.  Engine creates world entity before game start.  
    &nbsp;&nbsp;&nbsp;&nbsp;worldEntity  
1. Load model.  
    &nbsp;&nbsp;&nbsp;&nbsp;worldEntity  
    &nbsp;&nbsp;&nbsp;&nbsp;model  
2. Create entity. (type is model)  
    &nbsp;&nbsp;&nbsp;&nbsp;worldEntity  
    &nbsp;&nbsp;&nbsp;&nbsp;model  
    &nbsp;&nbsp;&nbsp;&nbsp;entity  
3. Bind model to entity.  
    &nbsp;&nbsp;&nbsp;&nbsp;worldEntity  
    &nbsp;&nbsp;&nbsp;&nbsp;entity  
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;model  
4. Bind entity to world entity.  
    &nbsp;&nbsp;&nbsp;&nbsp;worldEntity  
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;entity  
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;model  
5. Render worldEntity.  

The reason for this structure is to make manipulating models easier, as is explained below.

### Models

Models are standard 3D models. They contain vertices and other information required for rendering. Models can be reused as many times as needed in a scene after they are loaded. They contain no position or orientation information, so they must be bound to an entity to be placed in the scene.

#### Model functions

| Function | Description |
|-|-|
| int modelIndex, int error = l_loadOoliteModel(string filePath) | Loads an Oolite model of the most recent format and returns its index in the model list. |

### Entities

An entity is a node in the entity tree. Each entity has a type, and only objects of that type can be bound to an entity. This was touched on a bit already. To bind a model to an entity, that model must be created with the model type. To bind an entity to another entity, the parent entity must be created with the entity type. A list of types is presented below.

#### Entity types

| Type | Description |
|-|-|
| none | Nothing can be bound to this entity. |
| entity | Entities can be bound to this entity. |
| model | Models can be bound to this entity. |
| ... | More coming soon! [^1] |

The structure of an entities are very general due to the variety of objects that can be bound to them, so not all elements will be used in all situations.

#### Entity structure

    children        Array of child objects.
    children_length Length of the "children" array. Can be ignored in Lua.
    childType       Type of the entity.
    position        3D position. x is to the right, y is up, and z is into the screen. Default is {0, 0, 0}.
    orientation     Orientation quaternion. Default is {1, 0, 0, 0}.
    inUse           Internal variable used to check if an entity is deleted. Can be ignored in Lua.

This structure only exists in the engine, but the engine provides functions to manipulate it.
Note that position and orientation are cumulative. This allows multiple models to be moved and rotated as one unit. When an entity is moved in space, all its children are moved as well since a child's reference frame is always its parent entity. The same happens when an entity is rotated. The children are rotated around the entity's origin, and the orientation of the children is rotated as well.

#### Entity functions

| Function | Description |
|-|-|
| int entityIndex, int error = l_createEntity(int entityType) | Create an entity of type "entityType". Returns the index in the entity list.|
| int error = l_entity_linkChild(int parentEntity, int childObject) | Binds a child object to an entity if it has the appropriate type. |
| l_entity_setPosition(int entityIndex, {x, y, z} position) | Sets the position of the given entity. |
| l_entity_setOrientation(int entityIndex, {w, x, y, z} orientation) | Sets the orientation of the given entity. |

[^1]: Jesus is also coming soon. Expect the time frame to be about the same.
