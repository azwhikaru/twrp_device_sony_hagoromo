$(call inherit-product-if-exists, $(SRC_TARGET_DIR)/product/embedded.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)
$(call inherit-product, $(SRC_TARGET_DIR)/product/languages_full.mk)

# Inherit from hagoromo device
$(call inherit-product, device/sony/hagoromo/device.mk)

# Inherit some common Omni stuff.
$(call inherit-product, vendor/omni/config/common.mk)
$(call inherit-product, vendor/omni/config/gsm.mk)

# Device identifier. This must come after all inclusions
PRODUCT_DEVICE := hagoromo
PRODUCT_NAME := hagoromo
PRODUCT_BRAND := Sony
PRODUCT_MODEL := HAGOROMO_MODEL
PRODUCT_MANUFACTURER := Sony
PRODUCT_RELEASE_NAME := Sony HAGOROMO_MODEL