TARGET=spbx graham

normal: $(TARGET)

spbx: spbx.c
	gcc -Wall spbx.c -o spbx

graham: graham.c
	gcc -Wall graham.c -o graham

clean:
	$(RM) $(TARGET)
