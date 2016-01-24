#define __CL_ENABLE_EXCEPTIONS

#include <CL/cl.hpp>
#include <iostream>

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
	}
	catch(const cl::Error& err) {
		cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl;
		exit(1);
	}

	return 0;
}
