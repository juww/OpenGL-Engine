# OpenGL-GLTF
maybe later to add some of topic that i have learned in this project...


# how to get run on visual studio
1. Project tab -> Properties -> C/C++ -> General -> Additional Include Directories -> edit -> add the folder/directory include in this project
2. if some error like cannot use scanf and printf add some on Project -> Properties -> C/C++ -> Preprocessor -> add Definitions "_CRT_SECURE_NO_WARNINGS"
3. go to the Linker select Additional Library Directories -> edit -> add -> glad and glfw-lib-vc2022 (use a same version visual studio currently use)
4. go to the linker input -> select Additional Dependencies -> add all this below
    glfw3.lib
    opengl32.lib
    user32.lib
    gdi32.lib
    shell32.lib

# issue
1. make transformation matrix on model can be set in the while loop
2. some of object .gltf is render very messy and not rendered
3. in loadmodel need to add something that have many of texcoord and color
4. not apply yet about skins, joint, animation(interpolation).


# learn self
1. learning about Image Based Rendering(IBL) on Physical Based Rendering(PBR);
    1. need to learn about IBL itself, HDR, cubemap, Reflection and Refrection
2. interpolation (lerp / slerp)


# maybe next time to learn about these
1. try to apply inworld.ai in this project

2. ray tracing
3. ray marching
4. navMesh
5. QuadTree and collision
6. 
