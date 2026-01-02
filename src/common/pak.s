	.section .rodata
	.global g_pak
	.balign 16
g_pak:
	.incbin "game.zip"
	.global g_pak_end
	.balign 1
g_pak_end:
	.byte 0
