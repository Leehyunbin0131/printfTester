#ifndef PRINT_HPP
# define PRINT_HPP
# include <unistd.h>
# include <string.h>
# include <sys/wait.h>
# include <sstream>
# include <string>
# include <cstdio>
# define B_SIZE 4096

extern "C"
{
	int ft_printf(const char *format, ...);
}
# define TEST(nb, test) {if (ac == 1 || testNumber == nb) test;}
# define SUBCATEGORY(min, max, output) {if (ac == 1 || (testNumber >= min && testNumber <= max)) output}

using namespace std;

extern int iTest;
extern int testNumber;
extern char * testName;
extern bool showTest;
int pipeOut, stdOut;

struct CapturedOutput
{
	std::string	output;
	int			ret;
};

static std::string escapeOutput(const std::string &output)
{
	std::ostringstream	escaped;

	for (std::string::const_iterator it = output.begin(); it != output.end(); ++it)
	{
		unsigned char c = static_cast<unsigned char>(*it);
		if (c == '\0')
			escaped << "\\0";
		else if (c == '\n')
			escaped << "\\n";
		else if (c == '\t')
			escaped << "\\t";
		else
			escaped << *it;
	}
	return (escaped.str());
}

static std::string readFd(int fd)
{
	char		buffer[4096];
	std::string	output;
	ssize_t		bytes;

	while ((bytes = read(fd, buffer, sizeof(buffer))) > 0)
		output.append(buffer, bytes);
	if (bytes < 0)
		throw std::runtime_error("read failed");
	return (output);
}

template<typename Callable>
CapturedOutput captureOutput(Callable call)
{
	FILE			*tmp;
	int				tmpFd;
	CapturedOutput	captured;

	tmp = tmpfile();
	if (tmp == NULL)
		throw std::runtime_error("tmpfile() failed");
	tmpFd = fileno(tmp);
	stdOut = dup(1);
	if (stdOut < 0)
		throw std::runtime_error("dup() failed");
	pipeOut = tmpFd;
	fflush(stdout);
	if (dup2(pipeOut, 1) < 0)
		throw std::runtime_error("dup2() failed");
	captured.ret = call();
	fflush(stdout);
	dup2(stdOut, 1);
	close(stdOut);
	lseek(tmpFd, 0, SEEK_SET);
	captured.output = readFd(tmpFd);
	fclose(tmp);
	return (captured);
}

void printTestNumber(char * n, int limit)
{
	testNumber = atoi(n);
	cout << FG_LYELLOW << ", number: " << testNumber << RESET_ALL;
	if (testNumber == 0 || testNumber > limit)
		throw std::runtime_error("invalid test number");
	showTest = true;
}

void showTestInfos(void)
{
	cout << FG_MAGENTA << "args:      [";
	std::ostringstream cmd; cmd << "cat tests/" << testName << ".cpp | grep \"TEST(" << testNumber << ",\" | cut -d \"(\" -f 3 | rev | cut -c4- | rev | tr -d '\n'";
	system(cmd.str().c_str());
	cout << "]" << ENDL;
}

template<typename... Args>
void print(const char * s, Args... args)
{
	pid_t actualTest = fork(); stdOut = 1;
	setbuf(stdout, NULL);
	if (actualTest < 0)
		throw std::runtime_error("I have dropped my fork");
	else if (actualTest == 0)
	{
		CapturedOutput printfResult;
		CapturedOutput ftPrintfResult;

		printfResult = captureOutput([&]() { return printf(s, args...); });
		if (showTest)
		{
			showTestInfos();
			cout << FG_GREEN << "printf:    [" << escapeOutput(printfResult.output) << "] = " << printfResult.ret << ENDL;
		}
		ftPrintfResult = captureOutput([&]() { return ft_printf(s, args...); });
		if (showTest)
			cout << FG_LGREEN << "ft_printf: [" << escapeOutput(ftPrintfResult.output) << "] = " << ftPrintfResult.ret << ENDL;
		else
			check(ftPrintfResult.output == printfResult.output && ftPrintfResult.ret == printfResult.ret);
		showLeaks();
		exit(EXIT_SUCCESS);
	}
	else
	{
		usleep(TIMEOUT_US); int status;
		if (waitpid(actualTest, &status, WNOHANG) == 0)
		{
			kill(actualTest, 9);
			if (showTest)
				cout << FG_BLUE << "ft_printf: " << FG_RED << "[TIMEOUT]" << ENDL;
			else
				cout << FG_RED << iTest << ".TIMEOUT ";
		}
		if (showTest)
			exit(EXIT_SUCCESS);
	}	
	++iTest;
}

template<typename requiredType=int, typename... Args>
void checkn(const char * s, Args... args)
{

	pid_t actualTest = fork(); stdOut = 1;
	setbuf(stdout, NULL);
	if (actualTest < 0)
		throw std::runtime_error("I have dropped my fork");
	else if (actualTest == 0)
	{
		char	printfStr[B_SIZE], ft_printfStr[B_SIZE];
		int		printfRet, ft_printfRet;
		char	eof = EOF; 
		int		readReturn;
		int		p[2];
		unsigned long long int printfn = -1 , ft_printfn = -1;
	
		if (pipe(p) < 0)
			throw std::runtime_error("pipe() failed");
		stdOut = dup(1); pipeOut = p[1]; dup2(pipeOut, 1);
	
		printfRet = printf(s, args..., &printfn); write(1, &eof, 1);
		if ((readReturn = read(p[0], printfStr, B_SIZE)) < 0)
			throw std::runtime_error("read failed");
		printfStr[readReturn - 1] = 0;
		if (showTest)
		{
			dup2(stdOut, 1);
			showTestInfos();
			cout << FG_CYAN << "printf:    [" << printfStr << "] = " << printfRet << " n = " << (unsigned long long int)static_cast<requiredType>(printfn) << ENDL;
			dup2(pipeOut, 1);
		}
		
		ft_printfRet = ft_printf(s, args..., &ft_printfn); write(1, &eof, 1);
		if ((readReturn = read(p[0], ft_printfStr, B_SIZE)) < 0)
			throw std::runtime_error("read failed");
		ft_printfStr[readReturn - 1] = 0;
		close(p[0]); close(pipeOut); dup2(stdOut, 1);
		if (showTest)
		{
			cout << FG_BLUE << "ft_printf: [" << ft_printfStr << "] = " << ft_printfRet << " n = " << (unsigned long long int)static_cast<requiredType>(ft_printfn) << ENDL;
			if (printfn == ft_printfn) cout << FG_GREEN << "cast:      [OK]" << ENDL;
			else cout << FG_RED << "cast:      [KO]" << ENDL;
		}
		else
			check(!strcmp(ft_printfStr, printfStr) && printfRet == ft_printfRet && printfn == ft_printfn);
		showLeaks();
		exit(EXIT_SUCCESS);
	}
	else
	{
		usleep(TIMEOUT_US); int status;
		if (waitpid(actualTest, &status, WNOHANG) == 0)
		{
			kill(actualTest, 9);
			if (showTest)
				cout << FG_BLUE << "ft_printf: " << FG_RED << "[TIMEOUT]" << ENDL;
			else
				cout << FG_RED << iTest << ".TIMEOUT ";
		}
		if (showTest)
			exit(EXIT_SUCCESS);
	}	
	++iTest;
}

#endif