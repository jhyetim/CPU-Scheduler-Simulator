# CPU-Scheduler-Simulator

## Architecture

When the CPU cycle clock is equal to the arrival time for a process in the Arrival Queue, it is moved from there into the Ready Queue. When a process is in any blue queue, its time spent waiting increases. When a process is inside the I/O Wait Queue, its current I/O burst time decreases. When a process is in the CPU, its remaining time for service decreases (you need to remember the initial service time for output). When a process inside the I/O Wait Queue has its current I/O burst time reach zero, that I/O burst is removed and it is moved from there to the Ready Queue. When a process in the CPU reaches zero remaining time for service, it is moved from there into the Finish Queue.

The program will implement schedulers such as:

• FF: First-Come-First-Served\
• RR: Round Robin (with settable quantum)\
• SP: Shortest Process Next\
• SR: Shortest Remaining Time\
• HR: Highest Response Ratio Next

The program will accept a switch -s which selects which scheduler to simulate, an optional time quantum -q and an input and output filename:
```
./schsim -s FF -q 2 input.csv output.csv
```

If the optional time quantum is provided for a selected scheduler that doesn't require it, the program ignores that switch and runs normally.

## Input File Format
The input file will be a .csv containing a list of processes, at which CPU cycle they arrive, how many cycles they take to complete and an optional list of I/O times and how many cycles that I/O burst takes:

```
"A",0,3
"B",2,6
,3,2
"C",4,4
"D",6,5
"E",8,2
```

Each line in the file will either begin with a " character indicating it is a process or a , character indicating it is an I/O burst for the preceding process.

## Output File Format
The output file will look similar to the input file, but will have added the process start time, time spent in wait, finish time, turnaround time and normalized turnaround time (as a float with 2 decimal points) for each of the processes. Finally it will list the mean turnaround time and the mean normalized turnaround time (also as a float with 2 decimal points).

```
"A",0,3,0,0,3,3,1.00
"C",4,4,6,2,10,6,1.50
"B",2,6,3,5,13,11,1.83
"D",6,5,13,7,18,12,2.40
"E",8,2,18,10,20,12,6.00
8.80,2.55
```
