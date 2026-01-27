#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/3ds_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
# GRAPHICS is a list of directories containing graphics files
# GFXBUILD is the directory where converted graphics files will be placed
#   If set to $(BUILD), it will statically link in the converted
#   files as if they were data files.
#
# NO_SMDH: if set to anything, no SMDH file is generated.
# ROMFS is the directory which contains the RomFS, relative to the Makefile (Optional)
# APP_TITLE is the name of the app stored in the SMDH file (Optional)
# APP_DESCRIPTION is the description of the app stored in the SMDH file (Optional)
# APP_AUTHOR is the author of the app stored in the SMDH file (Optional)
# ICON is the filename of the icon (.png), relative to the project folder.
#   If not set, it attempts to use one of the following (in this order):
#     - <Project name>.png
#     - icon.png
#     - <libctru folder>/default_icon.png
#---------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source  source/std source/std/GW_ROM source/virtual_i_o  source/SM5XX  source/SM5XX/SM5A  source/SM5XX/SM510  source/SM5XX/SM511_SM512
DATA		:=	data
INCLUDES	:=	include source  source/std source/std/GW_ROM source/virtual_i_o  source/SM5XX  source/SM5XX/SM5A  source/SM5XX/SM510  source/SM5XX/SM511_SM512
GRAPHICS	:=	gfx
#GFXBUILD	:=	$(BUILD)
ROMFS		:=	romfs
GFXBUILD	:=	$(ROMFS)/gfx

# Set EMBEDDED=1 to build with embedded ROMs/graphics.
# Default (EMBEDDED=0) builds without embedded assets and relies on an external .ykp pack.
EMBEDDED ?= 0

# Set SHOW_MSG_ROM=0 to build without show Warning rom data
SHOW_MSG_ROM ?= 1

# Set DEBUG=0 to build with debug text
DEBUG ?= 0

# Minimum external pack content version that this build accepts.
# Bump this when the app expects newer pack contents (textures/layout/etc.).
ROMPACK_CONTENT_VERSION_REQUIRED ?= 3

# Extra defines for embedded/pack builds (appended after CFLAGS is set).
ROMPACK_DEFINES ?=

# Always define the required content version (applies when a pack is loaded).
ROMPACK_DEFINES += -DYOKOI_ROMPACK_REQUIRED_CONTENT_VERSION=$(ROMPACK_CONTENT_VERSION_REQUIRED)

ifeq ($(strip $(EMBEDDED)),0)
	SOURCES := $(filter-out source/std/GW_ROM,$(SOURCES))
	INCLUDES := $(filter-out source/std/GW_ROM,$(INCLUDES))
	# Suffix output artifacts so pack-only builds are easy to distinguish.
	TARGET := $(TARGET)_rompack
	# Pack-only build: do not embed any game textures/ROMs.
	# We still embed minimal UI textures (font, logo) via romfs so menu text renders.
	GRAPHICS := gfx
	ROMFS := romfs
	GFXBUILD := $(ROMFS)/gfx
	ROMPACK_UI_GFXFILES := texte_3ds.t3s logo_pioupiou.t3s noise.t3s
endif

ifneq ($(strip $(EMBEDDED)),0)
	ROMPACK_DEFINES += -DYOKOI_EMBEDDED_ASSETS=1
endif

ifneq ($(strip $(DEBUG)),0)
	ROMPACK_DEFINES += -DYOKOI_DEBUG=1
endif


ifneq ($(strip $(SHOW_MSG_ROM)),0)
	ROMPACK_DEFINES += -DYOKOI_SHOW_MSG_ROM
else
	TARGET := $(TARGET)_no_warning
endif


APP_TITLE		:=	Yokoi G&W Emulator
APP_DESCRIPTION	:=	a Game&Watch emulator for 3ds
APP_AUTHOR		:=	RetroValou

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft

CFLAGS	:=	-g -Wall -O2 -mword-relocations \
			-ffunction-sections \
			$(ARCH)

CFLAGS	+=	$(INCLUDE) -D__3DS__

# Apply pack-only defines (if any).
CFLAGS	+=	$(ROMPACK_DEFINES)

#CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++20

ASFLAGS	:=	-g $(ARCH)
LDFLAGS	=	-specs=3dsx.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)

LIBS	:= -lcitro2d -lcitro3d -lctru -lm

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CTRULIB)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PICAFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.v.pica)))
SHLISTFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.shlist)))
GFXFILES	:=	$(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.t3s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

	# In non-embedded builds, only include minimal UI textures in romfs.
	# (Game textures come from the external .ykp pack.)
	ifeq ($(strip $(EMBEDDED)),0)
		GFXFILES := $(ROMPACK_UI_GFXFILES)
	endif

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#---------------------------------------------------------------------------------
	export LD	:=	$(CC)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
ifeq ($(GFXBUILD),$(BUILD))
#---------------------------------------------------------------------------------
export T3XFILES :=  $(GFXFILES:.t3s=.t3x)
#---------------------------------------------------------------------------------
else
#---------------------------------------------------------------------------------
export ROMFS_T3XFILES	:=	$(patsubst %.t3s, $(GFXBUILD)/%.t3x, $(GFXFILES))
#export T3XHFILES		:=	$(patsubst %.t3s, $(BUILD)/%.h, $(GFXFILES))
#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

export OFILES_SOURCES 	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES)) \
			$(PICAFILES:.v.pica=.shbin.o) $(SHLISTFILES:.shlist=.shbin.o) \
			$(addsuffix .o,$(T3XFILES))

export OFILES := $(OFILES_BIN) $(OFILES_SOURCES)

export HFILES	:=	$(PICAFILES:.v.pica=_shbin.h) $(SHLISTFILES:.shlist=_shbin.h) \
			$(addsuffix .h,$(subst .,_,$(BINFILES))) #\
			#$(GFXFILES:.t3s=.h)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export _3DSXDEPS	:=	$(if $(NO_SMDH),,$(OUTPUT).smdh)

ifeq ($(strip $(ICON)),)
	icons := $(wildcard meta/*.png)
	ifneq (,$(findstring $(TARGET).png,$(icons)))
		export APP_ICON := $(TOPDIR)/meta/$(TARGET).png
	else
		ifneq (,$(findstring icon.png,$(icons)))
			export APP_ICON := $(TOPDIR)/meta/icon.png
		endif
	endif
else
	export APP_ICON := $(TOPDIR)/meta/$(ICON)
endif

ifeq ($(strip $(NO_SMDH)),)
	export _3DSXFLAGS += --smdh=$(CURDIR)/$(TARGET).smdh
endif

ifneq ($(ROMFS),)
	export _3DSXFLAGS += --romfs=$(CURDIR)/$(ROMFS)
endif

.PHONY: all clean

.PHONY: build_3dsx build_cia

all: build_3dsx build_cia

build_3dsx: $(BUILD) $(GFXBUILD) $(DEPSDIR) $(ROMFS_T3XFILES) $(T3XHFILES)
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

build_cia: build_3dsx
	@echo "=== Build .cia ==="
	@bannertool makebanner -i "meta/banner.png" -a "meta/audio.wav" -o "$(BUILD)/banner.bnr"
	@bannertool makesmdh -s "$(APP_TITLE)" -l "$(APP_DESCRIPTION)" -p "$(APP_AUTHOR)" -i "$(APP_ICON)" -f "nosavebackups,visible" -o "$(BUILD)/icon.icn"
	@makerom -f cia -o "$(OUTPUT).cia" -target t -exefslogo \
		-elf "$(TARGET).elf" \
		-rsf "meta/app.rsf" \
		-banner "$(BUILD)/banner.bnr" \
		-icon "$(BUILD)/icon.icn" \
		-DAPP_TITLE="$(APP_TITLE)" \
		-DAPP_PRODUCT_CODE="CTR-P-YOKO" \
		-DAPP_UNIQUE_ID="0x82523" \
		-DAPP_ROMFS="$(CURDIR)/$(ROMFS)"


#---------------------------------------------------------------------------------

$(BUILD):
	@mkdir -p $@

ifneq ($(GFXBUILD),$(BUILD))
$(GFXBUILD):
	@mkdir -p $@
endif

ifneq ($(DEPSDIR),$(BUILD))
$(DEPSDIR):
	@mkdir -p $@
endif

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).3dsx $(OUTPUT).smdh $(TARGET).elf $(GFXBUILD)

#---------------------------------------------------------------------------------
$(GFXBUILD)/%.t3x	:	%.t3s
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@tex3ds -i $< -d $(DEPSDIR)/$*.d -o $(GFXBUILD)/$*.t3x

#---------------------------------------------------------------------------------
else

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).3dsx	:	$(OUTPUT).elf $(_3DSXDEPS)

$(OFILES_SOURCES) : $(HFILES)

$(OUTPUT).elf	:	$(OFILES)

#---------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#---------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

#---------------------------------------------------------------------------------
.PRECIOUS	:	%.t3x %.shbin
#---------------------------------------------------------------------------------
%.t3x.o	%_t3x.h :	%.t3x
#---------------------------------------------------------------------------------
	$(SILENTMSG) $(notdir $<)
	$(bin2o)

#---------------------------------------------------------------------------------
%.shbin.o %_shbin.h : %.shbin
#---------------------------------------------------------------------------------
	$(SILENTMSG) $(notdir $<)
	$(bin2o)

-include $(DEPSDIR)/*.d

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------