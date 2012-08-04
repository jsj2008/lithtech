AuthLogin:	AuthLogin.cpp ../libWON.a
	g++ AuthLogin.cpp -o AuthLogin -I../. -D_LINUX ../libWON.a -lpthread


ServerPing:	ServerPing.cpp ../libWON.a
	g++ ServerPing.cpp -o ServerPing -I../. -D_LINUX ../libWON.a -lpthread

../libWON.a:
	make -C ../
