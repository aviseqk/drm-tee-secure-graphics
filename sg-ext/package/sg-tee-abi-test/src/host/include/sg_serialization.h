#ifndef SG_SERIALIZATION_H
#define SG_SERIALIZATION_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>

#include <tee_client_api.h>	// op-tee tee client api, built by optee-client
#include <sg_tee_abi_test_ta.h>	// for the function IDs, as found in the TA's header file

void sg_serialize_header(uint8_t *buf, size_t *offset,
	const struct sg_request_header *header);

void sg_serialize_transaction(uint8_t *buf, size_t *offset,
	const struct sg_transaction_metadata *metadata);

int sg_deserialize_response(uint8_t *buffer, size_t offset, size_t size, 
		struct sg_validate_response *response);

void generate_sg_request_header(struct sg_request_header *header, uint32_t size);

void print_serialized_bytes(uint8_t *buf, uint32_t req_size, uint32_t offset);

void print_sg_response(struct sg_validate_response *resp);
#endif
