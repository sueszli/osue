/**
 * @file header.h
 * @author Christian Bigos <e1633026@student.tuwien.ac.at>
 * @date 06.11.2021
 *
 * @brief Header für Exercise 1b
 *
 * Der Header für die Programme supervisor und generator wird hier implementiert.
 **/

#ifndef HEADER_FILE
#define HEADER_FILE

#define SEMFREE "/01633026_free_sem"
#define SEMUSED "/01633026_used_sem"
#define BUFFER "/01633026_circular_buffer"
#define BUFFER_SIZE (1024)

/**
 * @brief Eine Kante wird hierdurch definiert, u = Anfang, v = Ende
 */
typedef struct Edge { 
    int u;
    int v;
} Edge_t;

/**
 * @brief Der Circular Buffer wird hierdurch definiert
 */
struct circBuf{
    /**
     * @brief Hier werden die Lösungen gespeichert
     */
    Edge_t data[BUFFER_SIZE];
    /**
     * @brief Lese- und Schreibende des Circular Buffer
     */
    int readEnd;
    int writeEnd;
    /**
     * @brief Die beste und jetzige Lösung. Zum Vergleichen.
     */
    int bestSolution;
    int currentSolution;
    /**
     * @brief (BOOL) Sollen jeweils den Fakt vom mindestens einem durchgangen Zyklus, 
     * die Eigenschaft, dass der Graph azylisch ist, und das Signal zum Ende des Programms speichern.
     */
    int cycled;
    int acyclic;
    int terminate;
};

#endif