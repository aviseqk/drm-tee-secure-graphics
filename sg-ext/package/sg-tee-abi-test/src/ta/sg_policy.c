#include "include/sg_request_response.h"
#include <sg_policy.h>


TEE_Result sg_policy_validate(uint32_t *decision, uint32_t *reason)
{
	if (decision == NULL || reason == NULL)
		return TEE_ERROR_BAD_PARAMETERS;

	// TODO: replace this placeholder static return of success case to actual decision policy strategy
	*decision = (uint32_t)SG_DECISION_ALLOW;
	*reason = (uint32_t)SG_REASON_POLICY_ALLOWED;
	
	// fake placeholder ALLOW Atomic Commit Transaction
	//*decision = (uint32_t)SG_DECISION_DENY;
	//*reason = (uint32_t)SG_REASON_INVALID_METADATA;

	return TEE_SUCCESS;
}
