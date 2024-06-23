# tasks-communication-by-queue
Designed a task communication system with three sender tasks (two with the same priority, one with higher priority) and one receiver task.
Sender tasks sleep for a random period (Tsender), send messages with timestamps, and update success or block counters per task.
Receiver task sleeps for a fixed period (Treceiver = 100 ms), checks the queue, processes messages, and updates the received messages counter.
Used timers to control task sleep/wake cycles via semaphores.
Implemented callback functions to manage queue operations and reset system statistics every 1000 received messages.
Adjusted Tsender values through predefined arrays and terminated the system after all values are used, indicating "Game Over."
