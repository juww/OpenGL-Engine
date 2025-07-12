## About
A modern OpenGL Engine written in C++. The purpose about this project, primarily a playground for me to learn and implement different graphics programming techniques or/and physics engine game.

## Library
1. [`OpenGL/GLAD`](https://glad.dav1d.de/) - Allows for drawing of graphics via the GPU
2. [`GLFW`](https://www.glfw.org/) - For creating the window and handling OS-specific operations
3. [`glm`](https://github.com/g-truc/glm) - mathematics library for OpenGL Shading Language (GLSL) specification.
4. [`ImGui`](https://github.com/ocornut/imgui) - For graphical user interfaces (GUIs) for development and debugging tools
5. [`tinyGLTF`](https://github.com/syoyo/tinygltf) - For parsing and loading glTF 3D models

## How to run
WIP, use visual studio 2022

## Current Features
1. Terrain Generation

   simple terrain generation
   ![terrain_generation_1](screenshots/terrain_generation_1.png)
2. grass on terrain
   ![grass_1](screenshots/grass.png)
3. wave ocean simulation (sine wave and FFT)

   sine waves + blind-phong lighting
   ![sine_waves](screenshots/sine_waves_water.png)

   Wave Ocean use spectrum and FFT
  ![ocean_FFT](screenshots/ocean_waves_FFT.png)
4. PBR, BRDF and IBL
   ![example_PBR](screenshots/pbr.png)
5. load gltf Model
   ![model_1](screenshots/pbr_1.png)
   ![model_2](screenshots/pbr_2.png)
   ![model_3](screenshots/sponza.png)
6. play model animation
   
   https://github.com/user-attachments/assets/a05a1b30-f211-4552-b9ee-e554d008867b
   
   https://github.com/user-attachments/assets/0733f9a5-ce67-4a6d-91fe-b7c0d26f659f

## Future Progress
1. ray tracing
2. global illumination
3. fluid simulation
4. ray marching
5. smoke and fog
6. HDR or tonemapping
7. bloom
8. cloth simulation
9. collision detection
10. smooth step for transition animation
11. procedural animation
