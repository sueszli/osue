/****************************************************************/
/* CLIENT in Pseudo-Code                                        */
/*--------------------------------------------------------------*/

// open(shared_memory)  <--- set up
// open(sem_server)
// open(sem_client)
// open(sem_ready)

// wait(sem_client)     <--- client mutex
// write(shared_memory)
// post(sem_server)     <--- tell server to start processing
// wait(sem_ready)      <--- wait until server is done processing
// read(shared_memory)
// post(sem_client)     <--- client mutex

// unmap(shared_memory) <--- clean up
