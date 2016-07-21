203E4DCh: ldrb	r3,[r1],1h  ;r1 is memory address where savegame has been loaded
XXXXXXXX: cmp	r2,0h    ; If r2==0 set conditional flags
XXXXXXXX: sub	r2,r2,1h ; substracts 0x1 from r2 so r2--;
XXXXXXXX: eor	r3,r3,r0,asr 8h  
XXXXXXXX: mov	r3,r3, lsl 1h
XXXXXXXX: ldrh	r3,[r12,r3]
XXXXXXXX: eor	r0,r3,r0,lsl 8h 
XXXXXXXX: mov	r0,r0,lsr 10h
XXXXXXXX: mov	r0,r0,lsr 10h
XXXXXXXX: bne 203E4DCh  ;If no conditional flags are enabled, branch(jump) to the start and again..