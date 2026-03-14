This is my active development branch. Here I experiment with new features, concepts and the like which might not make it to the final version on main.

Most recent changes:
- greatly Improved parser, redirections now get handled in the parser instead of afterwards, much more efficient than before, pipelining now works agan with the new parser
- added error handling for a lot of functions, will work on expanding this as much and cleanly as possible
- better modularisation

Currently working:
- builtin pipelining
- further improvments to errorhandling
- more support for special characters (e.g. $, ;, &)

Still conceptualising:
- a history structure
- a module responsible for handling command execution, with an easy to expand design
