create database if not exists blog_system;

use blog_system;

drop table if exists `blog_table`;
create table `blog_table`(blog_id int not null primary key auto_increment,
                          title varchar(50), content text, tag_id int, create_time varchar(50));

drop table if exists `tag_table`;
create table `tag_table`(tag_id int not null primary key auto_increment,
                         tag_name varchar(50));

insert into tag_table values(null, 'C');
insert into tag_table values(null, 'C++');
insert into tag_table values(null, 'Java');

insert into blog_table values(null, '第一篇博客', '### 这是我的第一篇博客', 1, '2019-05-21 12:00');
insert into blog_table values(null, '第二篇博客', '### 这是我的第二篇博客', 1, '2019-05-21 12:00');
