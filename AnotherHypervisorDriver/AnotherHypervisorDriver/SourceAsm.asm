PUBLIC AsmEnableVmxOperation




.code _text



AsmEnableVmxOperation PROC PUBLIC

	PUSH R10			    ; Save the state
	
	XOR R10, R10			; Clear the R10
	MOV R10, CR4

	OR R10, 10000000000000b	; Set the 14th bit 
	MOV CR4, R10
	
	POP R10			     	; Restore the state
	RET

AsmEnableVmxOperation ENDP

END