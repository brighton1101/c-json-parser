FROM debian
RUN apt-get update
RUN apt-get install -y gcc gdb make valgrind
RUN apt-get install -y --reinstall build-essential

RUN mkdir files
