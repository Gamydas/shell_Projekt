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

General:
- there might be certain inputs that still cause unwanted behaviour in the shell, but every input that does that that I am aware of is now handled so the shell should work smoothly now

Known problems:
- command history does not yet allow you to navigate back to an empty command line if that was the original state

Installation: 
- if you have downloaded all projectfiles including the makefile you can simply run make all and from then on ./shell to start up the shell
- if you wish to run the shell with the GDB debugger simply run make clean, then make shell_debug and finally gdb ./shell_debug

Limitations and Syntax:
General Syntax
- sequential commands via ; and background execution via & are not yet implemented
- the Syntax for a command looks like this : cmd -flags (if there are any) target (if there is one)


Redirections:
- Redirections must be immediately followed by the target file
- Multiple redirections must be grouped together and cannot have new commands in between i.e. ls file.txt file2.txt > output.txt 2> error.txt 
- Redirection command can also be first in line i.e. > file.txt ls

Builtins:
- type does not yet support preexisting linux programs like wc, ls, grep etc. 

Tab Completion:
- works for full and partial completion(in case of multiple matches)
- does not yet support nested completion or printing out all options on double tab

Pipelining:
- a command may not begin with the "|" operator
- an example for the general pipelining syntax is: ls | grep main | wc 
- works for multiple and single pipelines
- does not yet work with builtins


If you see this project and find any problems/bugs or have recommendations for improvements please let me know.
