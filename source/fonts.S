.section .rodata
.balign 4
.global folio_sans_start, folio_sans_end, folio_serif_start, folio_serif_end
folio_sans_start:
.incbin "assets/fonts/sans.bcfnt"
folio_sans_end:
.balign 4
folio_serif_start:
.incbin "assets/fonts/serif.bcfnt"
folio_serif_end:
.balign 4
.global folio_inter_start, folio_inter_end, folio_garamond_start, folio_garamond_end, folio_baskerville_start, folio_baskerville_end
folio_inter_start:
.incbin "assets/fonts/inter.bcfnt"
folio_inter_end:
.balign 4
folio_garamond_start:
.incbin "assets/fonts/ebgaramond.bcfnt"
folio_garamond_end:
.balign 4
folio_baskerville_start:
.incbin "assets/fonts/librebaskerville.bcfnt"
folio_baskerville_end:
.section .note.GNU-stack,"",%progbits
