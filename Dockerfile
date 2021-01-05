FROM debian
RUN apt-get update
RUN apt-get install -y gcc gdb make valgrind

RUN mkdir files
