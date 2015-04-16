.globl main
.data
        
space: .asciiz " "
init: .asciiz "Initial array is:\n"
finished: .asciiz "Insertion sort is finished!\n"
names:
	.align 3
        .asciiz "Joe"
        .align 3
        .asciiz "Jenny"
        .align 3
        .asciiz "Jill"
        .align 3
        .asciiz "John"
        .align 3
        .asciiz "Jeff"
        .align 3
        .asciiz "Joyce"
        .align 3
        .asciiz "Jerry"
        .align 3
        .asciiz "Janice" #thang nay la Janice
        .align 3
        .asciiz "Jake" #thang nay la Jake
        .align 3
        .asciiz "Jonna"
        .align 3
        .asciiz "Jack"
        .align 3
        .asciiz "Jocelyn"
        .align 3
        .asciiz "Jessie"
        .align 3
        .asciiz "Jess"
        .align 3
        .asciiz "Janet"
        .align 3
        .asciiz "Jane"
        .align 3
        
data: 	.align 2
 	.space 64 #16 * 4

brace1: .asciiz "["
brace2: .asciiz "]"
spc: .asciiz " "
newline: .asciiz "\n"	
 
.text
main:
	
	la $a0, names
	la $a1, 16
	
	jal initialize_data
	
	la $a0, init
	li $v0, 4
	syscall
	
	la $a0, data
	la $a1, 16
	jal print_array_data
	
	la $a0, data
	la $a1, 16
	jal insertion_sort
	
	la $a0, finished
	li $v0, 4
	syscall
	
	la $a0, data
	la $a1, 16
	jal print_array_data
	
	li $v0, 10
	syscall

.text
initialize_data:
	# takes an array
	li $t0, 0 #i = 0
	la $t1, data
	init_data_loop:
	beq $t0, $a1, exit_initialize_data
	#else case
	sw $a0, ($t1)
	addi $a0, $a0, 8
	addi $t1, $t1, 4
	addi $t0, $t0, 1
	
	j init_data_loop
	exit_initialize_data: jr $ra

.text
str_lt:
	#$a0 is the pointer to 1st string, $a1 is the pointer to second string

	move $t0, $a0
	move $t1, $a1

	loop:
		lb $t2, 0($t0) #load the first character byte from first string
		lb $t3, 0($t1) #load the first character byte from first string
		
		beq $t2, $zero, return1 #string1 = null then return 1
		beq $t1, $zero, return0 #string2 = null then return 0
		
		#else case when $t2 and $t1 is different from null
		
		bgt $t2, $t3, return0 #if first byte str1 > firts byte str2 return 0
		bgt $t3, $t2, return1 #if first byte str2 > firts byte str1 return 0
		#else
		addi $t0, $t0, 1 #move to second character string 1
		addi $t1, $t1, 1 #move to second character string 2
		j loop
	return1:
		li $v0, 1
		jr $ra
	return0:
		li $v0, jr
		0 $ra

	
.text
swap: #swap the position of two position array[i] and array[i+1]
	# register $a0 is the array and register $a1 is the index i
	sll $t1, $a1, 2 # $t1 = 4*$a1
	add $t1, $a0, $t1 #t1 = array + 4i

	lw $t0, 0($t1)
	lw $t2, 4($t1)

	sw $t2, 0($t1)
	sw $t0, 4($t1)

	jr $ra
	
.text
insertion_sort:
	addi $sp, $sp, -4
	sw $ra, 0($sp)
	

	move $s2, $a0 #store the array to $s2
	move $s3, $a1 #store the length to $s3

	addi $s0, $zero, 1 #i = 1
	
	for1st:
		beq $s0, $s3, exit1
		addi $s1, $s0, -1 #j = i - 1
		#else case
		
		for2nd:
			blt $s1, $zero, exit2 #if $s1 = j < 0: exit2
			sll $t1, $s1, 2 #t1 = 4*$s1 = 4*j                
			add $t2, $s2, $t1 #t2 = array + 4 * j  t2 = a[j]           
			lw $t3, 0($t2)
			lw $t4, 4($t2)

			#comparing the two array[j] and array[j+1]	
			move $a0, $t3
			move $a1, $t4
			jal str_lt
			#if the return value = 0 (t3 > t4)
			beqz $v0, exit2

			#if not, swap them
			move $a0, $s2
			move $a1, $s1
			jal swap

			addi $s1, $s1, -1 #j = j - 1
			j for2nd #loop 2nd for loop again
	exit2:
		addi $s0, $s0, 1 #i = i + 1
		j for1st #jump to the first for loop
	exit1:
		lw $ra, 0($sp)
		addi $sp, $sp, 4
		
		jr $ra

.text
print_array_data:
	move $t0, $zero #initiate i = 0
	move $t1, $a0 # move the array of pointer to register $t1
	
	#printf("[");
	la $a0, brace1 
	li $v0, 4
	syscall
	
	#start looping
	start_print_data:
		bge $t0, $a1, exit_print_data #if i > lenght ($a1), exit

		#else 
		#print the space
		la $a0, spc 
		li $v0, 4
		syscall
		
		#printf("%s ", array[i])
		lw $a0, 0($t1)
		li $v0, 4
		syscall
		
		# array move to the next string, and i++
		addi $t1, $t1, 4
		addi $t0, $t0, 1
		j start_print_data
	
	exit_print_data:	
	#printf("]\n");
		la $a0, spc
		li $v0, 4
		syscall
		
		la $a0, brace2
		li $v0, 4
		syscall
		
		la $a0, newline
		li $v0, 4
		syscall
		
		jr $ra
