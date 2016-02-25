.PHONY: toolbox

CELL_MK_DIR = $(CELL_SDK)/samples/mk
include $(CELL_MK_DIR)/sdk.makedef.mk
CELL_INC_DIR = $(CELL_SDK)/target/include

#=========================================================

SOURCE	= source
RELEASE = release
BIN	= bin
NPDRM	= NPDRM_RELEASE
FTP	= openftp

#=========================================================

APPNAME	= toolBOX
APPID	= RBGTLBOX2
CONTENT_ID=UP0001-$(APPID)_00-0000000000000000

#=========================================================

MAKE_SELF1 = scetool -v --sce-type=SELF --compress-data=TRUE --skip-sections=FALSE --key-revision=04 --self-ctrl-flags=4000000000000000000000000000000000000000000000000000000000000002 --self-auth-id=1010000001000003 --self-app-version=0001000000000000 --self-add-shdrs=TRUE --self-vendor-id=01000002 --self-type=NPDRM --self-fw-version=0003004000000000 --np-license-type=FREE --np-content-id=$(CONTENT_ID) --np-app-type=EXEC --np-real-fname=EBOOT.BIN --encrypt $(PPU_TARGET)
MAKE_SELF2 = scetool --sce-type=SELF --compress-data=TRUE --skip-sections=FALSE --key-revision=04 --self-ctrl-flags=4000000000000000000000000000000000000000000000000000000000000002 --self-auth-id=1010000001000003 --self-app-version=0001000000000000 --self-add-shdrs=TRUE --self-vendor-id=01000002 --self-type=APP --self-fw-version=0003004000000000 --encrypt $(PPU_TARGET)
PSN_PKG_NPDRM = psn_package_npdrm
MOD_ELF = modELF

#=========================================================

PPU_SRCS =	$(SOURCE)/graphics.cpp 
PPU_SRCS +=	$(VPSHADER_PPU_OBJS) $(FPSHADER_PPU_OBJS) 
PPU_SRCS +=	$(SOURCE)/$(APPNAME).cpp $(SOURCE)/fonts.c $(SOURCE)/fonts_render.c $(SOURCE)/aes.c $(SOURCE)/sha1.c
PPU_SRCS +=	$(SOURCE)/$(FTP)/ftp.c $(SOURCE)/$(FTP)/ftpcmd.c $(SOURCE)/$(FTP)/functions.c

PPU_SRCS +=	$(SOURCE)/peek_poke.cpp $(SOURCE)/mm.cpp $(SOURCE)/hvcall.cpp $(SOURCE)/syscall36.cpp $(SOURCE)/aes_omac.cpp 
#=========================================================

PPU_TARGET =	$(APPNAME)_BARE.elf
PPU_INCDIRS= -Iinclude -I$(CELL_INC_DIR) -I$(CELL_INC_DIR)/usb/usbpad -I$(CELL_INC_DIR)/usb/usbkb -I$(CELL_SDK)/target/ppu/include/sysutil -I$(CELL_SDK)/target/ppu/include -Isource/zlib
PPU_LDLIBS += -lfont_stub -lfontFT_stub -lfreetype_stub -lpthread -lm -lnet_stub -lnetctl_stub -lpngdec_stub -ldbgfont_gcm -lgcm_cmd -lgcm_sys_stub -lio_stub -lsysutil_stub -lsysmodule_stub -lsysutil_game_stub -lfs_stub -lhttp_util_stub -ljpgdec_stub -lhttp_stub
PPU_LDLIBS += -lusbd_stub -lrtc_stub -lsysutil_screenshot_stub -lsysutil_np_stub -l./libccons

PPU_CPPFLAGS	:= -Wformat=0
PPU_CFLAGS	+= -g -O2 -fno-exceptions 
PPU_OPTIMIZE_LV := -O2 -fno-exceptions 

all : $(PPU_TARGET)

VPSHADER_SRCS = vpshader.cg vpshader2.cg
FPSHADER_SRCS = fpshader.cg fpshader2.cg

VPSHADER_PPU_OBJS = $(patsubst %.cg, $(OBJS_DIR)/$(SOURCE)/%.ppu.o, $(VPSHADER_SRCS))
FPSHADER_PPU_OBJS = $(patsubst %.cg, $(OBJS_DIR)/$(SOURCE)/%.ppu.o, $(FPSHADER_SRCS))

include $(CELL_MK_DIR)/sdk.target.mk

PPU_OBJS += $(VPSHADER_PPU_OBJS) $(FPSHADER_PPU_OBJS)

$(VPSHADER_PPU_OBJS): $(OBJS_DIR)/$(SOURCE)/%.ppu.o : %.vpo
	@mkdir -p $(dir $(@))
	@$(PPU_OBJCOPY)  -I binary -O elf64-powerpc-celloslv2 -B powerpc --set-section-align .data=7 --set-section-pad .data=128 $< $@ > nul

$(FPSHADER_PPU_OBJS): $(OBJS_DIR)/$(SOURCE)/%.ppu.o : %.fpo
	@mkdir -p $(dir $(@))
	@$(PPU_OBJCOPY)  -I binary -O elf64-powerpc-celloslv2 -B powerpc --set-section-align .data=7 --set-section-pad .data=128 $< $@ > nul


toolbox all : $(PPU_TARGET)
	@mkdir -p $(BIN)
	@$(PPU_STRIP) -s $< -o $(OBJS_DIR)/$(PPU_TARGET)
	@$(MOD_ELF) ./objs/$(APPNAME)_BARE.elf
	@$(MAKE_SELF1) $(RELEASE)/$(NPDRM)/USRDIR/EBOOT.BIN
	@$(MAKE_SELF2) $(RELEASE)/$(NPDRM)/USRDIR/RELOAD.SELF
	# @$(MAKE_SELF) n -c -s3.40.0 -034000 -x40 -z02 -prelease/cap_flag.bin -n$(CONTENT_ID) -tEBOOT.BIN -lFREE -me -orelease/sec_pad.bin ./objs/$(APPNAME)_BARE.elf $(RELEASE)/$(NPDRM)/USRDIR/EBOOT.BIN
#	@$(MAKE_SELF) a -f -c -s3.40.0 -034000 -x40 -z02 -prelease/cap_flag.bin -tEBOOT.BIN -lFREE -me -orelease/sec_pad.bin ./objs/$(APPNAME)_BARE.elf $(RELEASE)/$(NPDRM)/USRDIR/EBOOT_DISC.BIN
	$(PSN_PKG_NPDRM) $(RELEASE)/package.conf $(RELEASE)/$(NPDRM)/
	@mv ./$(CONTENT_ID)*.pkg $(BIN)/$(CONTENT_ID).pkg

	@rm ./$(PPU_TARGET)
	@rm ./*.vpo
	@rm ./*.fpo
	@mv ./objs/$(APPNAME)_BARE.elf $(RELEASE)/$(APPNAME).elf
	@echo 
	@echo PKG and binaries: [$(BIN)]
	@echo Done!
	@rm ./$(APPNAME)_BARE.self