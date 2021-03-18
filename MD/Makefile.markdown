Makefile:

这里说说规则(Rules)：
target ... : prerequisites ...
    command
    ...
    ...
  
    一条规则由三部分组成，目标（target）、先决条件（prerequisites）、命令（commands）。
    target也就是一个目标文件，可以是Object File，也可以是执行文件。还可以是一个标签（Label）。 
    prerequisites就是，要生成那个target所需要的文件或是目标。
    command也就是make需要执行的命令。（任意的Shell命令）
    这是一个文件的依赖关系，也就是说，target这一个或多个的目标文件依赖于prerequisites中的文件，其生成规则定义在command中。
    说白一点就是说，prerequisites中如果有一个以上的文件比target文件要新的话，command所定义的命令就会被执行。
    这就是Makefile的规则。也就是Makefile中最核心的内容。  

Makefile之模式规则
模式规则其实也是普通规则，但它使用了如%这样的通配符。如下面的例子：
%.o : %.c
       $(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

此规则描述了一个.o文件如何由对应的.c文件创建。规则的命令行中使用了自动化变量“$<”和“$@”，其中自动化变量“$<”代表规则的依赖，“$@”代表规则的目标。此规则在执行时，命令行中的自动化变量将根据实际的目标和依赖文件取对应值。
    = 是最基本的赋值    
    := 是覆盖之前的值
    ?= 是如果没有被赋值过就赋予等号后面的值
    += 是添加等号后面的值