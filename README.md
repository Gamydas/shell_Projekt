This is my attempt at a UNIX-Shell written in C. This is a learning project to deepen my understanding of the C language and learn new concepts like process creation and management and file descriptors. This shell, while not perfect, implements a variety of things such as the use of terminal raw mode for live input reading, a state-machine parser for quoting recognition, pipelining and redirection. This project does not use preexisting shell libraries.

Installation: 
- if you have downloaded all projectfiles including the makefile you can simply run make all and from then on ./shell to start up the shell
- if you wish to run the shell with the GDB debugger simply run make clean, then make shell_debug and finally gdb ./shell_debug

Currently implemented commands: 
- cd
- pwd
- echo  
- type 
- Redirections
- pre-installed Linux programs i.e ls, wc, vim etc.
- Completion via TAB)
- Command history with arrow keys
- Pipelining
- Quoting
To do:
- Proper error-handling via Errno

General:
- there might be certain inputs that still cause unwanted behaviour in the shell, but every input that does that that I am aware of is now handled so the shell should work smoothly now

Known problems:
- a quoted token i.e "foo" needs a space after the second quote before pressing enter, will fix this soon
Limitations and Syntax:
General Syntax
- sequential commands via ; and background execution via & are not yet implemented
- the Syntax for a command looks like this : cmd -flags (if there are any) target (if there is one)
- Single Quotes '' and Double Quotes take out any special meaning from seperators, once parser is expanded to more special    signs/characters, this will be documented here

Redirections:
- Redirections must be immediately followed by the target file
- Multiple redirections must be grouped together (i.e. ls file.txt file2.txt > output.txt 2> error.txt ) and cannot have new commands in between 
- Redirection command can also be first in line i.e. > file.txt ls (If this is done built in and executable completion will no longer work)

Builtins:
- type does not yet support executables like wc, ls, grep etc. 

Tab Completion:
- can complete builtins and executables with the limitation that the instruction needs to be the first token in the argument string, i.e "ls > text.txt" or similarily (which should cover 99% of use cases)
- can complete files, limitations are that flags are not yet completable (might be added in the future) and that there is no filter yet for filetypes depending on the instruction, i.e cd can get every filetype completed not just directories
- nested completition works, i.e. user/shell_Projekt/re [Tab] will complete to retired

History: 
- up arrow puts the prior input into the terminal line
- down arrow puts the next/newest input into terminal line, depending on position

Pipelining:
- a command may not begin with the "|" operator
- an example for the general pipelining syntax is: ls | grep main | wc 
- works for multiple and single pipelines
- does not yet work with builtins


If you see this project and find any problems/bugs or have recommendations for improvements please let me know.
