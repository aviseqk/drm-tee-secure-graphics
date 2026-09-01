/*
########################################################
#		sg-drm-atomic-modeset
#
## Authored By: Abhishek
## Version: 1.0
##
## Description:
## This is a utility program written using libdrm
## which constructs an atomic KMS request from
## userspace and succesfully commits it
##
## This project does not do an elaborate property
## discovery or deal with multiple planes and CRTCs
## and page flipping, etc, it rather just takes
## one known good atomic commit and performs it
## to validate a test pattern successfully scans out
## using the atomic KMS userspace request

## NOTE: This is the closest path to what our actual
## REE <-> TEE Secure Graphics Stack Application
## will be auditing and authorizing
##
## Eventually, when Secure World will need to validate
## these same properties, this utility will help. 
########################################################
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

uint32_t find_property_id(int fd, uint32_t object_id, uint32_t object_type, const char* name);
uint32_t find_property_value(int fd, uint32_t object_id, uint32_t object_type, const char *name);

typedef struct myObject{
	uint32_t connector_id;
	uint32_t encoder_id;
	uint32_t crtc_id;
	uint32_t fb_id;
	uint32_t plane_id;

	drmModeModeInfo mode;

	uint16_t mode_hdisp;
	uint16_t mode_vdisp;

	uint32_t mode_refresh;

	uint32_t bpp;
	uint32_t pitch;
	uint64_t size;

	uint32_t fb_handle;

	void *map;
} myObject;

static myObject obj;

static void printMyObject(myObject *obj) {
	printf("Connector: %u\n", obj->connector_id);
	printf("Encoder: %u\n", obj->encoder_id);
	printf("CRTC: %u\n", obj->crtc_id);
	printf("Mode: %ux%u@%u\n", obj->mode_hdisp, obj->mode_vdisp, obj->mode_refresh);
	printf("BPP: %u\n", obj->bpp);
	printf("Pitch: %u\n", obj->pitch);
	printf("Size: %u\n", obj->size);
	printf("GEM Handle: %u\n", obj->fb_handle);
}

static void populate_pixels_xrgb8888_rgb(myObject* obj)
{
	uint8_t *base = (uint8_t *)obj->map;
	int row_count = 0;
	int pixel_count = 0;

	for (int y = 0; y < obj->mode_vdisp; y++) {
		uint32_t *row_start = (uint32_t *)(base + y * obj->pitch);
		for (int x = 0; x < obj->mode_hdisp; x++) {
			uint32_t pixel;

			if (y < obj->mode_vdisp / 3)
				pixel = 0x00FF9933;
			else if (y < (2 * obj->mode_vdisp) / 3)
				pixel = 0x00FFFFFF;
			else
				pixel = 0x00008000;

			row_start[x] = pixel;

			pixel_count++;
		}
		row_count++;
	}
	printf("row_count : %d pixel_count : %d\n", row_count, pixel_count);
}

int main(void)
{

	// Open the primary DRM device node
	int drm_fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
	if (drm_fd < 0) {
		perror("Failed to open /dev/dri/card0");
		return 1;
	}

	// Enable universal planes
	if (drmSetClientCap(drm_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
		perror("drmSetClientCap(UNIVERSAL_PLANES)");
		goto cleanup;
	}

	// Enable atomic KMS to enable atomic properties and capabilities for this client
	if (drmSetClientCap(drm_fd, DRM_CLIENT_CAP_ATOMIC, 1)) {
		perror("drmSetClientCap(ATOMIC)");
		goto cleanup;
	}

	// Retrieve the DRM Resources
	drmModeRes *resources = drmModeGetResources(drm_fd);
	if (!resources) {
		perror("failed to retrieve DRM resources");
		close(drm_fd);
		return 1;
	}

	// Connector
	for (int i = 0; i < resources->count_connectors; i++) {
		drmModeConnector *conn = drmModeGetConnector(drm_fd, resources->connectors[i]);
		if (!conn) continue;

		if (conn->connection == 1) {
			printf("found the connected connector: %d\n", conn->connector_id);
			obj.connector_id = conn->connector_id;

			// finding the encoder associated with it
			printf("the associated encoder is %d\n", conn->encoder_id);
			obj.encoder_id = conn->encoder_id;
		}
		
                if (conn->count_modes > 0) {
			// TODO: verify this: rn assuming the preferred resolution is index 0
			obj.mode_hdisp = conn->modes[0].hdisplay;
			obj.mode_vdisp = conn->modes[0].vdisplay;
			obj.mode_refresh = conn->modes[0].vrefresh;

			obj.mode = conn->modes[0];
		}
		drmModeFreeConnector(conn);
        }


	// find the CRTC this encoder can drive
	drmModeEncoder *enc = drmModeGetEncoder(drm_fd, obj.encoder_id);
	if (!enc) {
		perror("unable to get the encoder");
		return 1;
	}

	for (int i = 0; i < resources->count_crtcs; i++) {
		
		if (enc->possible_crtcs & (1 << i)) {
			printf("encoder %u can drive crtc %u\n", enc->encoder_id, resources->crtcs[i]);
			obj.crtc_id = resources->crtcs[i];
		}
	}
	drmModeFreeEncoder(enc);


	// plane discovery - to find the primary plane ID
	drmModePlaneRes *plane_res = drmModeGetPlaneResources(drm_fd);
	if (!plane_res) {
		perror("drmModeGetPlaneResources");
		return 1;
	}

	int crtc_index = -1;
	for (int i = 0; i < resources->count_crtcs; i++) {
	if (resources->crtcs[i] == obj.crtc_id) {
		crtc_index = i;
		break;
		}
	}

	for (uint32_t i = 0; i < plane_res->count_planes; i++) {
		drmModePlane *plane = drmModeGetPlane(drm_fd, plane_res->planes[i]);

		if (!plane)
			continue;

		printf("Plane ID: %u\n", plane->plane_id);

		uint32_t plane_type = find_property_value(drm_fd, plane->plane_id, DRM_MODE_OBJECT_PLANE, "type");
		printf("Plane Type: %u\n", plane_type);

		if (plane->possible_crtcs & (1 << crtc_index)) {

			if (plane_type == DRM_PLANE_TYPE_PRIMARY) {
				printf("Found primary plane: %u\n", plane->plane_id);

				obj.plane_id = plane->plane_id;

				drmModeFreePlane(plane);
				break;
			}
			drmModeFreePlane(plane);
		}
	}
	



	obj.bpp = 32;	// making bits per pixel 32 as that is common apparently

	// Create the Dumb Framebuffer/Pixel Storage
	int ret = drmModeCreateDumbBuffer(drm_fd, obj.mode_hdisp, obj.mode_vdisp, obj.bpp, 0, &obj.fb_handle, &obj.pitch, &obj.size);

	if (ret < 0) {
		perror("drmModeCreateDumbBuffer");
		return 1;
	}
	

	// now, map the dumb buffer with the fb_handle
	struct drm_mode_map_dumb map = {
		.handle = obj.fb_handle,
	};

	if (ioctl(drm_fd, DRM_IOCTL_MODE_MAP_DUMB, &map) < 0) {
		perror("DRM_IOCTL_MODE_MAP_DUMB");
		return 1;
	}

	obj.map = mmap(0, obj.size, PROT_READ | PROT_WRITE, MAP_SHARED, drm_fd, map.offset);
	
	if (obj.map == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	printMyObject(&obj);

	// using XRGB8888 as the pixel pattern and populating the pixels
	populate_pixels_xrgb8888_rgb(&obj);

	uint32_t fb_id;

	// create a DRM framebuffer object around the dumb buffer
	ret = drmModeAddFB(drm_fd, obj.mode_hdisp, obj.mode_vdisp, 24, obj.bpp, obj.pitch, obj.fb_handle, &fb_id);

	if (ret) {
		perror("drmModeAddFB");
		return 1;
	}
	obj.fb_id = fb_id;
	printf("DRM Framebuffer created: %u\n", obj.fb_id);
	
	uint32_t connector_id = obj.connector_id;

	printf("DEBUG: before property queries\n");
	printf("DEBUG: connector_id = %u\n", obj.connector_id);
	printf("DEBUG: crtc_id      = %u\n", obj.crtc_id);
	printf("DEBUG: plane_id     = %u\n", obj.plane_id);

	// fetch the relavant properties for our atomic state
	
	uint32_t conn_crtc_id = find_property_id(drm_fd, obj.connector_id, DRM_MODE_OBJECT_CONNECTOR, "CRTC_ID");
	uint32_t crtc_active = find_property_id(drm_fd, obj.crtc_id, DRM_MODE_OBJECT_CRTC, "ACTIVE");
	uint32_t crtc_mode_id = find_property_id(drm_fd, obj.crtc_id, DRM_MODE_OBJECT_CRTC, "MODE_ID");

	// props for plane
	uint32_t plane_fb_id = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "FB_ID");
	uint32_t plane_crtc_id = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "CRTC_ID");
	uint32_t src_x = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "SRC_X");
	uint32_t src_y = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "SRC_Y");
	uint32_t src_w = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "SRC_W");
	uint32_t src_h = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "SRC_H");
	uint32_t crtc_x = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "CRTC_X");
	uint32_t crtc_y = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "CRTC_Y");
	uint32_t crtc_w = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "CRTC_W");
	uint32_t crtc_h = find_property_id(drm_fd, obj.plane_id, DRM_MODE_OBJECT_PLANE, "CRTC_H");

	printf("conn_crtc_id: %u\n", conn_crtc_id);
	printf("crtc_active: %u\n", crtc_active);
	printf("crtc_mode_id: %u\n", crtc_mode_id);
	printf("plane_fb_id: %u\n", plane_fb_id);
	printf("plane_crtc_id: %u\n", plane_crtc_id);
	printf("src_x: %u\n", src_x);
	printf("src_y: %u\n", src_y);
	printf("src_w: %u\n", src_w);
	printf("src_h: %u\n", src_h);
	printf("crtc_x: %u\n", crtc_x);
	printf("crtc_y: %u\n", crtc_y);
	printf("crtc_w: %u\n", crtc_w);
	printf("crtc_h: %u\n", crtc_h);

	// TODO: add a check that any property requested above does not result in zero

	// create a DRM Property Blob for the Mode, because atomic KMS expects the mode represented by DRM Property Blob
	// and not like raw direct drmModeModeInfo object as was the case in the legacy KMS commit
	uint32_t mode_blob_id;

	ret = drmModeCreatePropertyBlob(drm_fd, &obj.mode, sizeof(obj.mode), &mode_blob_id);

	if (ret) {
		perror("drmModeCreatePropertyBlob");
		goto cleanup;
	}

	printf("DEBUG: MODE_ID blob: %u\n", mode_blob_id);


	// create the atomic request - the userspace transaction object in terms of commit
	drmModeAtomicReq *req = drmModeAtomicAlloc();

	if (!req) {
		perror("drmModeAtomicAlloc");
		goto cleanup;
	}

	// adding the relavant properties to the atomic request object using the discovered property ids and the desired values
	drmModeAtomicAddProperty(req, obj.connector_id, conn_crtc_id, obj.crtc_id);

	drmModeAtomicAddProperty(req, obj.crtc_id, crtc_active, 1);
	drmModeAtomicAddProperty(req, obj.crtc_id, crtc_mode_id, mode_blob_id);

	drmModeAtomicAddProperty(req, obj.plane_id, plane_fb_id, obj.fb_id);
	drmModeAtomicAddProperty(req, obj.plane_id, plane_crtc_id, obj.crtc_id);

	drmModeAtomicAddProperty(req, obj.plane_id, src_x, 0);
	drmModeAtomicAddProperty(req, obj.plane_id, src_y, 0);
	drmModeAtomicAddProperty(req, obj.plane_id, src_w, obj.mode_hdisp << 16);
	drmModeAtomicAddProperty(req, obj.plane_id, src_h, obj.mode_vdisp << 16);

	drmModeAtomicAddProperty(req, obj.plane_id, crtc_x, 0);
	drmModeAtomicAddProperty(req, obj.plane_id, crtc_y, 0);
	drmModeAtomicAddProperty(req, obj.plane_id, crtc_w, obj.mode_hdisp);
	drmModeAtomicAddProperty(req, obj.plane_id, crtc_h, obj.mode_vdisp);


	// atomic commit call - send the transaction created via atomicReq to the kernel
	
	ret = drmModeAtomicCommit(drm_fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);

	if (ret) {
		perror("drmModeAtomicCommit");
		goto cleanup;
	} else {
		printf("Atomic modeset commmitted successfully\n");
	}


	sleep(3);
	// Cleanup
cleanup:
	printf("LOG: starting resources cleanup\n");
	
	drmModeRmFB(drm_fd, obj.fb_id);
	munmap(obj.map, obj.size);

	struct drm_mode_destroy_dumb destroy = {
		.handle = obj.fb_handle,
	};
	ioctl(drm_fd, DRM_IOCTL_MODE_DESTROY_DUMB, &destroy);

	drmModeFreeResources(resources);
	close(drm_fd);

	return 0;
}

uint32_t find_property_id(int fd, uint32_t object_id, uint32_t object_type, const char* name)
{
	drmModeObjectProperties *props = drmModeObjectGetProperties(fd, object_id, object_type);
	
	if (!props) {
		printf("drmModeObjectGetProperties failed: object=%u type=0x%x\n", object_id, object_type);
		return 0;
	}

	printf("object=%u type=0x%x has %u properties\n", object_id, object_type, props->count_props);

	uint32_t property_id = 0;

	for (uint32_t i = 0; i < props->count_props; i++) {
		drmModePropertyRes *prop = drmModeGetProperty(fd, props->props[i]);

		if (!prop)
			continue;

		printf("  property id=%u name='%s'\n", prop->prop_id, prop->name);

		if (strcmp(prop->name, name) == 0) {
			property_id = prop->prop_id;
			drmModeFreeProperty(prop);
			break;
		}
		drmModeFreeProperty(prop);
	}
	drmModeFreeObjectProperties(props);

	return property_id;
}

uint32_t find_property_value(int fd, uint32_t object_id, uint32_t object_type, const char *name)
{
	drmModeObjectProperties *props = drmModeObjectGetProperties(fd, object_id, object_type);
	
	if (!props)
		return 0;

	uint32_t property_value = 0;

	for (uint32_t i = 0; i < props->count_props; i++) {
		drmModePropertyRes *prop = drmModeGetProperty(fd, props->props[i]);

		if (!prop)
			continue;

		if (strcmp(prop->name, name) == 0) {
			property_value = props->prop_values[i];
			drmModeFreeProperty(prop);
			break;
		}
		drmModeFreeProperty(prop);
	}
	drmModeFreeObjectProperties(props);

	return property_value;

}
