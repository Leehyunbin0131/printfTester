extern "C"
{
#define new tripouille
#include "ft_printf.h"
#undef new
}

#include "sigsegv.hpp"
#include "check.hpp"
#include "print.hpp"
#define TEST_LIMIT 36

int iTest = 1;
int testNumber;
char * testName;
bool showTest = false;

int main(int ac, char ** av)
{
	int			local;
	char		buffer[8];
	void		*ptr;
	std::string	longStr;
	std::string	longLiteral;
	std::string	mixedLong;

	local = 42;
	ptr = &local;
	longStr.assign(12000, 'a');
	longLiteral.assign(9000, 'Z');
	mixedLong = std::string("start-") + longStr + "-end";
	signal(SIGSEGV, sigsegv);
	cout << FG_LYELLOW << "category: stress" << RESET_ALL;
	testName = av[0];
	if (ac != 1)
		printTestNumber(av[1], TEST_LIMIT);
	cout << endl;
	TEST(1, print(""));
	TEST(2, print("plain literal without conversions"));
	TEST(3, print(longLiteral.c_str()));
	TEST(4, print("%s", longStr.c_str()));
	TEST(5, print("%s", mixedLong.c_str()));
	TEST(6, print("A%cB", 0));
	TEST(7, print("%cA%cB%c", 0, 0, 0));
	TEST(8, print("%c%c%c%c", 0, 'A', 0, 'B'));
	TEST(9, print("%c%c%c", 255, 0, 127));
	TEST(10, print("%s%c%s%c%s", "left", 0, "middle", 0, "right"));
	TEST(11, print("%p", ptr));
	TEST(12, print("%p", buffer));
	TEST(13, print("%p", (void *)0));
	TEST(14, print("%p %p %p", ptr, buffer, (void *)0));
	TEST(15, print("%d %d %d %d", INT_MIN, -1, 0, INT_MAX));
	TEST(16, print("%i %i %i %i", INT_MIN, -1, 0, INT_MAX));
	TEST(17, print("%u %u %u %u", 0, 1, UINT_MAX / 2, UINT_MAX));
	TEST(18, print("%x %x %x %x", 0, 1, UINT_MAX / 2, UINT_MAX));
	TEST(19, print("%X %X %X %X", 0, 1, UINT_MAX / 2, UINT_MAX));
	TEST(20, print("%%"));
	TEST(21, print("%%%%%%"));
	TEST(22, print("%% %c %% %s %% %p %% %d %% %i %% %u %% %x %% %X %%", 'Q', "str", ptr, -42, 42, UINT_MAX, UINT_MAX, UINT_MAX));
	TEST(23, print("%d%d%d%d%d%d%d%d%d%d", 1, -2, 3, -4, 5, -6, 7, -8, 9, -10));
	TEST(24, print("%u%u%u%u%u%u%u%u", 0, 1, 10, 100, 1000, 10000, UINT_MAX - 1, UINT_MAX));
	TEST(25, print("%x%X%x%X%x%X", 0, 0, 0xabcdef, 0xabcdef, UINT_MAX, UINT_MAX));
	TEST(26, print("%s%s%s%s%s", "", "a", "", longStr.c_str(), ""));
	TEST(27, print("%c%s%c%d%c%x%c", 0, "binary", 0, INT_MIN, 0, UINT_MAX, 0));
	TEST(28, print("%p%c%p%c%p", ptr, 0, buffer, 0, (void *)0));
	TEST(29, print("%d %u %x %X", -1, -1, -1, -1));
	TEST(30, print("%d %i %u %x %X", 2147483647, -2147483647 - 1, 4294967295U, 4294967295U, 4294967295U));
	TEST(31, print("%s%s%s%s%s%s%s%s%s%s", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"));
	TEST(32, print("%c%c%c%c%c%c%c%c%c%c", '0', 0, '1', 0, '2', 0, '3', 0, '4', 0));
	TEST(33, print("%p %d %s %u %c %x %X %%", ptr, INT_MIN, longStr.c_str(), UINT_MAX, 0, UINT_MAX, UINT_MAX));
	TEST(34, print("prefix%s%c%s%s%s%c%suffix", "", 0, "mid", "", longStr.c_str(), 0, ""));
	TEST(35, print("%c", 0));
	TEST(36, print("%s%c", "", 0));
	cout << ENDL;
	return (0);
}
