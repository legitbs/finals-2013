CC=g++
CCFLAGS=-g3 -O1 -Wall -w -pedantic -fmessage-length=0 -fno-stack-protector -D_FORTIFY_SOURCE=0 -pie -fPIE -s

O_FILES = CIOConnection.o Domain.o MailData.o MailEngine.o MailPath.o main.o MersenneRNG.o SMTPClientInstance.o SmtpCommand.o SMTPHelperFunctions.o smtpserverinstance.o SpamFilter.o

atmail : $(O_FILES)
	$(CC) $(CCFLAGS) -o atmail $(O_FILES)

# dependency stuff, see: http://www.cs.berkeley.edu/~smcpeak/autodepend/autodepend.html
# pull in dependency info for *existing* .o files
-include $(O_FILES:.o=.d)

.SUFFIXES : .o .cpp

.cpp.o :  
	$(CC) $(CCFLAGS) -c $<
	$(CC) -MM $(CFLAGS) $*.cpp > $*.d

clean:
	rm -f atmail *.o *.d
