SG_DRM_ATOMIC_MODESET_VERSION=1.0
SG_DRM_ATOMIC_MODESET_SITE=$(BR2_EXTERNAL_SG_EXT_PATH)/package/sg-drm-atomic-modeset
SG_DRM_ATOMIC_MODESET_SITE_METHOD=local
SG_DRM_ATOMIC_MODESET_DEPENDENCIES = libdrm

define SG_DRM_ATOMIC_MODESET_BUILD_CMDS
	$(TARGET_MAKE_ENV) \
	$(MAKE) \
		CC="$(TARGET_CC)" \
		PKG_CONFIG="$(PKG_CONFIG_HOST_BINARY)" \
		STAGING_DIR="$(STAGING_DIR)" \
		-C $(@D)
endef

define SG_DRM_ATOMIC_MODESET_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 \
	$(@D)/sg-drm-atomic-modeset \
	$(TARGET_DIR)/usr/bin/sg-drm-atomic-modeset
endef

$(eval $(generic-package))
