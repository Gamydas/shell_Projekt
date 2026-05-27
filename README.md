This is my attempt at a UNIX-Shell written in C. This is a learning project to deepen my understanding of the C language and learn new concepts like process creation and management and file descriptors. This shell implements a variety of things such as the use of terminal raw mode for live input reading, a state-machine parser for quoting recognition and a separate execution logic that handles general instruction execution and instruction specifics like pipelining, sequential instructions via ; or redirections. This project does not use preexisting shell libraries.

Installation: 
- if you have downloaded all projectfiles including the makefile you can simply run make all and from then on ./shell to start up the shell
- if you wish to run the shell with the GDB debugger simply run make clean, then make shell_debug and finally gdb ./shell_debug

Currently implemented commands: 
- cd
- pwd
- echo (this actually uses linux executable)
- type 
- linux executables

Currently implemented features:
- Redirections
- Pipelining
- Quoting
- Sequential Commands via ; 
- Tab Completion
- History via Arrow keys

To do:
- improved dispatching/calling of builtins
- implementing signalhandling via syscall sigaction for certain signals
- Proper window control, more in Known Problems

Definitions:
- a command means either a builtin or a linux executable like ls
- an instruction means a processed string segment from its beginning till next Connector
- Connectors are either a Pipecall, a Seq Call (;) or the end the input

Known problems:
- Due to switching the terminal into raw mode, window handling has to be done manually, i.e currently if the commandlength exceeds the window width the formating will be very
off, this will soon be fixed with implementation of sigaction
- there might be certain inputs that still cause unwanted behaviour in the shell, but every input that does that that I am aware of is now handled so the shell should work smoothly now

Known limitations:
- The following operations are not yet implemented: |& (pipelining stdout and stderr), && (only execute if prior instruction was succesfull), || (only execute if prior instruction was unsuccesfull), & (execute instruction in the background), any form of substition or variable creation, grouping via () or {}
- builtins do not yet have any flags like in bash
- type builtin does not yet support executables like wc, ls, grep etc. 
- TabComplete checks for fileaccess, but if run on a windows machine via WSL or other VMs there will unwanted junk within matches
- TabComplete does not yet ask for permission to print all found matches on third tab press if there are over a certain amount of matches found
- Flag completion does not yet exist
- TabComplete does not yet filter out file types for certain commands, i.e cd gets every file recommended instead of only directories
- history is hardcoded to be a maximum of 50 long and does not yet save to and read from a file, will be changed ASAP
- Redirections are hardcoded to be a maximum of 10 per instruction 
- If Redirection is first in the entire Input, Tab-Completion for builtins and executables does not work

General Syntax
- the Syntax for a command looks like this: cmd -flags (if builtins have flags it will be documented, if not documented assume they dont) target (if there is one)
- Single Quotes '' and Double Quotes take out any special meaning from seperators, once parser is expanded to more special signs/characters, this will be documented here

Redirections:
- the 5 kinds of redirections are: > (switches stdout to target and truncates target), >> (switches stdout to target and appends to target), 2> (switches stderr to target and truncates target), 2>> (switches stderr to target and appends to target), < (switches stdin to target and read from it)
- Redirections must be immediately followed by the target file
- all output redirections will created the mentioned target if it does not yet exist, input redirections will signal an error occuring
- Redirections may also be called within a pipeline
- Redirection command can also be first in line i.e. > file.txt ls 

Tab Completion:
- can complete builtins and executables (keep aforementioned limitations in mind) 
- can complete files
- nested completition works, i.e. user/shell_Projekt/re [Tab] will complete to retired

History: 
- up arrow puts the prior input into the terminal line
- down arrow puts the next/newest input into terminal line, depending on position

Pipelining:
- a command may not begin with the "|" operator
- an example for the general pipelining syntax is: ls | grep main | wc 
- works for multiple and single pipelines
- every command is pipelineable, however consider that a pipeline creates children for every instruction, so local changes made by cd or the like will not apply to the shell

Quoting: 
- using single ' or double " Quotes switches the parser into the respective modes
- during Single_Quote, every type character is stripped of it's meaning and interpreted as a literal character
- Double_Quote currently has the same functionality as Single_Quote, due to the relevant characters/functions not being implemented yet 


If you see this project and find any problems/bugs or have recommendations for improvements please let me know.
