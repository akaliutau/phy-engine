About
======

This is an implementation of physically correct 3d render engine. I called it PhyEngine. I had been working on it quite long, starting from 2000 and till Summer of 2005. 

The project includes more than 100k lines of C-code (including roughly 30k lines of code from open-source low-level libraries for geometric calculations - polygon manipulations, tessellation, etc, - they are put into separate SE/ directory)
The biggest part of codebase is just an implementation of different algorithms which I looked up in scientific literature. The main accent was made on photon mapping and path tracing algorithms.
The latest version was quite advanced in terms of algorithms used and quality of code.

Overview
==========

The latest version is dated by 2005. 

# Performance and quality of renderer

A typical render time for 800x600 picture depended on choice of algorithm, topological complexity of scene and of the number of samples per pixel in MC sampling. 
For performance testing I used an average for 2000s machine: CPU Pentium III 1.1 GHz, 128 MB main memory (1-2 GFLOPS)
For Bi-Directional path tracing with 64 samples/pixel the render time on aforementioned configuration was about 50 min. Pretty good, taking into account that these pictures are accurate. 

Different noise reduction techniques (such as radiance gradient interpolation) could allow to get a visually acceptable images using just 16-64 spp.

A picture in images/cornell_box_glass.png shows the example of fully rendered image. Note the high-frequency noise, especially visible on scenes with big amount of surfaces with high reflectivity. I didn't implement different approximation techniques which could smooth the granulation (and improve the visual quality of image), because they will make the picture less accurate from the physical point of view, and my aim was to create a <em> physically correct </em> renderer. 

The Materials and Geometry Format (MGF) is considered an obsolete now and is not used anymore anywhere. I included the copy of specification (see doc/mgf_specification.pdf), the original document can be found at:

https://floyd.lbl.gov/mgf/mgfdoc.pdf

Building
==========
Linux env is needed (I didn't have plans to port project to any other OS, and used machine with Red Hat Linux 9.0), OpenGL and Motif libraries, gcc 3.2.2, glibc 2.3.2. 
GUI worked just fine on XFree86 4.3.0 (I am not sure it will work on modern display server such as Xorg, but the graphic interface is just a tiny and the least important part of the whole codebase). 
There is a cli mode, and it's a preferable one, because I didn't invest too much time into UX/UI and GUI is far from perfection.

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


_____
* The last resource was very useful because it covered most of the topics in CG imagery synthesis.
