# engine-1

![Screenshot of cubes built into structures, one of which is falling down.](/readme-assets/cubes-game.png)

Honestly, I have no idea what this is going to be yet. At this point, it's just a tech demo.
Here is what I do know:

* Written in C and moddable through Lua
* Client-server multiplayer, with optional single player
* Cross-platform (Linux + Windows 10) (Console does not currently work on Windows.)

## Installation

### Built-in libraries

Just for your information. You don't install these yourself.

```text
    SQLite 3.49.0
    Lua 5.4.8
    PhysicsFS 3.0.2
    ENet 1.3.18
    SDL 2.32.8
    GLEW 2.3.0
```

### Dependencies

Make sure you have these dependencies before building. GL and GLU are required for the client only. Qt is only required for the runner.

```text
    GL
    GLU
    Qt6
```

On linux, you will need these installed to run.  
On Windows, you will either need these installed, or you will need the DLLs in the working directory. I wish you luck in your library compilation.

### Building

```bash
cmake -B build
cmake --build build --parallel $(nproc)
```

I recommend using MSYS2 and mingw to build for Windows. It was pretty painless compared to building on Ubuntu (WSL) and Arch Linux.  
If you want to run the client or server in the background, be sure to define the preprocessor constant NOTERMINAL before compiling.  
I doubt the game will startup properly for you. There's some config files and database files that will likely need to be changed or created before the game will run.

### EVERYTHING THAT FOLLOWS IS OUT OF DATE AND IS UNLIKELY TO WORK.

### Configuration

```bash
cd ../base
# Clone the Oolite game resources. These are needed for the example.
git clone https://github.com/OoliteProject/oolite-binary-resources.git
cd ..
# Create the writable game directory.
mkdir workspace
# Start the server.
build/sengine-1
```

From the top directory on a second terminal...

```bash
# Start the client.
build/cengine-1
```

## Project status

### Currently implemented

Mod loading
Configuration scripts

### Partially implemented

Lua sandboxes  
.obj model import  
Scene graph  
Networking  


## Engine documentation

### Terminology

    model   A 3D model.
    entity  A node in the scene graph. It is used to manage collections of models and entities.
    object  A general term for a model or an entity.

### 3D graphics model

To render a model, it must first be loaded by the engine. The model is then bound to an entity. Before binding can occur, the entity must first be created. Since we are going to bind a model to it, the entity must be created as a model entity. The model can now be bound to the newly created entity. After this is done, the model entity must be linked to the world entity. The world entity is created by the engine and anything linked to it will be rendered.

More concisely, with results shown after each step:

0. Engine creates world entity before game start.  
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

Models are standard 3D models. They contain vertices and other information required for rendering. Models can be reused as many times as needed in a scene after they are loaded. They contain no position or orientation information, so they must be bound to an entity to be placed in the scene. This means that unless you have a multi-part model, you will usually want each entity to have only one model bound to it.

#### Model functions

| Function | Description |
|-|-|
| int modelIndex, int error = l_loadOoliteModel(string filePath) | Loads an Oolite model of the most recent format and returns its index in the model list. |

### Entities

An entity is a node in the entity tree. Each entity has a type, and only objects of that type can be bound to that entity. To bind a model to an entity, that model must be created with the model type. To bind an entity to another entity, the parent entity must be created with the entity type. A list of types is presented below.

#### Entity types

| Type | Description |
|-|-|
| none | Nothing can be bound to this entity. |
| entity | Entities can be bound to this entity. |
| model | Models can be bound to this entity. |
| ... | More coming soon! <sup id="a1">[1](#f1)</sup> |

The structure of entities are very general due to the variety of objects that can be bound to them, so not all members will be used in all situations.

#### Entity structure

```text
children        Array of child objects.
children_length Length of the "children" array. Can be ignored in Lua.
childType       Type of the entity.
position        3D position. x is to the right, y is up, and z is into the screen. Default is {0, 0, 0}.
orientation     Orientation quaternion. Default is {1, 0, 0, 0}.
inUse           Internal variable used to check if an entity is deleted. Can be ignored in Lua.
```

This structure only exists in the engine, but the engine provides functions to manipulate it.
Note that position and orientation are cumulative. This allows multiple models to be moved and rotated as one unit. When an entity is moved in space, all its children are moved as well since a child's reference frame is always its parent entity. The same happens when an entity is rotated. The children are rotated around the entity's origin, and the orientation of the children is rotated as well.
Something to watch out for is dangling indices and index reuse. The entity list is not an associative array. It is not a linked list. It is a normal array. The index that Lua is given is not a key, but an index to an array. The list can grow but never shrinks. When an entity is deleted, it is marked deleted, but it is not actually freed. References to a deleted entity will trigger a Lua error. When a new entity is created, it will look on the deleted entity list to see if any can be reused. If this is the case, the old deleted entity is revived with a different purpose. In this case, if the index of the deleted entity is used, the operation may be valid but will likely return an unexpected result.

#### Entity functions

| Function | Description |
|-|-|
| int entityIndex, int error = l_createEntity(int entityType) | Create an entity of type "entityType". Returns the index in the entity list.|
| int error = l_entity_linkChild(int parentEntity, int childObject) | Binds a child object to an entity if it has the appropriate type. |
| l_entity_setPosition(int entityIndex, {x, y, z} position) | Sets the position of the given entity. |
| l_entity_setOrientation(int entityIndex, {w, x, y, z} orientation) | Sets the orientation of the given entity. |

### Lua Scripting

Made for Lua 5.4, but other versions may work as well.

Games are written in Lua. Since one of the goals of this engine was to be somewhat secure, the script is sandboxed. Lua libraries cannot be used. Precompiled Lua can be run at the moment, but that will be change when I can be bothered to fix it.

#### Boilerplate

These functions are required to run a game.

```lua
function startup()
    -- Code that runs once on startup
end

function main()
    -- Main game code
    -- This function is called once per game tick. Do not put infinite loops in here.
end

function shutdown()
    -- Code that runs on exit
end
```

These functions do not have to be in `smain.lua` or `cmain.lua`, but if they are not present in those files they must be in an included file. Each one has a timeout, so don't let your code take too long to execute. If the function does time out, the game will be shut down.

#### API

This will take some time to document. Currently the API can be found in smain.c, cmain.c, and lua_common.c. Examples of how to use these functions can be found in the example code.

In the meantime, one important note: Since the standard libraries cannot be loaded, the `require` function cannot be called to include other scripts. In its place is the function `include`, which is called in about the same way.

### Key binding

Key binding is more complicated than I would have liked. Key bind functions can be found in input.c.  
How to bind a key:

1. Create a variable.
2. Set a callback for that variable (the action to be taken when the key is pressed).
3. Bind the variable to the key using the `bind` command.

This actually isn't that complicated from a user's perspective. The engine, autoexec, or gamecode should create all the bind variables and set the callbacks. The only thing the user should have to do is bind the prepared variable to the key. Here is an example.

```text
# This is done by the developer.
create none key_quit    # Create a command variable.
command key_quit quit   # Bind the command `quit` to the variable.

# This is done by the user.
bind k_27 key_quit      # Yes, this is horrible. I need to add nice names for all the keys.
```

Keys can be given both up and down actions.

```text
# Print all vars when ESC is pressed, and then quit when ESC is released.
create none +key_esc
create none -key_esc
command +key_esc vars
command -key_esc quit
bind k_27 +key_esc -key_esc
```

`+` and `-` mean nothing special. They are normal characters that are used here to easily differentiate between up and down actions.

This can also be done from Lua since it can execute config commands. The real power of this is that Lua can set a Lua function as the callback to a config variable. When the key is pressed, the Lua function is called, and then Lua can act on the key press.

A key is pressed. The key bind is found. The callback is run. If the callback is a Lua function, then that function is run. This likely sets a variable. Maybe it's just me, but this seems like way too many layers.

<b id="f1">[1](#a1)</b> Jesus is also coming soon. Expect the time frame to be about the same.
