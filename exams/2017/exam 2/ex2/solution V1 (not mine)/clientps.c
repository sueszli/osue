/****************************************************************/
/* CLIENT in Pseudo-Code                                        */
/*--------------------------------------------------------------*/
/* open(shared_memory)                                          */
/* open(sem_server)                                             */
/* open(sem_client)                                             */
/* open(sem_ready)                                              */
/*                                                              */
/* wait(sem_client)                                             */
/* write(shared_memory)                                         */
/* post(sem_server)                                             */
/* wait(sem_ready)                                              */
/* read(shared_memory)                                          */
/* post(sem_client)                                             */
/*                                                              */
/* unmap(shared_memory)                                         */
/****************************************************************/
