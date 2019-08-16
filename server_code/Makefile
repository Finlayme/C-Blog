FLAGS=-std=c++11 -L/usr/lib64/mysql -lmysqlclient -ljsoncpp -lpthread -g

.PHONY:all
all:db_test blog_server

db_test:db_test.cc db.hpp
	g++ db_test.cc -o db_test $(FLAGS)

blog_server:blog_server.cc db.hpp
	g++ blog_server.cc -o blog_server $(FLAGS)

.PHONY:clean
clean:
	rm db_test blog_server
