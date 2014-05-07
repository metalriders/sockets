#Makefile
#
# Instrucciones para compilar

# Flags para usar ssl -lcrypto -lssl  (poner al final)

NAME = https
OBJS = tcp.o server.o main.o

CC = gcc
CFLAGS = -Wall -O2 

all: $(OBJS) $(NAME)
	

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(NAME)

doc:
	doxygen
