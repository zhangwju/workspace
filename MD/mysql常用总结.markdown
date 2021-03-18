#mysql常用总结：
    mysql命令格式： mysql -h主机地址 -u用户名 －p用户密码
    mysqladmin -u 用户名 -p 旧密码 password 新密码
    
#命令
#数据库
    create database <数据库名>;
    show databases;
    drop database <数据库名>;
    use <数据库名>;
#表
    create table <表名> (<字段名1> <类型1> [,..<字段名n> <类型n>]);
    select * from MyClass order by id limit 0,2;
    delete from MyClass where id=1;
    update set命令格式：update 表名 set 字段=新值,… where 条件;

