.PHONY:all
all:mysql_insert mysql_select

mysql_insert:mysql_insert.cc
	g++ $^ -o $@ -L /usr/lib64/mysql -lmysqlclient

mysql_select:mysql_select.cc
	g++ $^ -o $@ -L /usr/lib64/mysql -lmysqlclient

.PHONY:clean
clean:
	rm mysql_insert mysql_select
