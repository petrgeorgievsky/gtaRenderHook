#pragma once
inline static void Patch(void* address, void* data, int size)
{
	unsigned long protect[2];
	VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &protect[0]);
	memcpy(address, data, size);
	VirtualProtect(address, size, protect[0], &protect[1]);
}
inline static void SetPointer(int address, void *value)
{
	Patch((void *)address, &value, 4);
}