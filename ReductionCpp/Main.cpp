#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#include <array>

using namespace std;

int main()
{
	try {
		auto platform = cl::Platform::get();
		auto platformName = platform.getInfo<CL_PLATFORM_NAME>();
		auto platformVendor = platform.getInfo<CL_PLATFORM_VENDOR>();
		auto platformVersion = platform.getInfo<CL_PLATFORM_VERSION>();
		cout << platformName.c_str() << endl;
		cout << platformVendor.c_str() << endl;
		cout << platformVersion.c_str() << endl;

		constexpr auto ARRAY_SIZE = 1024;

		auto h_a = array<float, ARRAY_SIZE>();
		auto h_b = array<float, ARRAY_SIZE>();
		auto h_c = array<float, ARRAY_SIZE>();

		for (auto i = 0; i < ARRAY_SIZE; ++i) {
			h_a[i] = 1.0f;
			h_b[i] = 2.0f;
			h_c[i] = 0.0f;
		}

		vector<cl::Device> devices;
		platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);

		auto context = cl::Context(devices);

		auto programFile = ifstream("add.cl");
		auto programString = string(
			istreambuf_iterator<char>(programFile),
			istreambuf_iterator<char>());
		auto sources = cl::Program::Sources(
			1,
			make_pair(programString.c_str(), programString.length() + 1));
		auto program = cl::Program(context, sources);

		program.build(devices);

		auto d_a = cl::Buffer(context, begin(h_a), end(h_a), true);
		auto d_b = cl::Buffer(context, begin(h_b), end(h_b), true);
		auto d_c = cl::Buffer(context, begin(h_c), end(h_c), false);

		auto commandQueue = cl::CommandQueue(context, CL_QUEUE_PROFILING_ENABLE);

		auto enqueueArgs = cl::EnqueueArgs(commandQueue, cl::NDRange(ARRAY_SIZE));
		auto addFunctor = cl::make_kernel<cl::Buffer, cl::Buffer, cl::Buffer>(program, "add");
		addFunctor(enqueueArgs, d_a, d_b, d_c);

		commandQueue.enqueueReadBuffer(d_c, false, 0, sizeof(float)*ARRAY_SIZE, h_c.data());

		commandQueue.finish();

		cout << h_c.front() << endl;
		cout << h_c.back() << endl;
	}
	catch(const cl::Error& err) {
		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
		exit(1);
	}

	return 0;
}
