/*
########################################################
#		sg-drm-modeset
#
## Authored By: Abhishek
## Version: 1.0
##
## Description:
## This is a utility program written using libdrm
## which discovers the currently usable KMS objects
## and create a FB using our own test pattern
## and uses KMS legacy API to make that framebuffer
## scanout to the display
##
## Eventually, when Secure World will need to validate
## these same properties, this utility will help. 
########################################################
*/

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

typedef struct myObject{
	uint32_t connector_id;
	uint32_t encoder_id;
	uint32_t crtc_id;
	uint32_t fb_id;

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
				pixel = 0x00FF0000;
			else if (y < (2 * obj->mode_vdisp) / 3)
				pixel = 0x0000FF00;
			else
				pixel = 0x000000FF;

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

	// legacy modeset drmModeSetCrtc API call
	ret = drmModeSetCrtc(drm_fd, obj.crtc_id, obj.fb_id, 0, 0, &connector_id, 1, &obj.mode);

	if (ret) {
		perror("drmModeSetCrtc");
		return 1;
	}

	printf("Modeset successful!\n");


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
