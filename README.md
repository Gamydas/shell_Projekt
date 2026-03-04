In this project I am trying to create my own UNIX Shell.

Currently implemented commands: 
- cd
- pwd
- echo (added 25.02.2026) [not used yet due to preexisting Unix program]
- type (added 25.02.2026)
- Redirections! (added 26.02.2026)
- pre-installed Linux programs i.e ls, wc, vim etc.
- Completion via TAB! (finally some quality of life) (added 28.02.2026)
- Command history with arrow keys
- Pipelining! (added 04.03.2026)
  
To do:
- Quoting
- Proper error-handling via Errno

Known problems:
- due to an entirely new input structure (termios raw mode and bitwise reading) some for now unhandled signs (like arrow keys) cause unwanted behaviour in the shell, will be fixed ASAP
- command history sometimes causes a realloc error (working on a fix)
- command history does not yet allow you to navigate back to an empty command line if that was the original state

Installation: 
- if you have downloaded all projectfiles including the makefile you can simply run make all and from then on ./shell to start up the shell
- if you wish to run the shell with the GDB debugger simply run make clean, then make shell_debug and finally gdb ./shell_debug

Limitations and Syntax:
General Syntax
- sequential commands via ; and background execution via & are not yet implemented
- the Syntax for a command looks like this : cmd -flags (if there are any) target (if there is one)

Redirections:
- Redirections only work in succession i.e. ls file.txt file2.txt > output.txt 2> error.txt and not with seperate commands strung inbetween
- Redirection command can also be first in line i.e. > file.txt ls

Builtins:
- type does not yet support preexisting linux programs like wc, ls, grep etc.

Tab Completion:
- works for full and partial completion(in case of multiple matches)
- does not yet support nested completion or printing out all options on double tab

Pipelining:
- an example for the general pipelining syntax is: ls | grep main | wc 
- works for multiple and single pipelines
- does not yet work with builtins


If you see this project and find any problems/bugs or have recommendations for improvements please let me know.
