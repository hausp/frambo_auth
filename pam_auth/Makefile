pam_frambo: pam_frambo.c
	gcc -fPIC -DPIC -shared -rdynamic -o pam_frambo.so pam_frambo.c

install-pam: pam_frambo
	mv -f pam_frambo.so /lib/arm-linux-gnueabihf/security/
