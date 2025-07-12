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
![Example Image](screenshots/terrain_generation_1.png)
2. grass on terrain
3. wave ocean simulation (sine wave and FFT)
4. load gltf Model
5. PBR, BRDF and IBL
6. play model animation

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