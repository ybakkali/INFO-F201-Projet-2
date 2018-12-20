TARGET=spbx graham

normal: $(TARGET)

spbx: spbx.c
	gcc -std=c99 -Wall spbx.c -o spbx

graham: graham.c
	gcc -std=c99 -Wall graham.c -o graham

clean:
	$(RM) $(TARGET)
