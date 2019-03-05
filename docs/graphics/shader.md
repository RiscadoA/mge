# Shader

Shaders are used to program the visual properties of a renderable object.

Each shader has the following properties:

- Type.
- Vertex format.
- Source code.
- List of parameters.
- Default parameter values.
- Blend state.
- Use transparency?
- Use depth testing?

## Type

### Static Mesh

Used to render static meshes, canvas elements, etc.

### Skeleton Mesh

Used to render skeletons.

Takes an uniform buffer with the bone matrices.

### Particle

Used to render particles in particle systems.

These shaders take as input, in addition to the vertexes, a vector 4 containing the particle position (.xyz) and the particle age (.w).

They also take an optional uniform integer which specifies the current particle animation frame.

## Vertex Format

The following vertex formats are supported:

- `3 Position;`
- `3 Position; 3 Normal;`
- `3 Position; 2 UV 0;`
- `2 Position; 2 UV 0;`
- `3 Position; 3 Normal; 2 UV 0;`
- `3 Position; 4 Color;`
- `3 Position; 3 Normal; 4 Color;`
- `3 Position; 4 Bone Indices; 4 Bone Weights;`
- `3 Position; 4 Bone Indices + Weights;`
- `3 Position; 3 Normal; 4 Bone Indices; 4 Bone Weights;`
- `3 Position; 3 Normal; 4 Bone Indices + Weights;`
- `3 Position; 2 UV 0; 4 Bone Indices; 4 Bone Weights;`
- `3 Position; 2 UV 0; 4 Bone Indices + Weights;`
- `3 Position; 3 Normal; 2 UV 0; 4 Bone Indices; 4 Bone Weights;`
- `3 Position; 3 Normal; 2 UV 0; 4 Bone Indices + Weights;`
- `3 Position; 4 Color; 4 Bone Indices; 4 Bone Weights;`
- `3 Position; 4 Color; 4 Bone Indices + Weights;`
- `3 Position; 3 Normal; 4 Color; 4 Bone Indices; 4 Bone Weights;`
- `3 Position; 3 Normal; 4 Color; 4 Bone Indices + Weights;`

## Example Shaders

```
version 1.0;

type = static mesh;

VERTEX
{
    input
    {
    	f32v3 position : POSITION;
    	f32v3 normal : NORMAL;
    	f32v2 uv : UV0;
    }
    
    output
    {
    	f32v3 world_normal : WORLD_NORMAL;
    	f32v3 world_position : WORLD_POS;
        f32v4 position : SCREEN_POSITION;
        f32v2 uv : UV;
    }
    
    f32m4x4 model : MODEL;
    f32m4x4 view : VIEW;
	f32m4x4 projection : PROJECTION;
    
    void main()
    {
    	output.world_position = (model * f32v4(input.position, 1.0f)).xyz;
    	output.world_normal = (model * f32v4(input.normal, 1.0f)).xyz;
        output.position = projection * view * output.world_position;
        output.uv = input.uv;
    }
}

PIXEL
{
    input
    {
    	f32v3 world_normal : WORLD_NORMAL;
    	f32v3 world_position : WORLD_POS;
        f32v2 uv : UV;
    }
    
    output
    {
        f32v4 pos_spec : GBUFFER_POSITION_SPECULAR;
        f32v4 normals : GBUFFER_NORMALS;
        f32v4 albedo : GBUFFER_ALBEDO;
    }
    
    material
	{
		f32v4 color : COLOR;
		f32 specular : SPECULAR;
        texture2d diffuse : DIFFUSE;
	}
    
    void main()
    {
        output.pos_spec.xyz = input.world_position;
        output.pos_spec.w = material.specular;
        output.normals.xyz = input.world_normal;
        output.albedo = material.color * material.diffuse.sample(input.uv);
    }
}

```

```
version 1.0;

type = skeleton mesh;
bone_matrix_count = 32; // Hint that this shader uses 32 bone matrices

VERTEX
{
    input
    {
    	f32v3 position : POSITION;
    	f32v3 normal : NORMAL;
    	f32v2 uv : UV0;
    	i32v4 bone_indices : BONE_INDICES;
    	f32v4 bone_weights : BONE_WEIGHTS;
    }
    
    output
    {
    	f32v3 world_normal : WORLD_NORMAL;
    	f32v3 world_position : WORLD_POS;
        f32v4 position : SCREEN_POSITION;
        f32v2 uv : UV;
    }
    
    f32m4x4 model : MODEL;
    f32m4x4 view : VIEW;
	f32m4x4 projection : PROJECTION;
	
	f32m4x4 bone_matrices[32] : BONE_MATRICES;
    
    void main()
    {
    	f32m4x4 bone = bone_matrices[input.bone_indices.x] * input.bone_weights.x;
    	bone *= bone_matrices[input.bone_indices.y] * input.bone_weights.y;
    	bone *= bone_matrices[input.bone_indices.z] * input.bone_weights.z;
    	bone *= bone_matrices[input.bone_indices.w] * input.bone_weights.w;
    	
    	output.world_position = (model * bone * f32v4(input.position, 1.0f)).xyz;
    	output.world_normal = (model * bone * f32v4(input.normal, 1.0f)).xyz;
        output.position = projection * view * output.world_position;
        output.uv = input.uv;
    }
}

PIXEL
{
    input
    {
    	f32v3 world_normal : WORLD_NORMAL;
    	f32v3 world_position : WORLD_POS;
        f32v2 uv : UV;
    }
    
    output
    {
        f32v4 pos_spec : GBUFFER_POSITION_SPECULAR;
        f32v4 normals : GBUFFER_NORMALS;
        f32v4 albedo : GBUFFER_ALBEDO;
    }
    
    material
	{
		f32v4 color : COLOR;
		f32 specular : SPECULAR;
        texture2d diffuse : DIFFUSE;
	}
    
    void main()
    {
        output.pos_spec.xyz = input.world_position;
        output.pos_spec.w = material.specular;
        output.normals.xyz = input.world_normal;
        output.albedo = material.color * material.diffuse.sample(input.uv);
    }
}

```

```
version 1.0;

type = static mesh;

desc = "Renders only the albedo from the GBuffer";

VERTEX
{
    input
    {
    	f32v2 position : POSITION;
    	f32v2 uv : UV0;
    }
    
    output
    {
        f32v4 position : SCREEN_POSITION;
        f32v2 uv : UV;
    }
    
    void main()
    {
        output.position = f32v4(input.position, 0.0, 1.0);
        output.uv = input.uv;
    }
}

PIXEL
{
    input
    {
        f32v2 uv : UV;
    }
    
    output
    {
        f32v4 color : TARGET0;
    }
    
    texture2d pos_spec : GBUFFER_POSITION_SPECULAR;
    texture2d normals : GBUFFER_NORMALS;
    texture2d albedo : GBUFFER_ALBEDO;
    
    void main()
    {
        output.color = f32v4(albedo.sample(input.uv).xyz, 1.0f);
    }
}
```

