.PHONY: clean check

all:
	$(MAKE) -C src

check:
	$(MAKE) -C tests

clean :
	for dir in $(cleandirs); do $(MAKE) clean -C $$dir; done

cleandirs = src
