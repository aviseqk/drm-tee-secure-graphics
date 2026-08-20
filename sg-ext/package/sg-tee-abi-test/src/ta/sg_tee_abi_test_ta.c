#include <tee_api.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <sg_tee_abi_test_ta.h>
#include <sg_drm_metadata.h>
#include <sg_request_response.h>
#include <sg_policy.h>

// called when the instance of TA is created, first call in the TA
TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

// called when the instance of TA is destroyed, last call in the TA
void TA_DestroyEntryPoint(void){}


// called when a new session is opened to the TA
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __unused params[4],
		void __unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
			TEE_PARAM_TYPE_NONE,
			TEE_PARAM_TYPE_NONE,
			TEE_PARAM_TYPE_NONE);
	
	IMSG("Session Opened: Welcome REE!\n");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	return TEE_SUCCESS;
}

static TEE_Result print_message(uint32_t param_types, TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT, 
			TEE_PARAM_TYPE_NONE, 
			TEE_PARAM_TYPE_NONE, 
			TEE_PARAM_TYPE_NONE); 

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	char *buffer = (char *)params[0].memref.buffer;
	size_t len = params[0].memref.size;

	IMSG("REE sent a message : %.*s", (int)len, buffer);

	if (params[0].memref.size < len)
		return TEE_ERROR_SHORT_BUFFER;

	snprintf(buffer, len, "Hello back from Secure World!");

	IMSG("Replying to REE as: %.*s", (int)len, buffer);
	return TEE_SUCCESS;

}

static TEE_Result sg_validate_transaction(uint32_t param_types, TEE_Param params[4])
{
	DMSG("has been called");
	TEE_Result res;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT, 
			TEE_PARAM_TYPE_NONE, 
			TEE_PARAM_TYPE_NONE, 
			TEE_PARAM_TYPE_NONE);

	if (param_types  != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	struct sg_validate_request request;
	memset(&request, 0, sizeof(request));

	struct sg_validate_response response;
	memset(&response, 0, sizeof(response));

	struct sg_reader reader = {
		.buffer = (uint8_t *)params[0].memref.buffer,
		.size = params[0].memref.size,
		.offset = 0,
	};

	DMSG("deserializing request.header");
	res = sg_deserialize_header(&reader, &request.header);
	if (res != TEE_SUCCESS)
		return res;

	DMSG("deserializing request.metadata");
	res = sg_deserialize_transaction(&reader, &request.metadata);
	if (res != TEE_SUCCESS)
		return res;

	if (reader.offset != request.header.size) {
		IMSG("declared size:%zu does not match actual deserialized metadata size: %u\n",
				reader.offset, request.header.size);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	IMSG("Post Request Deserialization: buffer->size: %zu buffer->offset: %zu",
			reader.size, reader.offset);

	print_sg_validate_request(&request);
	if (res != TEE_SUCCESS)
		IMSG("sg_deserialize_header() failed\n");

	// DECIDE
	response.version = SG_ABI_VERSION;
	res = sg_policy_validate(&response.decision, &response.reason); 
	if (res != TEE_SUCCESS) {
		IMSG("sg_policy_validate() failed\n");
		return res;
	}

	struct sg_writer writer = {
		.buffer = (uint8_t *)params[0].memref.buffer,
		.size = params[0].memref.size,
		.offset = request.header.response_offset,
	};

	res = sg_serialize_response(&writer, &response);
	if (res != TEE_SUCCESS){
		DMSG("failed in sg_serialize_response()\n");
		return res;
	}

	print_serialized_bytes(writer.buffer, 
			request.header.response_size + request.header.response_offset, 0);

	return TEE_SUCCESS;
}

// called when a session is closed
void TA_CloseSessionEntryPoint(void __unused *sess_ctx)
{
	IMSG("Session Closed: Goodbye!\n");
}

// called when a TA is invoked
TEE_Result TA_InvokeCommandEntryPoint(void __unused *sess_ctx,
		uint32_t cmd_id, uint32_t param_types,
		TEE_Param params[4])
{
	switch (cmd_id) {
		case TA_SG_TEE_ABI_TEST_SAY_MESSAGE:
			return print_message(param_types, params);
		case SG_CMD_VALIDATE_TRANSACTION:
			return sg_validate_transaction(param_types, params);
		default:
			return TEE_ERROR_BAD_PARAMETERS;
	}
}




