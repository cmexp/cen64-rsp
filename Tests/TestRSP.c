#include "CPU.h"
#include "Pipeline.h"
#include <stdio.h>
#include <stdlib.h>

struct BusController;

/* Stub functions. */
uint32_t BusReadWord(struct BusController *bus, uint32_t address) { return 0; }
void BusWriteWord(const struct BusController *bus, uint32_t address, uint32_t size) {}

void DMAFromDRAM(struct BusController *bus, void *dest, uint32_t src, uint32_t size) {}
void DMAToDRAM(struct BusController *bus, uint32_t dest, const void *src, size_t size) {}

void BusClearRCPInterrupt(struct BusController *bus, unsigned i) {}
void BusRaiseRCPInterrupt(struct BusController *bus, unsigned i) {}

/* Entry point. */
int main(int argc, const char *argv[]) {
	FILE *rspUCodeFile;
	struct RSP *rsp;
	size_t total, size;
	long i, cycles;

	if (argc != 3) {
		printf("Usage: %s <uCode> <Cycles>\n", argv[0]);
		return 0;
	}

	/* Open the uCode file, create an RSP instance. */
	if ((rspUCodeFile = fopen(argv[1], "r")) == NULL) {
		printf("Failed to open RSP uCode.\n");

		return 1;
	}

	if ((rsp = CreateRSP()) == NULL) {
		printf("Failed to initialize the RSP.\n");

		fclose(rspUCodeFile);
		return 2;
	}

	/* Read the uCode into the RSP. */
	for (size = 0, total = 0; total < 8192; total += size) {
		size = fread(rsp->dmem + total, 1, 8192 - total, rspUCodeFile);

		if (ferror(rspUCodeFile)) {
			printf("Unable to read the uCode file.\n");

			fclose(rspUCodeFile);
			DestroyRSP(rsp);
			return 3;
		}
	}

	fclose(rspUCodeFile);
	cycles = strtol(argv[2], NULL, 10);
	printf("Running RSP for %ld cycles.\n", cycles);

	for (i = 0; i < cycles; i++)
		CycleRSP(rsp);

	RSPDumpRegisters(rsp);
	return 0;
}

