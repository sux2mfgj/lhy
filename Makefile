SRCS=lhy.c vmx.c x64.c guest.s
KMOD=lhy

.include <bsd.kmod.mk>

d: debug.c lhy_api.h
	$(CC) -Og -o debug debug.c

dc:
	rm -rf debug_bin
