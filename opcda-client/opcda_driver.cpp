#include "json_rpc.h"

int main() {
	OPCDADriverServer s(8891);
	s.start();
}