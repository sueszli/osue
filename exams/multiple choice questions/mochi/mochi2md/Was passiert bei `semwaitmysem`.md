Was passiert bei ``sem_wait(my_sem)``?

---
Der Wert von ``my_sem`` wird dekrementiert. Wenn dieser danach ``0`` ist kann der Prozess zugreifen, ist dieser ``< 0`` wird blockiert.