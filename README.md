# CrossProcess
trying to make process pipes and command runin across operating systems (god the horror)

# BUGs/Rough edges
1. if a process does not produce any output then it would block forever (at least on linux)
2. even when data is return the function cpipe_available_bytes needs more work