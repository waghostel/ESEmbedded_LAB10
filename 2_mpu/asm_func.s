.syntax unified


.global	read_sp
read_sp:
	mov r0, sp //註解
	bx lr //跳轉回lr

.global	read_msp
read_msp:
	mrs r0, msp //Move msp to r0
	bx lr

.global	read_psp
read_psp:
	mrs r0, psp //Move psp to r0
	bx lr

.global	read_ctrl
read_ctrl:
	mrs r0, control 
	bx lr
	

.global	start_user
start_user:
	movs lr, r0 
	msr psp, r1

	movs r3, #0b1
	msr control, r3
	isb
	
	

.global	sw_priv
sw_priv:
	movs r3, #0b11
	msr control, r3
	isb
	
