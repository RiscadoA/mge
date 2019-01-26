# Resources

Resources are used to store read-only permanent game data.

## Types

### Empty

Empty resource.

The type value is 0x00.

### Text

Stores simple UTF-8 text.

The type value is 0x01.

### Mesh

Stores a mesh.

The type value is 0x02.

### Skeleton

Stores a skeleton.

The type value is 0x03.

### Animation

Stores a skeletal animation.

The type value is 0x04.

### Sound

Stores a sound.

The type value is 0x05.

### Streaming Sound

Stores a streaming sound (long audio, music).

The type value is 0x06.

### Material

Stores a material.

The type value is 0x07.

### Shader

Stores a shader.

The type value is 0x08.

## Hints

Hint flags:

- CPU Only (0x00000001): hints that the resource should be stored only on the CPU.
- GPU Only (0x00000002): hints that the resource should be stored only on the GPU.
- Permanent (0x00000004): hints that the resource should be loaded on startup and only unloaded on shutdown.

## Resource Manager

Is in charge of loading and unloading resources as they are needed or unneeded.

The resource manager object should be thread safe and able to load resources asynchronously.

### Usage Example

```c
#include <mge/resources/manager.h>
#include <mge/resources/text.h>

#include <mgl/stream/stream.h>

(...)
void test_load(mge_resource_manager_t* manager)
{
    // Add resource info file to manager
    mge_add_resource_info_file(manager, u8"data/info.mri");
    
    // Search for resource
    mge_resource_t* rsc = mge_find_resource(manager, u8"test_text");
    
    // Open access structure
    mge_text_resource_access_t access;
  	mge_open_resource(rsc, &access, MGE_RESOURCE_TEXT); // Increases the resource reference count and loads it if it is not loaded already
    
    // Print resource data
    mgl_print(mgl_stdout_stream, access.data->text);
    
    // Close access structure
    mge_close_resource(&access); // Decreases the resource reference count
}
(...)

```



## Storage

Resources are accessed through a MGL archive.

They are separated in two files:

- Resource Info File (.mri).
- Resource Data file (.mrd).

### Resource Info File

Stores info about one or more resources.

Format (little-endian):

```
(u32) Version;
(u32) Resource count;
for in range(0, resource count)
	(u32) Resource type;
	(u32) Resource hints;
	(u64) Resource data offset;
	(u8[64]) Resource name;
	(u8[256]) Resource data path;
	(u32) Dependency count;
	for in range(0, dependency count)
		(u8[64]) Dependency resource name;
```

### Resource Data File

Stores data of one or more resources.

Format (little-endian):

```
u8[N] data_1;
(...)
u8[M] data_n;
```

