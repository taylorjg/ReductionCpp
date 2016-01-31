
## Description

This is my first foray into using [OpenCL](https://www.khronos.org/opencl/) from C++ (specifially, Microsoft Visual C++ 2015).

The aim is to implement the Ch10_reduction_complete example from chapter 10 of the book, [_OpenCL in Action_](https://www.manning.com/books/opencl-in-action).

## Update

This code is now working fine. However, I ran into an issue when trying to run this on CPU devices.
See [this discussion](https://software.intel.com/en-us/forums/opencl/topic/558984) for details
of the problem (race condition).

### Fix 1

To fix the race condition re reading/writing to the global `data` buffer, I changed the first kernel
to use two global buffers which I call `dataIn` and `dataOut`. Each time I invoke the first kernel,
I swap the order of the two buffers:

```C++
for (auto index = 0; true; ++index) {

    auto deviceDataIn = index % 2 == 0 ? deviceData1 : deviceData2;
    auto deviceDataOut = index % 2 == 0 ? deviceData2 : deviceData1;
    deviceResult = deviceDataOut;

    kernel1.setArg(0, deviceDataIn);
    kernel1.setArg(1, deviceDataOut);
    kernel1.setArg(2, cl::__local(localWorkSize * numValuesPerWorkItem * sizeof(float)));

    commandQueue.enqueueNDRangeKernel(
        kernel1,
        cl::NullRange,
        cl::NDRange(globalWorkSize),
        cl::NDRange(localWorkSize));

    globalWorkSize /= localWorkSize;
    if (globalWorkSize <= localWorkSize) break;
}
```

`deviceResult` is used to keep track of which of the two buffers should be
passed to the second kernel.

### Fix 2

After having resolved the race condition, I encountered another issue.
I got the following results:

* Intel(R) OpenCL / Intel(R) Core(TM) i7-4720HQ CPU @ 2.60GHz => 44040192
* Intel(R) OpenCL / Intel(R) HD Graphics 4600 => 44040192
* NVIDIA CUDA / GeForce GTX 960M => 44040192
* Experimental OpenCL 2.0 CPU Only Platform / Intel(R) Core(TM) i7-4720HQ CPU @ 2.60GHz => 5505024

When invoking the second kernel, I was passing in a `NullRange` for the `localWorkSize` parameter. On most of the devices, this resulted in a single work group whose size was equal to `globalWorkSize`. However, for the last device, this resulted in `globalWorkSize` number of work groups each with a size of 1. I fixed this by explicitly setting `localWorkSize`
to `globalWorkSize`:

```C++
commandQueue.enqueueNDRangeKernel(
    kernel2,
    cl::NullRange,
    cl::NDRange(globalWorkSize),
    cl::NDRange(globalWorkSize));
```


## Screenshot

![Screenshot](https://raw.github.com/taylorjg/ReductionCpp/master/Images/Screenshot.png)

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
