MAKEFLAGS += --silent

report:
	cd Report && $(MAKE)

clean:
	-@cd Report && $(MAKE) clean
