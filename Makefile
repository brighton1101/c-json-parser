OUTPUT_TARGET = cjson.o

clean:
	rm $(OUTPUT_TARGET)

compile:
	gcc -g cjson.c -o $(OUTPUT_TARGET)

all: clean compile