HOST_CC			:= gcc
E_CC			:= e-gcc
HOST_CCFLAGS	:= -O3
E_CCFLAGS		:= -O3

HOST_MAIN		:= src/main.c src/api.c
E_MAIN			:= src/e_main.c src/api.c

HOST_INCLUDES	:= -I/usr/local/include -I./include/
HOST_LIB_FLAGS	:= -le-hal -le-loader -lpapi
E_LIB_FLAGS		:= -le-lib

PAPI_LIB		:= /usr/local/lib/libpapi.a
SUPPORT_FILES	:= support/support.c $(PAPI_LIB) #support/mmio.c

# Epiphany Global Libraries (need ESDK installed)
# Update ESDK to where Adapteva ESDK is installed on your machine.
ESDK			:= /opt/adapteva/esdk
EHDF			:= $(ESDK)/bsps/current/platform.hdf
ELIBS			:= $(ESDK)/tools/host/lib
EINCS			:= $(ESDK)/tools/host/include
ELDF			:= $(ESDK)/bsps/current/fast.ldf
ELIBS_RUN		:= $(ESDK)/tools/host/lib:${LD_LIBRARY_PATH}

all : main e_main

# Build HOST side application
main: $(HOST_MAIN) $(SUPPORT_FILES)
	@mkdir -p bin/
	@$(HOST_CC) -o bin/$@.elf $(HOST_INCLUDES) $(HOST_CCFLAGS) $(SUPPORT_FILES) $(HOST_MAIN) -I$(EINCS) -L$(ELIBS) $(HOST_LIB_FLAGS)

# Build DEVICE side program + Convert ebinary to SREC file
e_main: $(E_MAIN)
	@mkdir -p bin/
	@$(E_CC) -T $(ELDF) $(E_CCFLAGS) $(E_MAIN) -o bin/$@.elf $(E_LIB_FLAGS)
	@e-objcopy --srec-forceS3 --output-target srec bin/$@.elf bin/$@.srec

# Adapted from: http://stackoverflow.com/questions/2214575/passing-arguments-to-make-run
# If the first argument is "run"...
ifeq (run,$(firstword $(MAKECMDGOALS)))
    # use the rest as arguments for "run"
    RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
    # ...and turn them into do-nothing targets
    $(eval $(RUN_ARGS):;@:)
endif

# Run the program
run: bin/main.elf
	@sudo -E LD_LIBRARY_PATH=$(ELIBS_RUN) EPIPHANY_HDF=$(EHDF) bin/main.elf $(RUN_ARGS)

clean:
	@rm -rf bin/