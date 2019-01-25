# Scene

The scene graph contains data about the game scene. It is stored as a tree of scene nodes.

## Scene Node

A scene node has:

- A parent node (the root node has a null parent node).
- Optionally one or more children.
- Optionally one or more components.
- A transform.

## Component

A component is used to gives action to a scene node.

There are the following types of components:

- Static Mesh - Renders a static mesh.
- Skeleton Mesh - Renders a skeleton mesh.
- Particle System - Renders a particle system.
- Skeleton Animator - Animates a set of scene nodes as if it was a skeleton (IK, animation playing, etc).
- Canvas - Acts as a 2D renderable surface (children scene node components are rendered into this canvas).
- Canvas Element - Every canvas child element must have this component.
- Behaviour - Where game-specific logic is implemented.
- 2D Image - Renders an image into a canvas.
- 2D Text - Renders text into a canvas.
- Box Collider.
- Sphere Collider.
- Capsule Collider.
- Mesh Collider.
- Rigidbody - Adds physics to a scene node.
- Camera - Defines a view.
- VR Camera - Defines a VR view (HMD view).

## Initialization

On engine startup (after all subsystems are initialized) the `void mge_game_load(void)` function (which is implemented in the game code) is called and it is in charge of initializing the scene.

On engine termination `void mge_game_unload(void)` is called.