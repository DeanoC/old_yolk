#pragma OPENCL EXTENSION cl_amd_printf : enable

kernel void helloworld() {
	printf( "Hello, world from OpenCL on " );
#ifdef __Cayman__
	printf("Device Name: Cayman\n");
#elif __Barts__
	printf("Device Name: Barts\n");
#elif __Cypress__
	printf("Device Name: Cypress\n");
#elif defined(__Juniper__)
	printf("Device Name: Juniper\n");
#elif defined(__Redwood__)
	printf("Device Name: Redwood\n");
#elif defined(__Cedar__)
	printf("Device Name: Cedar\n");
#elif defined(__ATI_RV770__)
	printf("Device Name: RV770\n");
#elif defined(__ATI_RV730__)
	printf("Device Name: RV730\n");
#elif defined(__ATI_RV710__)
	printf("Device Name: RV710\n");
#elif defined(__Loveland__)
	printf("Device Name: Loveland\n");
#elif defined(__GPU__)
	printf("Device Name: GenericGPU\n");
#elif defined(__X86__)
	printf("Device Name: X86CPU\n");
#elif defined(__X86_64__)
	printf("Device Name: X86-64CPU\n");
#elif defined(__CPU__)
	printf("Device Name: GenericCPU\n");
#else
	printf("Device Name: UnknownDevice\n");
#endif
}