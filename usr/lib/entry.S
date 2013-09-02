.global _start
.extern main
_start:
 
## here you might want to get the argc/argv pairs somehow and then push
## them onto the stack...

## right now, though, we have no concept of a "command line" and thus no
## args that could be pushed
pushl $0
pushl $0
 
# call the user's function
call main

# once it exists, call a syscall to terminate the process 
 
# loop in case we haven't yet rescheduled
lp:
hlt
jmp lp
