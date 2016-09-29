#include "ndncatchunks.hpp"
#include "version.hpp"
#include "options.hpp"
#include "consumer.hpp"

int main()
{
	std::string name = "/ndn/version.hpp";
	ndn::chunks::ndnChunks nchunks;
	nchunks.startChunk(name);
}
