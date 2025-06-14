# GNU Make workspace makefile autogenerated by Premake

ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifeq ($(config),debug)
  ai_cli_config = debug

else ifeq ($(config),release)
  ai_cli_config = release

else
  $(error "invalid configuration $(config)")
endif

PROJECTS := ai_cli

.PHONY: all clean help $(PROJECTS) 

all: $(PROJECTS)

ai_cli:
ifneq (,$(ai_cli_config))
	@echo "==== Building ai_cli ($(ai_cli_config)) ===="
	@${MAKE} --no-print-directory -C . -f ai_cli.make config=$(ai_cli_config)
endif

clean:
	@${MAKE} --no-print-directory -C . -f ai_cli.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "  debug"
	@echo "  release"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   ai_cli"
	@echo ""
	@echo "For more information, see https://github.com/premake/premake-core/wiki"