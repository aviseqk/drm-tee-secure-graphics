#ifndef SG_POLICY_H
#define SG_POLICY_H

#include <tee_internal_api.h>
#include <sg_request_response.h>

TEE_Result sg_policy_validate(uint32_t *decision, uint32_t *reason);

#endif
