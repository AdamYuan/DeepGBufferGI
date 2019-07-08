# Deep G-Buffer Global Illumination
An implementation of https://casual-effects.com/research/Mara2016DeepGBuffer/, using OpenGL 4.5

## Compilation
```bash
cmake . -DCMAKE_BUILD_TYPE=Release
make
```

## Built With
* [GL3W](https://github.com/skaslev/gl3w) - For modern OpenGL methods
* [GLFW](http://www.glfw.org/) - Window creation and management
* [GLM](https://glm.g-truc.net/) - Maths calculations
* [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) - Image loading
* [TinyOBJLoader](https://github.com/syoyo/tinyobjloader) - Obj loading

## Reference
* https://casual-effects.com/research/Mara2016DeepGBuffer/ - Bilateral Filter implementation
* https://github.com/Jam3/glsl-fast-gaussian-blur/ - Fast Gaussian Blur for shadow map filtering
* https://github.com/TheRealMJP/Shadows/blob/master/Shadows/MSM.hlsl - Moment Shadow Mapping implementation

## Screenshots
![](https://raw.githubusercontent.com/AdamYuan/DeepGBufferGI/master/screenshots/0.png)
![](https://raw.githubusercontent.com/AdamYuan/DeepGBufferGI/master/screenshots/1.png)
![](https://raw.githubusercontent.com/AdamYuan/DeepGBufferGI/master/screenshots/2.png)
![](https://raw.githubusercontent.com/AdamYuan/DeepGBufferGI/master/screenshots/3.png)

## Video
https://www.bilibili.com/video/av58167879

