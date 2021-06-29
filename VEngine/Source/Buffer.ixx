module;
#include <stdint.h>
export module Buffer;

struct ID3D11Buffer;

export class Buffer
{
public:

	ID3D11Buffer* data;
	uint64_t size;
};
