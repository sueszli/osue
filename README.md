This repository contains some previous exams and all exercises provided by the Operating Systems course at the TU Wien up until the 2022 Winter Semester, as well as my solutions to them.

It garnered a lot of attention during the 2022 exam as it was successfully solved on the very same day.

![Screenshot 2023-01-25 211427](https://user-images.githubusercontent.com/61852663/214799331-991ec7f6-a881-4857-9159-30f3cc0ac52d.png)

Also the exercises `ispalindrome`, `fb_arc_set` and `http` received full points and passed all unit tests in 2022.

Topics include (but are not limited to):

- Sockets
- Process creation (fork, exec)
- Pipes
- Semaphores
- Shared memory

```
$ tree
.
├── exams
│   ├── 2006 ✅
│   ├── 2012 ✅
│   ├── 2017 ✅
│   ├── 2020 ✅
│   └── 2022 ✅
│
└── exercises
    └── regular
        ├── 1
        │   ├── a [getopt]
        │   │   ├── binary-digits ✅
        │   │   ├── ispalindrom ✅
        │   │   ├── mycompress
        │   │   ├── mydiff
        │   │   ├── myexpand
        │   │   ├── mygrep ✅
        │   │   ├── mysort
        │   │   ├── postfixcalc
        │   │   └── stegit
        │   │
        │   └── b [tcp]
        │       ├── battleship
        │       ├── coffeemaker
        │       ├── mastermind
        │       │
        │       └── random_np [shared memory]
        │           ├── 3coloring ✅
        │           └── fb_arc_set ✅
        │
        ├── 2 [pipe/fork]
        │   ├── calc
        │   ├── cpair
        │   ├── dsort
        │   ├── encr
        │   ├── forkFFT
        │   ├── forksort
        │   ├── hashsum
        │   ├── intmul ✅
        │   ├── mygzip
        │   ├── proxy
        │   ├── randsched
        │   ├── stillepost
        │   └── websh
        │
        └── 3 [shared memory]
            ├── 2048
            ├── 4wins
            ├── auth
            ├── banking
            ├── battleships
            ├── caesar
            ├── calendar
            ├── chstat
            ├── hangman
            ├── http ✅ [exception: tcp]
            ├── integrate
            ├── lastmsg
            ├── mrna
            ├── procdb
            ├── sortserver
            ├── storetool
            └── tictactoe
```

Also, check out the solutions from my fellow colleagues:

- [@flofriday's solutions](https://github.com/flofriday/OSUE-2020)
- [@filipppp's solutions](https://github.com/filipppp/OSUE-2021)
- [@HED's solutions](https://github.com/HED-GIT/TU_WIEN_BETRIEBSSYSTEME)

