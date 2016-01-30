
## Description

This is my first foray into using [OpenCL](https://www.khronos.org/opencl/) from C++ (specifially, Microsoft Visual C++ 2015).

The aim is to implement the Ch10_reduction_complete example from chapter 10 of the book, [_OpenCL in Action_](https://www.manning.com/books/opencl-in-action).

## Update

This code is now working fine. However, I ran into an issue when trying to run this on CPU devices.
See [this discussion](https://software.intel.com/en-us/forums/opencl/topic/558984) for details.

## Links

* [The OpenCL C++ Wrapper API](https://www.khronos.org/registry/cl/specs/opencl-cplusplus-1.2.pdf)
* [OpenCL in Action](https://www.manning.com/books/opencl-in-action)
    * [Book's Source Code for VS2010](https://manning-content.s3.amazonaws.com/download/8/56a2ab3-4fe2-440b-8db1-bd5fa93deec6/source_code_vs2010.zip)
* [Simon McIntosh-Smith](https://www.cs.bris.ac.uk/home/simonm/)
    * _Head of the Microelectronics Group and Bristol University Business Fellow_
    * _Senior Lecturer in High Performance Computing and Architectures_
    * [OpenCL: A Hands-on Introduction](https://www.cs.bris.ac.uk/home/simonm/SC13/OpenCL_slides_SC13.pdf)
    * [COMPILING OPENCL KERNELS](http://www.cs.bris.ac.uk/home/simonm/montblanc/AdvancedOpenCL_full.pdf)
* [Problems with reduction done in CPU](https://software.intel.com/en-us/forums/opencl/topic/558984)
