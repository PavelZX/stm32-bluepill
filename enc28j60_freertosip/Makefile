PROJECT = enc28j60_freertosip

CFILES = enc28j60_freertosip.c
CFILES += web_server.c
CFILES += enc28j60.c NetworkInterface.c application_freertosip_hooks.c
CFILES += uart.c uart_stdio.c dwt_delay.c util.c
CFILES += $(CFILES_FREERTOS) $(CFILES_FREERTOS_HEAP_4)
CFILES += $(CFILES_FREERTOS_PLUS_TCP)

include ../freertos.mk
include ../freertos-plus-tcp.mk
include ../rules.mk
