MAKEFLAGS += --silent

report:
	cd Report && $(MAKE)

code:
	cd Code\ and\ Work && $(MAKE)

clean:
	-@cd Report && $(MAKE) clean
	-@cd Code\ and\ Work && $(MAKE) clean
