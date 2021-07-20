# Bluetooth-Detector-App
BT detector simulator in C using pThreads

~ Implementation ~

For the construction of the app, i utilized pThreads in order to enhance the functionality of the program and increase its time tracking precision. The program consists of five workers, i.e pThreads, which are waken at the right time using a simple scheduling logic, which is going to be  explained in more detail in the following paragraph. The scheduling is facilitated by mutexes and conditions, which allow each worker to "freeze" whenever there is not any necessary work to be done.

~ Scheduling ~

As mentioned above, five pThreads are used during the app's lifetime cycle. Now let's clarify the role of each one and how it contributes to the whole program's flow. The first thread (thread-0) works as a time counter. It is the main event handler since, keeping track of time, it sends signals to other threads when needed. Specifically, every ten seconds it wakes thread-1 and every 45 minutes it wakes thread-2, as well as terminates the program after a user-specified duration. Thread-1 when woken, immediately detects a random Bluetooth Mac Address and stores it into a FIFO queue. This thread also checks every address already stored in the queue and deletes those who have remained in it for too long. It also saves each close contact in a secondary queue named "closeContactList". At this point it should be underlined that the Mac Addresses are stored into a text file, and they are passed into a 2-D global matrix when the program is initiated. Thread-2 is responsible for the COVID test. In case its positive, it wakes thread-3 which uploads the close contacts (from the "closeContactList" queue) to a binary file named "covidTrace.bin". When everything is saved, thread-4 is woken and it clears the queue containing the close contacts.

~ Notes ~
The binary file named "covidTrace.bin" contains all the close contacts that have been uploaded during the app's lifetime cycle. Regarding the use of queues, a structure containing two queues was created and passed as argument to the threads' functions. This gives the workers, access to both queues. The real "BTnearMe" function-calling-rate is tracked and as the app terminates, it is printed in a text file named "realBTtimer.txt".

~ Statistical Analysis ~

As discussed previously, the time-tracking ability of the app is essential for its functionality, however there can only be precision to a certain point. In this section, i illustrate a statistical analysis of the time-counting inaccuracy of the program. From the following graphs, we can make useful conclusions about the deviation between the time a function was called and the time it should have been called. 
![alt text](https://github.com/kpetridis24/Bluetooth-Detector-App/issues/1#issue-948694725)

~ Conclusions ~
The app was tested both on a PC and a Raspberry Pi 3b+. As the graphs depict, running the program on the PC resulted in significantly larger time-inaccuracy compared to running it on the Raspberry Pi. It is easily noticeable that the time deviation of the function's call on the RPi is one microsecond. On the other hand, on the PC it varies anywhere from ten up to 140 microseconds. That could be due to the much simpler architecture of the RPi (ARM) which may include less processes running parallel to the program's execution. Apart from the absolute value of deviation, the PC version seems to come with a more unpredictable distribution of the time a function is called relative to the time it should have been called. On the contrary, the timestamps measured on the RPi seem to form a standardized pattern.
