# Rendering

## Deferred

There are at least three passes on the deferred renderer:

1. Draw all opaque objects into the G-Buffer;

2. Screenspace lighting pass (using the G-Buffer);
3. Draw all transparent objects and particles;
4. (Optional) Fullscreen effect passes;

### G-Buffer

The G-Buffer is composed of 3 textures:

- The first stores both the world position of the fragment (.xyz) and its specular value (.w).
- The second stores the world normals of the fragment (.xyz).
- The third stores the albedo of the fragment.