Was passiert beim Aufruf von ``listen(sock_fd, 1)``?

---
Der Server wartet auf eingehende Verbindungen. Es ist maximal eine Verbindung erlaubt (sog. backlog). Verbindet sich ein zweiter Client wird die Verbindung verweigert.