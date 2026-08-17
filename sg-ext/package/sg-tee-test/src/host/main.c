#include <err.h>
#include <stdio.h>
#include <string.h>

#include <tee_client_api.h>	// op-tee tee client api, built by optee-client

#include <sg_tee_test_ta.h>	// for the uuid, as found in the TA's header file

int main(void)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_SharedMemory shm;
	TEEC_UUID uuid = TA_SG_TEE_TEST_UUID;
	uint32_t err_origin;

	// connect to TEE by initializing a Context
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

	// open a session with "sg-tee-test" TA, and if it succeeds, it, the TA
	// will print "Welcome REE!".
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS)
		errx(1, "TEEC_OpenSession failed with code 0x%x origin 0x%x",
				res, err_origin);

	// execute a function in TA by invoking it
	

	// Allocate Shared Memory
	shm.size = 1024;
	shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

	res = TEEC_AllocateSharedMemory(&ctx, &shm);
	if (res != TEEC_SUCCESS) {
		printf("Shared Memory Allocation Failed: 0x%x\n", res);
		goto cleanup_session;
	}

	// write message into the shared memory
	snprintf((char *)shm.buffer, shm.size, "Hello from REE via Shared Memory");

	// first clear the TEEC_Operation struct, and prepare it using the registered shared memory reference
	memset(&op, 0, sizeof(op));
	op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_MEMREF_PARTIAL_INOUT, 
			TEEC_NONE, 
			TEEC_NONE, 
			TEEC_NONE);

	op.params[0].memref.parent = &shm;
	op.params[0].memref.offset = 0;
	op.params[0].memref.size = shm.size;

	// the function being called in the TA is called "TA_SG_TEE_TEST_SAY_MESSAGE"
	printf("Invoking TA and sending a message - %s\n", (char *)shm.buffer);

	res = TEEC_InvokeCommand(&sess, TA_SG_TEE_TEST_SAY_MESSAGE, &op, &err_origin);
	if ( res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);

	printf("TA modified the buffer as: %s\n", (char *)shm.buffer);

	 // free Shared Memory
	TEEC_ReleaseSharedMemory(&shm);

cleanup_session:
	// done with TA, hence closing the session and closing the context
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
	return 0;
}
