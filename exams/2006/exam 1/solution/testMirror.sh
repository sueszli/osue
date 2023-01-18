# valid
ARGS="http://www.test.at"
ARGS="-t 5 http://www.test.at"
ARGS="-t 2 -m 50 http://www.test.at"
ARGS="http://test"
ARGS="-t 50 http://test"
ARGS="-t3 -m5 http://test"
ARGS="-t3 -m5 -l http://test"

# invalid
ARGS="htp://www.test.at"
ARGS="-l -t 5 -m 20"
ARGS="-t 2 -m 50 ftp://www.test.at"
ARGS=""
ARGS="-t http://test"
ARGS="-ll http://test"
ARGS="-t -l http://test"

# -------

clear

make all

valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./mirror $ARGS

make clean
