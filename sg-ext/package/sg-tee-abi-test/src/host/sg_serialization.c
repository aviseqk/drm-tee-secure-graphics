#include "sg_request_response.h"
#include "sg_serialization.h"

/* Deserialization Helper */
static void sg_read_u32(uint8_t *buf, size_t *offset, size_t size, uint32_t *value)
{
    // TODO: add size related checks here please post being passed from sg_deserialize_response
    uint32_t v;
    memcpy(&v, buf + *offset, sizeof(v));
    *offset += sizeof(v);

    *value = le32toh(v);
}

int sg_deserialize_response(uint8_t *buffer, size_t offset, size_t size, 
		struct sg_validate_response *response)
{
    sg_read_u32(buffer, &offset, size, &response->version);
    sg_read_u32(buffer, &offset, size, &response->decision);
    sg_read_u32(buffer, &offset, size, &response->reason);

    return 0;
}

/* Serialization Helpers */
static void sg_write_u32(uint8_t *buf, size_t *offset, uint32_t value)
{
    uint32_t v = htole32(value);
    memcpy(buf + *offset, &v, sizeof(v));
    *offset += sizeof(v);
}

static void sg_write_u64(uint8_t *buf, size_t *offset, uint64_t value)
{
    uint64_t v = htole64(value);
    memcpy(buf + *offset, &v, sizeof(v));
    *offset += sizeof(v);
}

static void sg_write_s32(uint8_t *buf, size_t *offset, int32_t value)
{
    int32_t v = htole32(value);
    memcpy(buf + *offset, &v, sizeof(v));
    *offset += sizeof(v);
}

static void sg_serialize_crtc(uint8_t *buf, size_t *offset,
	const struct sg_crtc_metadata *crtc)
{
    sg_write_u32(buf, offset, crtc->crtc_id);
    sg_write_u32(buf, offset, crtc->enable);
    sg_write_u32(buf, offset, crtc->active);
    sg_write_u32(buf, offset, crtc->mode_width);
    sg_write_u32(buf, offset, crtc->mode_height);
    sg_write_u32(buf, offset, crtc->refresh_millihz);
}

static void sg_serialize_connector(uint8_t *buf, size_t *offset,
	const struct sg_connector_metadata *connector)
{
    sg_write_u32(buf, offset, connector->connector_id);
    sg_write_u32(buf, offset, connector->crtc_id);
}

static void sg_serialize_buffer(uint8_t *buf, size_t *offset,
	const struct sg_buffer_metadata *buffer)
{
    sg_write_u32(buf, offset, buffer->security_class);
}

static void sg_serialize_plane(uint8_t *buf, size_t *offset,
	const struct sg_plane_metadata *plane)
{
    sg_write_u32(buf, offset, plane->plane_id);
    sg_write_u32(buf, offset, plane->crtc_id);
    sg_write_u32(buf, offset, plane->fb_id);
    sg_write_u32(buf, offset, plane->fb_width);
    sg_write_u32(buf, offset, plane->fb_height);
    sg_write_u32(buf, offset, plane->format);

    sg_write_u64(buf, offset, plane->modifier);

    sg_write_u32(buf, offset, plane->src_x);
    sg_write_u32(buf, offset, plane->src_y);
    sg_write_u32(buf, offset, plane->src_width);
    sg_write_u32(buf, offset, plane->src_height);

    sg_write_s32(buf, offset, plane->dst_x);
    sg_write_s32(buf, offset, plane->dst_y);

    sg_write_u32(buf, offset, plane->dst_width);
    sg_write_u32(buf, offset, plane->dst_height);
    sg_write_u32(buf, offset, plane->num_fb_planes);

    for (uint32_t i = 0; i < plane->num_fb_planes; i++) {
	sg_serialize_buffer(buf, offset, &plane->buffers[i]);
    }

}

void sg_serialize_header(uint8_t *buf, size_t *offset,
	const struct sg_request_header *header)
{
    sg_write_u32(buf, offset, header->version);
    sg_write_u32(buf, offset, header->command);
    sg_write_u32(buf, offset, header->size);
    sg_write_u32(buf, offset, header->response_offset);
    sg_write_u32(buf, offset, header->response_size);
}

void sg_serialize_transaction(uint8_t *buf, size_t *offset,
	const struct sg_transaction_metadata *metadata)
{
    sg_write_u32(buf, offset, metadata->version);


    sg_write_u32(buf, offset, metadata->num_crtcs);
    sg_write_u32(buf, offset, metadata->num_connectors);
    sg_write_u32(buf, offset, metadata->num_planes);

    for (uint32_t i = 0; i < metadata->num_crtcs; i++) {
	sg_serialize_crtc(buf, offset, &metadata->crtcs[i]);
    }

    for (uint32_t i = 0; i < metadata->num_connectors; i++) {
	sg_serialize_connector(buf, offset, &metadata->connectors[i]);
    }

    for (uint32_t i = 0; i < metadata->num_planes; i++) {
	sg_serialize_plane(buf, offset, &metadata->planes[i]);
    }

}

void print_serialized_bytes(uint8_t *buf, uint32_t req_size, uint32_t offset)
{
    printf("serialized request size: %u\n", req_size);
    printf("response offset: %u\n", offset);

    printf("\n\nhex dump:\n");

    for (size_t i = 0; i < req_size; i++) {
	printf("%02X ", buf[i]);

	if ( (i + 1) % 8 == 0)
	    printf("\n");
    }
}

void generate_sg_request_header(struct sg_request_header *header, uint32_t size)
{
	header->version = SG_ABI_VERSION;
	header->command = SG_CMD_VALIDATE_TRANSACTION;
	header->size = size;
	header->response_offset = size;
	header->response_size = sizeof(struct sg_validate_response);
}

void print_sg_request_header(struct sg_request_header *header)
{
	printf("struct sg_request_header:\n");
	printf("SG_ABI_VERSION: %u\n", header->version);
	printf("COMMAND: %u\n", header->command);
	printf("SIZE: %u\n", header->size);
	printf("RESPONSE_OFFSET: %u\n", header->response_offset);
	printf("RESPONSE_SIZE: %u\n", header->response_size);
}

void print_sg_response(struct sg_validate_response *resp)
{
	printf("struct sg_validate_response:\n");
	printf("VERSION: %u\n",resp->version);
	printf("DECISION: %u\n",resp->decision);
	printf("REASON: %u\n",resp->reason);
}


