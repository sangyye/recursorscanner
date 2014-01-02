Recursorscanner - Find open DNS recursors
=========================================

Recursorscanner is a small program written in C to exactly solve one task:

Find all open DNS recursors on your (or other) networks.   
The program simply sends a question for an A record (google.com) to the destination ip.   
If an answer is received we've found an open recursor.

Should run on all Linux/Unix systems.
