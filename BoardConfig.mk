DEVICE_PATH := device/sony/hagoromo

# For building with minimal manifest
ALLOW_MISSING_DEPENDENCIES := true


# Bootloader
TARGET_NO_BOOTLOADER := true
TARGET_BOOTLOADER_BOARD_NAME :=

# Architecture
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_VARIANT := generic

# Assert
TARGET_OTA_ASSERT_DEVICE := hagoromo

# File systems
BOARD_HAS_LARGE_FILESYSTEM := true
BOARD_SYSTEMIMAGE_PARTITION_TYPE := ext4
BOARD_USERDATAIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_USERIMAGES_USE_EXT4 := true
TARGET_USERIMAGES_USE_F2FS := true
TARGET_COPY_OUT_VENDOR := vendor
BOARD_HAS_NO_REAL_SDCARD := true

BOARD_BOOTIMAGE_PARTITION_SIZE := 0x1000000
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 0x1000000

# Kernel
BOARD_HAS_MTK := true
BOARD_CUSTOM_BOOTIMG_MK := $(DEVICE_PATH)/mkmtkbootimg.mk
# TARGET_PREBUILT_KERNEL := $(DEVICE_PATH)/prebuilt/zImage-dtb
BOARD_MKBOOTIMG_ARGS := --pagesize 2048 --base 0x10000000 \
			--kernel_offset 0x00008000 --ramdisk_offset 0x01000000 --tags_offset 0x00000100
# BOARD_KERNEL_BASE := 0x10000000
# BOARD_KERNEL_PAGESIZE := 2048
# BOARD_RAMDISK_OFFSET := 0x01000000
# BOARD_KERNEL_TAGS_OFFSET := 0x00000100
BOARD_FLASH_BLOCK_SIZE := 131072

# Mkbootimg Args
# BOARD_MKBOOTIMG_ARGS += --ramdisk_offset $(BOARD_RAMDISK_OFFSET)
# BOARD_MKBOOTIMG_ARGS += --tags_offset $(BOARD_KERNEL_TAGS_OFFSET)

# BOARD_KERNEL_IMAGE_NAME := zImage-dtb
TARGET_KERNEL_ARCH := arm
TARGET_KERNEL_HEADER_ARCH := arm
# TARGET_KERNEL_SOURCE := kernel/sony/hagoromo
# TARGET_KERNEL_CONFIG := hagoromo_defconfig


# Use offical kernel source
TARGET_KERNEL_SOURCE := $(DEVICE_PATH)/kernel_source
TARGET_KERNEL_CONFIG := BBDMP2_linux_debug_defconfig
BOARD_KERNEL_IMAGE_NAME := zImage-dtb
TARGET_KERNEL_CROSS_COMPILE_PREFIX := arm-linux-androideabi-


# Platform
TARGET_BOARD_PLATFORM := mt8590

# TWRP Configuration
TW_THEME := portrait_hdpi
TW_EXTRA_LANGUAGES := false
TW_SCREEN_BLANK_ON_BOOT := true
TW_INPUT_BLACKLIST := "hbtp_vm"
TW_USE_TOOLBOX := true

# Other
TW_NEW_ION_HEAP := true

# Neon
ARCH_ARM_HAVE_NEON := true

# Logs
TARGET_USES_LOGD := true
TWRP_INCLUDE_LOGCAT := true
