#
# Makefile for nfc devices
#

obj-$(CONFIG_NFC_PN544)		+= pn544/
obj-$(CONFIG_NFC_MICROREAD)	+= microread/
obj-$(CONFIG_NFC_PN533)		+= pn533.o
obj-$(CONFIG_NFC_WILINK)	+= nfcwilink.o
obj-$(CONFIG_NFC_MEI_PHY)	+= mei_phy.o
obj-$(CONFIG_NFC_CXD224X_I2C)	+= cxd224x/
obj-$(CONFIG_FELICA_RCS730)    += rcs730/

ccflags-$(CONFIG_NFC_DEBUG) := -DDEBUG
