zta: zta.c libuevloop
	gcc -o zta zta.c -I../../include -L../../dist -luevloop -lncurses -O3 -flto

.PHONY: libuevloop

libuevloop:
	$(MAKE) -C ../.. clean
	$(MAKE) -C ../.. CFLAGS="-I./include --include threads.h -D UEL_CRITICAL_SECTION_OBJ_TYPE=mtx_t -D UEL_CRITICAL_ENTER='mtx_lock(&uel_critical_section)' -D UEL_CRITICAL_EXIT='mtx_unlock(&uel_critical_section)'"
