.globl main
.data
    prompt: .asciiz "Input positive integer: "
    data1: .asciiz "The factorial is: "

.text
main:
    #printing the prompt
    la $a0, prompt #load the data from $a0
    li $v0, 4 #system call for print
    syscall

    #reading the input
    li $v0, 5 #system call for read integer
    syscall
    
    move $t0, $v0 #store input in $t0

    move $a0, $t0
    jal factorial #call factorial

    move $s0, $v0
    
    # print the answer
    la $a0, data1
    li $v0, 4
    syscall

    #print the result
    move $a0, $s0 #move the return value to $a0
    li $v0, 1 #syscall print integer
    syscall

    #exit
    li $v0, 10 #system call for exit
    syscall

.text
factorial:
    #if the input = 0, return 1
    beq $a0, $zero, return1
    #else case
    addi $sp, $sp, -8 #allocate call frame
    sw $a0, 0($sp) #store input
    sw $ra, 4($sp) #store the return address
   
    addi $a0, $a0, -1 #n = n -1
    jal factorial #call factorial
	
    lw $a0, 0($sp) #load the original input	
    lw $ra, 4($sp) #load the return address
    addi $sp, $sp, 8 #allocate back a stack
    
    mul $v0, $v0, $a0 #multiple the return value with the input
    jr $ra #exit

return1:
    li $v0, 1 #the return value is 1
    jr $ra #exit    
