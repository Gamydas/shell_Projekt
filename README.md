This is my active development branch. Here I experiment with new features, concepts and the like which might not make it to the final version on main.

Most recent changes:
- greatly Improved parser, redirections now get handled in the parser instead of afterwards, much more efficient than before
- added error handling for a lot of functions, will work on expanding this as much and cleanly as possible
- Pipelining is currently disabled!!! This is because soon it might be implemented into the parser and it's still working on the old shell struct
- better modularisation

Currently working:
- Pipelining in the parser
- further improvments to errorhandling
- more support for special characters (e.g. $, ;, &)

Still conceptualising:
- a history structure
