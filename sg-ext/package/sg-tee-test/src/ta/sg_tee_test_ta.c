#include <string.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <sg_tee_test_ta.h>

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
		case TA_SG_TEE_TEST_SAY_MESSAGE:
			return print_message(param_types, params);
		default:
			return TEE_ERROR_BAD_PARAMETERS;
	}
}

