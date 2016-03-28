subdirs := $(wildcard */)

.PHONY: check $(subdirs)

check: $(subdirs)

$(subdirs):
	$(MAKE) check -C $@
