#include "common.h"

#define MAX_SOLUTION_LEN (64)

static EdgeList parseInput(int argc, char* argv[]) {
  // validate
  if ((argc - 1) < 1) {
    usage("too few arguments");
  }
  if ((argc - 1) > MAX_NUM_EDGES) {
    usage("too many arguments");
  }

  for (int i = 1; i < argc; i++) {
    for (int j = 1; j < argc; j++) {
      if ((i != j) && (strcmp(argv[i], argv[j])) == 0) {
        usage("duplicate arguments");
      }
    }
  }

  for (int i = 1; i < argc; i++) {
    if (index(argv[i], '-') == NULL) {
      usage("no dash in argument");
    }
    if (index(argv[i], '-') != rindex(argv[i], '-')) {
      usage("more than one dash in argument");
    }
  }

  for (int i = 1; i < argc; i++) {
    char* str = strdup(argv[i]);
    if (str == NULL) {
      error("strdup");
    }

    char* saveptr;
    char* fst = strtok_r(str, "-", &saveptr);
    char* snd = strtok_r(NULL, "-", &saveptr);
    if ((fst == NULL) || (snd == NULL)) {
      usage("missing node name");
    }

    for (size_t j = 0; j < strlen(fst); j++) {
      if (!isdigit(fst[j])) {
        usage("non digit character in node name");
      }
    }
    for (size_t j = 0; j < strlen(snd); j++) {
      if (!isdigit(snd[j])) {
        usage("non digit character in node name");
      }
    }

    free(str);
  }

  // make struct
  EdgeList output;
  output.len = (size_t)argc - 1;

  for (int i = 1; i < argc; i++) {
    char* saveptr;
    char* fst = strtok_r(argv[i], "-", &saveptr);
    char* snd = strtok_r(NULL, "-", &saveptr);

    errno = 0;
    output.edges[i - 1].from.name = strtoul(fst, NULL, 10);
    output.edges[i - 1].to.name = strtoul(snd, NULL, 10);
    if (errno != 0) {
      error("strtoul");
    }
  }
  return output;
}

static EdgeList generateSolution(EdgeList edgeList) {
  // get nodeArray
  Node nodeArray[MAX_NUM_EDGES * 2];
  size_t counter = 0;

  for (size_t i = 0; i < edgeList.len; i++) {
    Node fst = edgeList.edges[i].from;
    Node snd = edgeList.edges[i].to;

    bool foundFst = false;
    bool foundSnd = false;
    for (size_t i = 0; i < counter; i++) {
      if (nodeArray[i].name == fst.name) {
        foundFst = true;
      }
      if (nodeArray[i].name == snd.name) {
        foundSnd = true;
      }
    }
    if (!foundFst) {
      nodeArray[counter++] = fst;
    }
    if (!foundSnd) {
      nodeArray[counter++] = snd;
    }
  }

  // color nodeArray
  for (size_t i = 0; i < counter; i++) {
    nodeArray[i].color = rand() % 3;
  }

  // color edgeList
  for (size_t i = 0; i < edgeList.len; i++) {
    for (size_t j = 0; j < counter; j++) {
      if (edgeList.edges[i].from.name == nodeArray[j].name) {
        edgeList.edges[i].from.color = nodeArray[j].color;
      }
      if (edgeList.edges[i].to.name == nodeArray[j].name) {
        edgeList.edges[i].to.color = nodeArray[j].color;
      }
    }
  }

  // add illegal edges to solution
  EdgeList solution;
  size_t sCounter = 0;
  for (size_t i = 0; i < edgeList.len; i++) {
    Edge e = edgeList.edges[i];
    if (e.from.color == e.to.color) {
      solution.edges[sCounter++] = e;
    }
  }
  solution.len = sCounter;
  return solution;
}

static void writeSolution(EdgeList solution, Shm_t* shmp) {
  if ((sem_wait(&shmp->writeMutex) == -1) && (errno != EINTR)) {
    error("sem_wait");
  }

  // use bool flag so we don't need another semaphore
  static bool updatedCounter = false;
  if (!updatedCounter) {
    shmp->numGenerators++;
    updatedCounter = true;
  }

  if (solution.len < MAX_SOLUTION_LEN) {
    shmp->buf[shmp->writeIndex] = solution;
    shmp->writeIndex = (shmp->writeIndex + 1) % BUF_LEN;
  }

  if (sem_post(&shmp->writeMutex) == -1) {
    error("sem_post");
  }
}

int main(int argc, char* argv[]) {
  EdgeList input = parseInput(argc, argv);

  srand((unsigned int)time(NULL));

  int fd = shm_open(SHM_PATH, O_RDWR, 0);
  if (fd == -1) {
    error("shm_open");
  }
  Shm_t* shmp =
      mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    error("mmap");
  }

  while (!shmp->terminateGenerators) {
    if ((sem_wait(&shmp->numFree) == -1) && (errno != EINTR)) {
      error("sem_wait");
    }
    if (shmp->terminateGenerators) {
      // supervisor called sem_post() to free generators for shutdown
      break;
    }

    EdgeList solution = generateSolution(input);
    writeSolution(solution, &shmp);

    if (sem_post(&shmp->numUsed) == -1) {
      error("sem_post");
    }
  }

  if (munmap(shmp, sizeof(*shmp)) == -1) {
    error("munmap");
  }
  if (close(fd) == -1) {
    error("close");
  }
  exit(EXIT_SUCCESS);
}
