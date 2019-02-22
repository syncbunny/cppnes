#include <string>
#include <unistd.h>
#include "nesfortest.h"

int main(int argc, char* argv[]) {
	NESForTest* nes = new NESForTest();
	nes->test();

	return 0;
}
