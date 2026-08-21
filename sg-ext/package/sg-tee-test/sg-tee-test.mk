SG_TEE_TEST_VERSION = 1.0
SG_TEE_TEST_SITE = $(BR2_EXTERNAL_SG_EXT_PATH)/package/sg-tee-test/src
SG_TEE_TEST_SITE_METHOD = local

SG_TEE_TEST_DEPENDENCIES = optee_client_ext

SG_TEE_TEST_TA_DEV_KIT = $(BR2_PACKAGE_SG_TEE_TEST_TA_DEV_KIT)

SG_TEE_TEST_TA_UUID = cc6c3285-e725-4249-bfc4-3ad5a558e051

define SG_TEE_TEST_BUILD_HOST
	$(TARGET_MAKE_ENV) \
		$(MAKE) \
		CC="$(TARGET_CC)" \
		TEEC_EXPORT="$(STAGING_DIR)/usr" \
		-C $(@D)/host
endef

define SG_TEE_TEST_BUILD_TA
	$(TARGET_CONFIGURE_OPTS) \
	$(MAKE) \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		O=out \
		TA_DEV_KIT_DIR="$(SG_TEE_TEST_TA_DEV_KIT)" \
		-C $(@D)/ta \
		all
endef

define SG_TEE_TEST_BUILD_CMDS
	$(SG_TEE_TEST_BUILD_HOST) && $(SG_TEE_TEST_BUILD_TA)
endef

define SG_TEE_TEST_INSTALL_HOST_BINARY
	$(INSTALL) -D -m 0755 \
		$(@D)/host/sg-tee-test \
		$(TARGET_DIR)/usr/bin/sg-tee-test
endef

define SG_TEE_TEST_INSTALL_TA
	mkdir -p $(TARGET_DIR)/lib/optee_armtz
	$(INSTALL) -m 0444 \
		$(@D)/ta/out/$(SG_TEE_TEST_TA_UUID).ta \
		$(TARGET_DIR)/lib/optee_armtz/
endef

define SG_TEE_TEST_INSTALL_TARGET_CMDS
	$(SG_TEE_TEST_INSTALL_HOST_BINARY) && $(SG_TEE_TEST_INSTALL_TA)
endef

$(eval $(generic-package))
