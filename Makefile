MODULES_DIRS := k2tree

all: fetch_deps format modules

re: clean all

fetch_deps:
	chmod a+x fetch_deps.sh && bash fetch_deps.sh

clean:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) clean -C $$dir;  \
	done

modules:
	for dir in ${MODULES_DIRS}; do \
		$(MAKE) -C $$dir; \
	done


format:
	find . -path ./lib -prune -o -regex '.*\.\(c\|h\)' -exec clang-format -style=file -i {} \;
