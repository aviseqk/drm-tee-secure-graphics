#include <err.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <tee_client_api.h>	// op-tee tee client api, built by optee-client

#include <sg_tee_abi_test_ta.h>	// for the uuid, as found in the TA's header file

#include <sg_request_response.h>
#include <sg_drm_metadata.h>
#include <sg_serialization.h>


// TODO: similarly we can add all various test cases, and then make this into a test suite for both TEE REE ABI as well as different policy scenarios
void generate_fake_drm_metadata(struct sg_transaction_metadata *metadata)
{
    *metadata = (struct sg_transaction_metadata) {
        .version = SG_METADATA_VERSION,
        .num_connectors = 1,
        .num_crtcs = 1,
        .num_planes = 1,

        .crtcs = {
            [0] = {
                .crtc_id = 30,
                .active = 1,
                .enable = 1,
                .mode_height = 1920,
                .mode_width = 1080,
                .refresh_millihz = 59480,
            },
        },

        .connectors = {
            [0] = {
                .crtc_id = 30,
                .connector_id = 29,
            },
        },

        .planes = {
            [0] = {
                .crtc_id = 30,
                .num_fb_planes = 1,
                .plane_id = 40,
                .buffers = {
                    [0] = {
                        .security_class = SG_BUFFER_NORMAL,
                    },
                },
                .dst_height = 1920,
                .dst_width = 1080,
                .dst_x = 0,
                .dst_y = 0,
                .src_x = 0,
                .src_y = 0,
                .src_width = 1080,
                .src_height = 1920,
                .fb_height = 400,
                .fb_width = 200,
                .fb_id = 44,
                .format = 4,
                .modifier = 0,
            },
        },
    };
}

int main(void)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_SharedMemory shm;
	TEEC_UUID uuid = TA_SG_TEE_ABI_TEST_UUID;
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
	shm.size = 2048;
	shm.flags = TEEC_MEM_INPUT | TEEC_MEM_OUTPUT;

	res = TEEC_AllocateSharedMemory(&ctx, &shm);
	if (res != TEEC_SUCCESS) {
		printf("Shared Memory Allocation Failed: 0x%x\n", res);
		goto cleanup_session;
	}

	uint8_t *buf = shm.buffer;
	size_t offset;

	struct sg_request_header header;
	struct sg_transaction_metadata metadata;
	struct sg_validate_request request;

	generate_fake_drm_metadata(&metadata);

	request.metadata = metadata;

	offset = sizeof(struct sg_request_header);
	printf("proposed reserving offset: %zu\n", offset);
	sg_serialize_transaction(buf, &offset, &request.metadata);
    
	generate_sg_request_header(&header, offset);
	request.header = header;

	offset = 0;

	sg_serialize_header(buf, &offset, &request.header);

	print_serialized_bytes(buf, header.size, header.response_offset);

	printf("\n\n");

	//print_sg_request_header(&header);


	// write message into the shared memory
	//snprintf((char *)shm.buffer, shm.size, "Hello from REE via Shared Memory");

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

	//res = TEEC_InvokeCommand(&sess, TA_SG_TEE_ABI_TEST_SAY_MESSAGE, &op, &err_origin);
	res = TEEC_InvokeCommand(&sess, SG_CMD_VALIDATE_TRANSACTION, &op, &err_origin);
	if ( res != TEEC_SUCCESS)
		errx(1, "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
				res, err_origin);

	printf("TA modified the buffer as: %s\n", (char *)shm.buffer);

	printf("POST TA RESPONSE: The buffer is:\n");
	print_serialized_bytes(op.params[0].memref.parent->buffer,
		header.response_offset + header.response_size, 0);

	struct sg_validate_response response;

	int ret = sg_deserialize_response((uint8_t *)shm.buffer, header.response_offset, header.response_size,&response);
    

	printf("\nTHE RESPONSE IS DESERIALIZED\n");
	print_sg_response(&response);

	
	// free Shared Memory
	TEEC_ReleaseSharedMemory(&shm);

cleanup_session:
	// done with TA, hence closing the session and closing the context
	TEEC_CloseSession(&sess);
	TEEC_FinalizeContext(&ctx);
	return 0;
}


