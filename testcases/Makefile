.PHONY: check update

subdirs = $(wildcard */)

update :
	for dir in $(subdirs); do $(MAKE) TARGET=output.xml -C $$dir; done

check :
	for dir in $(subdirs); do $(MAKE) check -C $$dir; done
