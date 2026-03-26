.intel_syntax noprefix

.global itoa
.type   itoa, @function
itoa:
  # char* itoa(int, char*, int UNUSED)
  # rdi: int
  # rsi: char*
  # rax: char* (ret, points to same place as arg1 at beginning)

  sub rsp, 8
  mov [rsp], rbp
  mov rbp, rsp

  # rbx used, so prior val must be saved
  sub rsp, 8
  mov [rsp], rbx

  xor rax, rax

  mov rbx, rsi # temp hold init pointer

  cmp rdi, 0
  je itoa_zero

  mov rax, rdi
  mov rcx, 10 # to div by to get lsd from rax
  l1_itoa:
    xor rdx, rdx # to store remainder from div
    div rcx # rax storing quotient, rdx storing remainder
    add dl, 0x30 # '0'
    mov byte ptr [rsi], dl
    inc rsi
    test rax, rax
    jnz l1_itoa

  rev:
    # Null-terminate for strrev to work
    mov byte ptr [rsi], 0x0

    # no longer need to save registers
    # after strrev, rax is only important
    mov rdi, rbx
    call strrev

    jmp end_itoa

  itoa_zero:
    mov byte ptr [rsi], 0x30 # '0'
    inc rsi
    # Null-terminate
    mov byte ptr [rsi], 0x0

    mov rax, rbx

  end_itoa:
    mov rsp, rbp
    mov rbp, [rsp]
    add rsp, 8

    ret


strrev:
  # strrev(char*)
  # rdi: char*
  # rax: char* (ret)

  sub rsp, 8
  mov [rsp], rbp
  mov rbp, rsp

  # using rbx, so must save prior to using it
  sub rsp, 8
  mov [rsp], rbx

  # arg0 already set
  call _strlen
  mov rcx, rax # storing length of str

  # rdi contains pointer to end
  # I have no idea what this is for but must be done to work properly
  mov rsi, rdi
  dec rsi
  sub rdi, rcx
  mov rax, rdi
  # rax contains pointer to beginning to return
  # rdi contains pointer from left hand side
  # rsi contains pointer from right hand side


  cmp rdi, rsi
  # in case rdi pointer passed or equals to rsi pointer
  #  no need to reverse anymore
  jge end_strrev

  strrev_l:
    mov bl, byte ptr [rdi]
    mov cl, byte ptr [rsi]
    mov byte ptr [rdi], cl
    mov byte ptr [rsi], bl

    inc rdi
    dec rsi

    cmp rdi, rsi
    # in case rdi is still less than rsi, keep going when it's not (rdi >= rsi), fall to end
    jl strrev_l

  end_strrev:
    # input str is assumed to be null-terminated so no null-termination needed

    # restore rbx
    lea rbx, [rsp]
    add rsp, 8

    mov rsp, rbp
    mov rbp, [rsp]
    add rsp, 8

    ret


_strlen:
  # int strlen(char*)
  # rdi: char*
  # rax: int (ret)

  # str is assumed to be null-terminated
  # null-char not accounted for in len

  sub rsp, 8
  mov [rsp], rbp
  mov rbp, rsp

  # using rbx, so save it
  sub rsp, 8
  mov [rsp], rbx

  xor rax, rax

  mov bl, byte ptr [rdi]
  cmp bl, 0x0
  je end_strlen

  strlen_l:
    inc rax
    inc rdi

    mov bl, byte ptr [rdi]
    cmp bl, 0x0
    jne strlen_l

  end_strlen:
    lea rbx, [rsp]
    add rsp, 8

    mov rsp, rbp
    mov rbp, [rsp]
    add rsp, 8

    ret
