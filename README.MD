# Fractal Engine
Developed during the UPC Master's Degree in Video Game Design and Development. 
Currently under development.

![Fractal gif example](ReadmeGifExample.gif)

## Windows
### Scene & Game
Scene camera will render the geometry loaded allowing user to select them with the mouse click.
In order to move the camera in the window:
* **WASD** fps-like movement while **Right-clicking** to free look around 
* **Mouse wheel** to zoom in and out
* **Alt+Left** click for orbiting the selcted model 
* **F** to focus the camera on selected model
* Hold **shift** to duplicate camera movement

Fractal Engine uses ImGuizmo, which allows users to move selected GameObjects using a little gizmo centered in his location. 
GameObject transformations could be edited by clicking on the different buttons on screen (Position, rotation and scale). Also, the *snap* button allows to transform an object using a regular scale customizable.
The engine allows changing the transformation mode from world space to local space in order to ease the placement of GameObjects in the scene.

The game window will render the main camera view without debug gizmos and the interactivity of the editor. If no main camera is found it will render a "not camera found" texture in it.

### Hierarchy
The hierarchy allows to visualize the structure of the different GameObjects presents in the scene. In this window you can select a GameObject to view its information in the inspector. **Right-clicking** in a GameObject will provide you multiple operations like: creating a primitive, a light, deleting the selected object or duplicating it.
Be aware that a primitive is created according to the current scale of the scene!!
You can also drag and drop GameObjects to other GameObjects to change the structure of the scene and make them inherit the properties.

### Inspector
The inspector shows information about the selected Game Object and it's components. You can edit, add or remove components in each GameObject in order to build your own game behaviour.

### Configuration
Here you have different sections that allows you modify different visualization options. Be aware that using some options may affect your computer performance!! 
The current configuration is saved when the engine exits.

### Console
This will print any relevant log about importing, loading or saving files.

## More information
C++ coding style,  Guide step by step, etc.
[Fractal Wiki](https://github.com/fractal-puppy/Engine/wiki)

Changelog with new features and fixes
[last Release Notes](https://github.com/fractal-puppy/Engine/releases).

[Github](https://github.com/fractal-puppy/Engine) and
[License](https://github.com/fractal-puppy/Engine/blob/master/LICENSE)
