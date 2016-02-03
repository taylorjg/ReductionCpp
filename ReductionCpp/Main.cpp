#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <memory>

using namespace std;

void runReduction(const cl::Context& context);
cl::Program loadProgram(const cl::Context& context, const string& fileName);

int main()
{
    try {
        vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);

        for (auto& platform : platforms) {

            auto platformName = platform.getInfo<CL_PLATFORM_NAME>();
            auto platformVendor = platform.getInfo<CL_PLATFORM_VENDOR>();
            auto platformVersion = platform.getInfo<CL_PLATFORM_VERSION>();

            cout << platformName << endl;
            cout << platformVendor << endl;
            cout << platformVersion << endl;

            try {
                vector<cl::Device> devices;
                platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

                for (auto& device : devices) {
                    auto deviceName = device.getInfo<CL_DEVICE_NAME>();
                    cout << "\t" << deviceName << endl;
                    auto context = cl::Context(device);
                    runReduction(context);
                }
            }
            catch (const cl::Error& ex) {
                if (ex.err() != CL_DEVICE_NOT_FOUND) throw;
                cout << "No devices found for this platform!" << endl;
            }

            cout << endl;
        }
    }
    catch(const cl::Error& ex) {
        cerr << "ERROR: " << ex.what() << " (" << ex.err() << ")" << endl;
        exit(1);
    }
    catch (const ios_base::failure& ex) {
        cerr << ex.what() << endl;
        exit(1);
    }

    return 0;
}

void runReduction(const cl::Context& context)
{
    auto program = loadProgram(context, "reduction.cl");
    auto kernel1 = cl::Kernel(program, "reductionVector");
    auto kernel2 = cl::Kernel(program, "reductionComplete");

    constexpr auto numValues = 1024 * 1024;
    constexpr auto numValuesPerWorkItem = 4;
    constexpr auto initialGlobalWorkSize = numValues / numValuesPerWorkItem;
    auto globalWorkSize = initialGlobalWorkSize;
    constexpr auto localWorkSize = 32;
    constexpr auto value = 42;
    constexpr auto correctAnswer = 42 * numValues;
    constexpr auto initialNumWorkGroups = initialGlobalWorkSize / localWorkSize;

    constexpr auto hostData1Size = numValues;
    constexpr auto hostData2Size = initialNumWorkGroups * numValuesPerWorkItem;

    auto hostData1(make_unique<float[]>(hostData1Size));
    for (auto i = 0; i < numValues; ++i) hostData1[i] = 42.0f;

    auto deviceData1 = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, hostData1Size * sizeof(float), hostData1.get());
    auto deviceData2 = cl::Buffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, hostData2Size * sizeof(float), nullptr);
    auto deviceResult = deviceData2;
    auto deviceSum = cl::Buffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR, sizeof(float), nullptr);

    auto commandQueue = cl::CommandQueue(context);

    for (auto index = 0; true; ++index) {

        auto deviceDataIn = index % 2 == 0 ? deviceData1 : deviceData2;
        auto deviceDataOut = index % 2 == 0 ? deviceData2 : deviceData1;
        deviceResult = deviceDataOut;

        kernel1.setArg(0, deviceDataIn);
        kernel1.setArg(1, deviceDataOut);
        kernel1.setArg(2, cl::__local(localWorkSize * numValuesPerWorkItem * sizeof(float)));

        cout
            << "Calling enqueueNDRangeKernel(kernel1) with globalWorkSize: " << globalWorkSize
            << "; localWorkSize: " << localWorkSize
            << "; num work groups: " << globalWorkSize / localWorkSize
            << endl;

        commandQueue.enqueueNDRangeKernel(
            kernel1,
            cl::NullRange,
            cl::NDRange(globalWorkSize),
            cl::NDRange(localWorkSize));

        globalWorkSize /= localWorkSize;
        if (globalWorkSize <= localWorkSize) break;
    }

    kernel2.setArg(0, deviceResult);
    kernel2.setArg(1, cl::__local(localWorkSize * numValuesPerWorkItem * sizeof(float)));
    kernel2.setArg(2, deviceSum);

    cout
        << "Calling enqueueNDRangeKernel(kernel2) with globalWorkSize: " << globalWorkSize
        << "; localWorkSize: " << globalWorkSize
        << "; num work groups: " << globalWorkSize / globalWorkSize
        << endl;

    commandQueue.enqueueNDRangeKernel(
        kernel2,
        cl::NullRange,
        cl::NDRange(globalWorkSize),
        cl::NDRange(globalWorkSize));

    auto sum = 0.0f;
    commandQueue.enqueueReadBuffer(deviceSum, CL_TRUE, 0, sizeof(float), &sum);

    auto finalAnswer = static_cast<int>(sum);
    cout << "OpenCL final answer: " << finalAnswer << "; Correct answer: " << correctAnswer << endl;
}

cl::Program loadProgram(const cl::Context& context, const string& fileName)
{
    auto devices = context.getInfo<CL_CONTEXT_DEVICES>();
    auto device = devices.front();

    auto programFile = ifstream(fileName);
    programFile.exceptions(ifstream::failbit);
    auto programString = string(
        istreambuf_iterator<char>(programFile),
        istreambuf_iterator<char>());
    auto sources = cl::Program::Sources(
        1,
        make_pair(programString.c_str(), programString.length() + 1));
    auto program = cl::Program(context, sources);

    try {
        program.build(devices);
    }
    catch (const cl::Error& ex) {
        if (ex.err() == CL_BUILD_PROGRAM_FAILURE) {
            auto buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
            cerr << "Error building " << fileName << endl;
            cerr << buildLog << endl;
        }
        throw;
    }

    return program;
}
