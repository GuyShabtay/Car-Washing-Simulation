# In order to compile and run the program please enter these 2 lines to the console on a Linux system:

- Compile: ```gcc carSimulation.c -o carSimulation -lm```
- Example to run: ```./carSimulation 5 2 3 20```

This project demonstrate a simulation of a car wash facility. The facility has stations for washing cars, there are N stations when M cars may arrive

The cars arrive at one of the stations, where at each station one car can be washed at any given moment.

Every car that arrives waits in line until one of the stations becomes vacant. As soon as one of the stations becomes vacant, a car waiting in line moves to the vacant station and at the end of the wash leaves the facility.

each car is simulated by a separate process. Entering and exiting the line is defined as a critical section.

For cars that are waiting in line, processes must be paused and not put in the cars, this must be done with the help of semaphores. Synchronization between busy waiting.

The main software that manages the entire system should take care of the management of the entire washing facility, the creation of the car processes and their closure accordingly.

At the begining, the software receives the following arguments on the command line:

- Number of washing stations (5 at most)
- Average time between car arrivals (reasonable time for use: around 1.5 seconds)
- Average time to wash a car (reasonable time to use: around 3 seconds)
- Running time of the simulation (reasonable time to use: 30 seconds)

The times are treated as the Poisson process of the distribution.
Each car (process) prints three lines to the screen in the following cases:

- When the car arrives at the facility and joins the line.
- When the car entered one of the parking spaces.
- When the car has finished washing and leaves and discards the facility.

The lines to be printed:

- Elapsed time since the system was activated.
- Appropriate message (eg for each car).
- Identification number of the car (of the process).

