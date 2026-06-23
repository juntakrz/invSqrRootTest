#include <intrin.h>
#include <math.h>
#include <stdio.h>

const int iterationCount = 10000000;
const float half = -0.5f;
const float threeHalves = 1.5f;
const int magic = 0x5f3759df;
const float fmodifier = 5.9604645e-8f;

__declspec(naked) float rsqrtASM(unsigned int seed)
{
	__asm
	{
		// Preserve callee-saved registers
		push edi

		xorps xmm3, xmm3
		mov edi, dword ptr[seed]
		mov edx, dword ptr[magic]
		movss xmm4, dword ptr[fmodifier]
		movss xmm5, dword ptr[half]
		movss xmm6, dword ptr[threeHalves]

		// Loop until iteration count is zero
		mov ecx, dword ptr[iterationCount]

		loopStart:
		// seed = 1664525u * seed + 1013904223u
		// value = (seed >> 8) * 5.9604645e-8f
		imul edi, edi, 1664525
		add edi, 1013904223
		mov eax, edi
		shr eax, 8
		movd xmm0, eax
		mulss xmm0, xmm4						// xmm0 = value
		movss xmm2, xmm0						// xmm2 = value (required for storing x2)

		// x2 = value * 0.5
		mulss xmm2, xmm5						// xmm2 = x2

		// y  = value;
		// i  = *(long*)&value;
		// i = 0x5f3759df - (i >> 1);
		// y = *(float*)&i;
		movd eax, xmm0
		shr eax, 1
		neg eax
		add eax, edx							// i = -(i >> 1) + 0x5f3759df, allows preserving of ESI
		movd xmm0, eax
		movss xmm1, xmm0						// xmm0 = y, xmm1 = y, xmm2 = x2

		// y = y * (threehalfs - (x2 * y * y));
		mulss xmm0, xmm0						// xmm0 = y * y
		mulss xmm0, xmm2						// xmm0 = x2 * xmm0
		addss xmm0, xmm6						// result of (threeHalves - (x2 * y * y) is stored in xmm1
		mulss xmm0, xmm1						// finally the result of (y * (threehalfs - (x2 * y * y)) is stored in xmm0

		addss xmm3, xmm0

		// Exit loop once iteration count is zero
		dec ecx
		jnz loopStart

		// Return via x87 ST(0) as per IA32 requirement
		sub esp, 4
		movss dword ptr[esp], xmm3
		fld dword ptr[esp]
		add esp, 4

		pop edi

		ret
	}
}

float Q_rsqrt(float variable)
{
	long i;
	float x2, y;

	x2 = variable * half;
	y = variable;
	i = *(long*)&y;
	i = 0x5f3759df - (i >> 1);
	y = *(float*)&i;
	y = y * (threeHalves - (x2 * y * y));

	return y;
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		//return 1;
	}

	float value = 0.0f;
	unsigned int seed = 123456789;

	unsigned long long now = __rdtsc();
	float cyclesPerIteration = 0.0f;

	volatile float resultSink = rsqrtASM(seed);

	now = __rdtsc() - now;
	cyclesPerIteration = (float)now / (float)iterationCount;
	printf("ASM: %d iterations took %llu cycles, ~%.2f cycles/it.\n", iterationCount, now, cyclesPerIteration);

	now = __rdtsc();

	for (int iteration = 0; iteration < iterationCount; ++iteration)
	{
		seed = 1664525u * seed + 1013904223u;
		value = (seed >> 8) * 5.9604645e-8f;

		resultSink += Q_rsqrt(value);
	}

	now = __rdtsc() - now;
	cyclesPerIteration = (float)now / (float)iterationCount;
	printf("C Quake3: %d iterations took %llu cycles, ~%.2f cycles/it.\n", iterationCount, now, cyclesPerIteration);

	now = __rdtsc();

	for (int iteration = 0; iteration < iterationCount; ++iteration)
	{
		seed = 1664525u * seed + 1013904223u;
		value = (seed >> 8) * 5.9604645e-8f;

		resultSink += 1.0f / sqrtf(value);
	}

	now = __rdtsc() - now;
	cyclesPerIteration = (float)now / (float)iterationCount;
	printf("C inverse sqrt: %d iterations took %llu cycles, ~%.2f cycles/it.\n", iterationCount, now, cyclesPerIteration);

	return 0;
}