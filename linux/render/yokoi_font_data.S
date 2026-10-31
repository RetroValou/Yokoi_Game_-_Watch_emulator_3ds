.section .rodata
.balign 4
.global yokoi_font_data
.hidden yokoi_font_data
yokoi_font_data:
.incbin "third_party/Audiowide-Regular.ttf"

.global yokoi_font_license
.hidden yokoi_font_license
yokoi_font_license:
.incbin "third_party/Audiowide-OFL.txt"
.byte 0

.section .note.GNU-stack,"",%progbits
