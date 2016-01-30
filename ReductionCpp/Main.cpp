#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <array>

using namespace std;

void runAdd(const cl::Context& context);
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
				platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

				for (auto& device : devices) {

					auto deviceName = device.getInfo<CL_DEVICE_NAME>();
					cout << "\t" << deviceName << endl;

					auto context = cl::Context(device);

					runAdd(context);
					runReduction(context);
				}
			}
			catch (const cl::Error& ex) {
				if (ex.err() != CL_DEVICE_NOT_FOUND) throw;
				cout << "No devices found for this platform!" << endl;
			}
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

void runAdd(const cl::Context& context)
{
	auto program = loadProgram(context, "add.cl");

	constexpr auto ARRAY_SIZE = 1024;

	auto h_a = array<float, ARRAY_SIZE>();
	auto h_b = array<float, ARRAY_SIZE>();
	auto h_c = array<float, ARRAY_SIZE>();

	h_a.fill(1.0f);
	h_b.fill(2.0f);

	auto d_a = cl::Buffer(context, begin(h_a), end(h_a), true);
	auto d_b = cl::Buffer(context, begin(h_b), end(h_b), true);
	auto d_c = cl::Buffer(context, begin(h_c), end(h_c), false);

	auto commandQueue = cl::CommandQueue(context, CL_QUEUE_PROFILING_ENABLE);

	auto enqueueArgs = cl::EnqueueArgs(commandQueue, cl::NDRange(ARRAY_SIZE));
	auto addFunctor = cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer>(program, "add");
	addFunctor(enqueueArgs, d_a, d_b, d_c);

	commandQueue.enqueueReadBuffer(d_c, false, 0, sizeof(float) * ARRAY_SIZE, h_c.data());

	commandQueue.finish();

	cout << h_c.front() << endl;
	cout << h_c.back() << endl;
}

void runReduction(const cl::Context& context)
{
	auto program = loadProgram(context, "reduction.cl");
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
