# CPU-Scheduler-Simulator

## Architecture

When the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue, it is moved from there into the Ready Queue. When a process is in any blue queue, its time spent waiting increases. When a process is inside the I/O Wait Queue, its current I/O burst time decreases. When a process is in the CPU, its remaining time for service decreases (you need to remember the initial service time for output). When a process inside the I/O Wait Queue has its current I/O burst time reach zero, that I/O burst is removed and it is moved from there to the Ready Queue. When a process in the CPU reaches zero remaining time for service, it is moved from there into the Finish Queue.

The program will implement schedulers such as:

FF: First-Come-First-Served
RR: Round Robin (with settable quantum)
SP: Shortest Process Next
SR: Shortest Remaining Time
HR: Highest Response Ratio Next

The program will accept a switch -s which selects which scheduler to simulate, an optional time quantum -q and an input and output filename:
```./schsim -s FF -q 2 input.csv output.csv```

If the optional time quantum is provided for a selected scheduler that doesn't require it, the program ignores that switch and runs normally.
