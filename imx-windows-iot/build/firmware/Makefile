# This Makefile inspired by http://lackof.org/taggart/hacking/make-example/

IMX8_BOARDS=\
	Franta_500M MX8M_EVK \
	MX8M_MINI_EVK \
	MX8M_NANO_EVK \

# UPDATEDIRS=$(IMX67_BOARDS:%=update-%)
# COMMITDIRS=$(IMX67_BOARDS:%=commit-%)
# CLEANDIRS=$(IMX67_BOARDS:%=clean-%)
# INITDIRS=$(IMX67_BOARDS:%=init-%)

#Serialize since some builds are mutually exclusive (imx8, EKD2)
# serialize_builds(command, common arguments, serialized list)
define serialize_builds
	$(foreach PARAM, $3, \
	$1 $2 $(PARAM) && \
	) true
endef

imx8_all: imx8_boards_serialized

#Allows for a call to specific boards ie: "make Franta_500M MX8M_EVK", but will not enforce serialization of the builds

$(IMX8_BOARDS):
	$(MAKE) imx8_build IMX8_TARGET=$@

imx8_boards_serialized:
	$(call serialize_builds,$(MAKE) -f imx8.mk, imx8_build, $(foreach BOARD,$(IMX8_BOARDS),IMX8_TARGET=$(BOARD)))

#Checks for the case sensitivity of all output folders all at once.
#Force serial build using $(foreach to avoid too many pop-ups which can cause some to fail.
init-dirs:
	$(foreach BOARD, $(INITDIRS), \
	$(MAKE) -C $(BOARD:init-%=%) verify_case_sensitivity && \
	sleep 1 && \
	) true

#IMX8 requires a re-build to do this.
imx8_update-ffu:
	$(call serialize_builds,$(MAKE) -f imx8.mk, imx8_update-ffu, $(foreach BOARD,$(IMX8_BOARDS),IMX8_TARGET=$(BOARD)))

#IMX8 requires a re-build to do this.
imx8_commit-firmware:
	$(call serialize_builds, $(MAKE) -f imx8.mk, imx8_commit-firmware, $(foreach BOARD,$(IMX8_BOARDS),IMX8_TARGET=$(BOARD)))

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean

include CGManifests.mk

# Print off a list of boards to display
.PHONY: print_board_list
print_board_list:
	@echo $(BOARDS) | tr " " "\n"

.PHONY: $(IMX8_BOARDS) $(INITDIRS) $(UPDATEDIRS) $(CLEANDIRS)
.PHONY: all update-ffu commit-ffu clean init-dirs
.PHONY: imx8_all imx8_update-ffu imx8_commit-firmware imx8_boards_serialized

