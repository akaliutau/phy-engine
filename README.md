![Generic badge](https://img.shields.io/badge/Build-passing-green.svg)
![Generic badge](https://img.shields.io/badge/Language-C-blue.svg)

About
======

This is an implementation of physically correct 3d render engine. I called it PhyEngine. I had been working on it quite long, starting from 2000 and till Summer of 2005. 

The project includes more than 100k lines of C-code (including roughly 30k lines of code from open-source low-level libraries for geometric calculations - polygon manipulations, tessellation, etc, - they are put into separate SE/ directory)
The biggest part of codebase is just an implementation of different algorithms which I looked up in scientific literature. The main accent was made on photon mapping and path tracing algorithms.
The latest version was quite advanced in terms of algorithms used and quality of code.

In this project I implemented a lot of innovative rendering technologies that were on the verge of CG in that time. I can name a few of them: Photon Maps, Monte-Carlo methods in solving the rendering equation (including importance sampling algorithms, both in classical Path Tracing and Bi-Directional Path Tracing), Tone Mapping, etc. Nowadays they are considered a de-facto standard in Photo-Realistic Computer Graphics.

Even though nowadays this project can be considered only a legacy one, the originality of ideas and approaches I used in this project still lasts. The code is working though - see the section Building project on VM with RH 9.0 (which can have some interest for code archaeologists and just curious personas). I have plans to develop this project in further, in order to make it the best and most accurate rendering engine in the world once again. 

It is quite interesting to observe the evolution of ideas and techniques since those times, but careful observer can notice that there are no too many changes actually. Physically correct computer graphics is based on solid fundamental principles, and you should not expect any changes there. If you want to calculate the amount of radiance received by surface from different light sources you <em>have to</em> solve equations (or set the actual experiment), there is no other way.

As for the numerical methods of solving equations, by my opinion the biggest progress one can consider the appearing of distributed versions of rendering algorithms and various light-caching techniques.

And of course, personally for me this project matters a lot. I consider it not only the first truly successful (and very challenging) one, it was also the passion project for me.
Later I published a small technical book (monograph) as a spin-off of this project.

Those seeking the <em>modern</em> example of photorealistic render engine could look up the following project: http://pbrt.org - which is based on very clear and consistent book:

2017. M. Pharr, W. Jakob, G. Humphreys Physically Based Rendering: from theory to implementation, 3rd ed. ISBN: 978-0-12-800645-0

I am not going to advertize it (it definitely does not require extra ads), but it's worth to use it as a starting point in implementation of your own version of rendering engine.
The site http://pbrt.org has general information about both the *Physically Based Rendering* book as well as many other resources for pbrt. As of October 2018, the full [text of the book](http://www.pbr-book.org) is now available online, for free.

The 3rd version of codebased can be found at https://github.com/mmp/pbrt-v3


Overview
==========

The latest version is dated by 2005. 

## Project structure

There are 18 modules in total, all modules are compiled into more or less self-contained libraries.

accelerators - containes datastructures to accelerate search in 3d spaces (kdtree)

bbox - datastructures for describing bounding boxes of objects in the scene

colors - datastructures to work with CIE/LUV color spaces and spectrums

generics - base data structures

geometry - base geometric entities

graphicutils - middle-level functions to work with draw canvas, double buffer techniques and similar stuff

imageutils - low-level support of .tiff, .ppm and .pic file formats (currently only .pic is supported - this is a RGBE HDRI format)

lighttransport - datastructures to describe interaction of light with surfaces

math - some mathematical routines

mcsampling - MC sampling generators

montecarlo - code implementing random walks in 3d spaces

photonmapping - functionality for building photon maps and using them during rendering

pools - memory management

raytracing - pathtracing functionality, brdf factories, calculation of geometric factors

tonemapping - mappers to transform HDRI into 24bits (256 colors/channel) images (original image is held in 32-float format https://en.wikipedia.org/wiki/RGBE_image_format)

## 3rd-party libraries:

MGF - low-level support of .mgf file format

PNM - low-level support of .pnm file format

SE - libraries for geometric calculations

RGBE file implementation is taken from RADIANCE project(see the details about format in documentation https://floyd.lbl.gov/radiance/refer/filefmts.pdf, section Picture File Format (.pic suffix), copy included in doc/ folder)

# Performance and quality of renderer

A typical render time for 800x600 picture depended on choice of algorithm, topological complexity of scene and of the number of samples per pixel in MC sampling. 
For performance testing I used an average for 2000s machine: CPU Pentium III 1.1 GHz, 128 MB main memory (1-2 GFLOPS)
For Bi-Directional path tracing with 64 samples/pixel the render time on aforementioned configuration was about 50 min. Pretty good, taking into account that these pictures are accurate. 

Different noise reduction techniques (such as radiance gradient interpolation) could allow to get a visually acceptable images using just 16-64 spp.

A pictures in images show the example of fully rendered image. 

images/cornell_box_glass.png  - a classic example of test scene consisting from steel and glass spheres in cornell box with 3 color walls. In this image caustics are rendered using photonmap techniques (1.2M global photons and 100k caustic photons). Main MC algorithm - Bi-Directional Path Tracing with 16 spp.

images/caustic.png  - a classic example of test scene for cardioid caustic rendered using photonmap techniques (0.5M global photons and 50k caustic photons). Main MC algorithm - Bi-Directional Path Tracing with 16 spp.


Note the high-frequency noise, especially visible on scenes with big amount of surfaces with high reflectivity. I didn't implement different approximation techniques which could smooth the granulation (and improve the visual quality of image), because they will make the picture less accurate from the physical point of view, and my aim was to create a <em> physically correct </em> renderer. 

The Materials and Geometry Format (MGF) is considered an obsolete now and is not used anymore anywhere. I included the copy of specification (see doc/mgf_specification.pdf), the original document can be found at:

https://floyd.lbl.gov/mgf/mgfdoc.pdf

Building
==========
Linux env is needed (Red Hat 9.0 specifically), OpenGL and Motif libraries, gcc 3.2.2, glibc 2.3.2. 
GUI worked just fine on XFree86 4.3.0 (I am not sure it will work on modern display server such as Xorg, but the graphic interface is just a tiny and the least important part of the whole codebase). 
There is a cli mode, and it's a preferable one, because I didn't invest too much time into UX/UI and GUI is far from perfection.

Run from root directory to build the binary file of application (phy2)

```
make clean all
```

Building project on VM with RH 9.0 (passed)
==========================================================

1. Install Red Hat 9.0 (Shrike),all packages, 32-bit version, on VirtualBox 6.1 (use PIIX4 type for cdrom device and BusLogic type for primary HDD as other settings do not work)

2. VBoxGuestAdditions does not compile on RH9 32-bit, so as a walk around one can use local ftp server running on host machine and access it from guest OS using gFTP or similar soft.

The following command instantiates docker container with ftp service:

```
docker run -d -v ./phy-engine:/home/vsftpd -p 20:20 -p 21:21 -p 47400-47470:47400-47470 -e FTP_USER=ftp -e FTP_PASS=ftp -e PASV_ADDRESS=127.0.0.1 --name ftp --restart=always bogem/ftp

Digest: sha256:cf7fe609a474c921002c3cb390b7116ce3631feb6556f95125842c1a9327bc6a
Status: Downloaded newer image for bogem/ftp:latest
cbf9e0cdba79e964dec32ee13e2edf0487cb3cb92368a5a79066e31f3662ee74

docker ps -a

CONTAINER ID   IMAGE                        COMMAND                  CREATED              STATUS                      PORTS                                                            NAMES
cbf9e0cdba79   bogem/ftp                    "/usr/sbin/run-vsftp"   About a minute ago   Up About a minute      
``` 
   By default VirtualBox maps localhost to 10.0.2.2 address, so the ftp should be visible at ftp://ftp@10.0.2.2:21 in guest machine

3. Copy sources to guest OS, compile and run as mentioned earlier.

Further plans
==============

1. Refactor and port code to 64-bit modern OS
2. Implement distributed version of algorithms


References
===========

The main articles which I used in my work are:

[1] Kajiya J. The Rendering Equation. In Computer Graphics (ACM SIGGRAPH '86 Proceedings), vol. 20, pp. 143–150, August 1986.

[2] Schlick C. An inexpensive BRDF model for physically-based rendering. In Proceedings Eurographics'94, Computer Graphics Forum, vol. 13(3), pp. 233—246, 1994.

[3] Ward G. Measuring and Modeling Anisotropic Reflection. In Computer Graphics (ACM SIGGRAPH '92 Proceedings), vol. 26, pp. 265–272, July 1992.

[4] Lafortune E. P. and Willems Y.D. Bi-directional Path Tracing. In H. P. Santo, editor, Proceedings of Third International Conference on Computational Graphics and Visualization Techniques (Compugraphics '93), pp. 145–153, Alvor, Portugal, December 1993.

[5] Veach E. and Guibas L.J. Metropolis Light Transport. In Computer Graphics (ACM SIGGRAPH '97 Proceedings), vol. 31, pp. 65–76, 1997.

[6] Metropolis N., Rosenbluth A., Rosenbluth M., Teller A., and Teller E. Equation of state calculations by fast computing machines. Journal of Chemical Physics, Vol. 21(6), pp .1087–1092, 1953.

[7] Ferwerda J., Pattanaik S., Shirley P., and Greenberg D. P. A Model of Visual Adaptation for Realistic Image Synthesis. In Computer Graphics (ACM SIGGRAPH'96 Proceedings), vol. 30, pp. 249-258.

[8] Fairchild M. D. Color Appearance Models. Addison Wesley, 1997

[9] Schlick C. Quantization Techniques for Visualization of High Dynamic Range Pictures. In Proceedings of the 5th Eurographics Workshop on Rendering, pp. 7–18, 1994.

[10] Qiu G. and Duan J. An Optimal Tone Reproduction Curve Operator for the Display of High Dynamic Range Images. Report-cvip-02-2004, February, 2004

[11] Jensen H. W. Global Illumination Using Photon Maps. In X. Pueyo and P. Scroder, editors, Rendering Techniques '96 (Proceedings of the 7th Eurographics Workshop on Rendering), pp. 21–30. Springer-Verlag, Wien New York, 1996.

[12] Bentley J. Multidimensional Binary Search Trees Used for Associative Searching. Communications of the ACM, vol.18, pp. 509-517, 1975.

[13] Jensen H. W. Importance Driven Path Tracing Using the Photon Map. In P. M. Hanrahan and W. Purgathofer, editors, Rendering Techniques '95 (Proceedings of the 6th Eurographics Workshop on Rendering), pp. 326–335. Springer-Verlag Wien New York, 1995.

[14] Cohen M.F., Wallace J.R. Radiosity and realistic image synthesis. Morgan Kaufmann, 1993. 385 p.

[15]* State of the Art in Monte Carlo Ray Tracing for Realistic Image Synthesis. Siggraph 2001 Course 29 (Monday, August 13, 2001)
https://cseweb.ucsd.edu/~viscomp/classes/cse274/wi18/readings/course29sig01.pdf

_____
* The last resource was very useful because it covered most of the topics in CG imagery synthesis.
