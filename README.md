In this project I am trying to create my own UNIX Shell.

Currently implemented commands: 
- cd
- pwd
- echo (added 25.02.2026) [not used yet due to preexisting Unix program]
- type (added 25.02.2026)
- Redirections! (added 26.02.2026)
- preinstalled Linux programs i.e ls, wc, vim etc.
- Completition via TAB! (finally some quality of life) (added 28.02.2026)
- Command history with arrow keys
- Pipelining! (added 04.03.2026)
  
To do:
- Quoting
- Proper Errorhandling via Errno

Known problems:
- due to entirely new input structure (terminos raw mode and bitwise reading) some for now unhandeled signs (like arrow keys) cause unwanted behaviour in the shell, will be fixed ASAP
- command history sometimes causes a realloc error (working on a fix)
- command history does not yet allow you to navigate back to an empty command line if that was the original state

Limitations and Syntax:
General Syntax
- currently cmd execution structure outside of pipelining can only handle 1 command, this will be expanded once basics structure is done and remaining issues are fixed
- while the above remains, the Syntax for a command looks like this, : cmd -flags (if there are any) target (if there is one)

Redrections:
- Redirections only work in succession i.e. ls file.txt file2.txt > output.txt 2> error.txt and not with seperate commands strung inbetween
- Redirection command can also be first in line i.e. > file.txt ls

Buildtins:
- type does not yet support preexisting linux programs like wc, ls, grep etc.

Tab Completition:
- works for full and partial completition(in case of multiple matches)
- does not yet support nested completition or priting out all options on double tab

Pipelining:
- an example for the general pipelining syntax is: ls | grep main | wc 
- works for multiple and single pipelines
- as stated above, this does already work with multiple pregiven commands
- does not yet work with buildt-ins


If you see this project and find any problems/bugs or have recommendations for improvements please let me know.
