# MOS
Multi-Programming OS

This directory includes three phases of a MOS.
There mainly 3 components of the MOS:
  - CPU
  - Card Reader
  - Line Printer
1. Along every phase of the MOS, CPU definitions such as Memory, Registers, etc becomes more structured.
2. Card Reader is the input given (from an input .txt file) to the CPU with a defined structure as follows - 
    i. $AMJ000100010001
      AMJ - A Multiprogramming Job
      The next 3 numbers of 4 digits each represent
        a. Job Id
        b. Total Time Limit
        c. Total Line Limit
  ii. GD10PD10H
        This is a Program Card consisting of program instructions. GD (Get Data), PD (Put Data), H (Halt) as such have certain OS functionalities associated with them.
  iii. $DTA
        This represents that following cards are Data Cards.
  iv. Hello World!
        Data provided to the OS.
  v. $END0001
        This depicts the end of the program or job with specified id.
3. Line Printer outputs the program output on an output .txt file.

With every phase, the complexity of the OS increases such as Paging is introduced over Phase 2 and overall error handling of the OS becomes better.
